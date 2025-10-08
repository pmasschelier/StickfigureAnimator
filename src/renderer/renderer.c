#include "renderer.h"
#include "arena.h"
#include "array.h"
#include "raymath.h"
#include "rlgl.h"
#include "src/pivot.h"
#include "src/raylib.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct RendererContext {
    Shader stickfigureShader;
    Shader postprocessShader;
    RenderTexture2D rendertexture;
    Rectangle worldViewport;
    struct {
        unsigned joint_radius;
        unsigned viewport;
    } locations;
    float joint_radius;
};

const size_t SizeofRendererContext = sizeof(RendererContext);

#define CASE_STR(str, prefix, match) case match: str = &#match[prefix]; break

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 [[maybe_unused]] GLsizei length,
                 const GLchar* message,
                 [[maybe_unused]] const void* userParam )
{
    char* source_str;
    switch (source) {
        CASE_STR(source_str, 16, GL_DEBUG_SOURCE_API);
        CASE_STR(source_str, 16, GL_DEBUG_SOURCE_WINDOW_SYSTEM);
        CASE_STR(source_str, 16, GL_DEBUG_SOURCE_SHADER_COMPILER);
        CASE_STR(source_str, 16, GL_DEBUG_SOURCE_THIRD_PARTY);
        CASE_STR(source_str, 16, GL_DEBUG_SOURCE_APPLICATION);
        CASE_STR(source_str, 16, GL_DEBUG_SOURCE_OTHER);
        default: source_str = "UNKNOWN"; break;
    }
    char* type_str;
    switch (type) {
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_ERROR);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_PORTABILITY);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_PERFORMANCE);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_OTHER);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_MARKER);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_PUSH_GROUP);
        CASE_STR(type_str, 14, GL_DEBUG_TYPE_POP_GROUP);
        default: type_str = "UNKNOWN"; break;
    }
    char* severity_str;
    switch (severity) {
        CASE_STR(severity_str, 18, GL_DEBUG_SEVERITY_NOTIFICATION);
        CASE_STR(severity_str, 18, GL_DEBUG_SEVERITY_HIGH);
        CASE_STR(severity_str, 18, GL_DEBUG_SEVERITY_MEDIUM);
        CASE_STR(severity_str, 18, GL_DEBUG_SEVERITY_LOW);
        default: severity_str = "UNKNOWN"; break;
    }
  fprintf( stderr, "GL CALLBACK: source = %s, type = %s, id = %d, severity = %s, message = %s\n",
            source_str, type_str, id, severity_str, message );
}

RendererContext* renderer_init(Rectangle worldViewport) {
    RendererContext* state = calloc(1, sizeof(RendererContext));
    state->worldViewport = worldViewport;
    state->stickfigureShader = LoadShader(RESOURCE_PATH "resources/shaders/stick.vert", RESOURCE_PATH "resources/shaders/stick.frag");
    if (state->stickfigureShader.id == 0) {
        free(state);
        return nullptr;
    }
    state->postprocessShader = LoadShader(RESOURCE_PATH "resources/shaders/stick.vert", RESOURCE_PATH "resources/shaders/postprocess.frag");
    if (state->stickfigureShader.id == 0) {
        free(state);
        return nullptr;
    }

    state->joint_radius = 1.f;
    state->locations.joint_radius = GetShaderLocation(state->stickfigureShader, "joint_radius");
    state->locations.viewport = GetShaderLocation(state->postprocessShader, "viewport");

    if(glewInit() != GLEW_OK) {
        fprintf(stderr, "FATAL ERROR: Failed to initialize GLEW\n");
        exit(-1);
    }

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );

    return state;
}

Texture2D* renderer_get_frame(RendererContext * state) {
    if(state->rendertexture.texture.id == 0)
        return nullptr;
    return &state->rendertexture.texture;
}

static Rectangle renderer_get_effective_viewport(Rectangle worldViewport, Vector2 res) {
    Rectangle effectiveViewport = worldViewport;
    float aspectRatioRatio = (res.x / res.y) / (worldViewport.width / worldViewport.height);
    if(aspectRatioRatio > 1.f) {
        float offset = worldViewport.width * (aspectRatioRatio - 1);
        effectiveViewport.x -= offset / 2.f;
        effectiveViewport.width += offset;
    } else if(aspectRatioRatio < 1.f) {
        float offset = worldViewport.height * (1.f / aspectRatioRatio - 1.f);
        effectiveViewport.y -= offset / 2.f;
        effectiveViewport.height += offset;
    }
    return effectiveViewport;
}

typedef struct {
    Vector2 start;
    Vector2 end;
    Vector4 color;
    GLuint type;
    GLfloat thickness;
    float padding[2];
} SSBOStick;

SSBOStick* renderer_compute_sticks(Arena* arena, Stickfigure* s) {
    SSBOStick* ret = arena_allocate(arena, s->edges.length, sizeof(SSBOStick));
    unsigned int index = 0;
    
    return ret;
}

void renderer_render(RendererContext *state, Stickfigure_array_t stickfigures, Vector2 res, float pivotRadius) {
    if(state->rendertexture.id == 0) {
        state->rendertexture = LoadRenderTexture(res.x, res.y);
    } else if(state->rendertexture.texture.width != res.x
        || state->rendertexture.texture.height != res.y) {
        UnloadRenderTexture(state->rendertexture);
        state->rendertexture = LoadRenderTexture(res.x, res.y);
    }
    if(state->rendertexture.id == 0) {
        fprintf(stderr, "Error: Failed to create render texture of size (%f, %f)\n", res.x, res.y);
        return;
    }
    
    Rectangle effectiveViewport = renderer_get_effective_viewport(state->worldViewport, res);
    const Vector2 wPositions[4] = {
        {effectiveViewport.x, effectiveViewport.y },
        { effectiveViewport.x + effectiveViewport.width, effectiveViewport.y },
        { effectiveViewport.x + effectiveViewport.width, effectiveViewport.y + effectiveViewport.height},
        { effectiveViewport.x, effectiveViewport.y + effectiveViewport.height}
    };
    /* const Vector2 wPositions[4] = { */
    /*     {0.f, 0.f }, */
    /*     { res.x, 0.f }, */
    /*     { res.x, res.y}, */
    /*     { 0.f, res.y} */
    /* }; */
    const Vector2 vertices[4] = {
        { -1.f, 1.f },
        { 1.f, 1.f },
        { 1.f, -1.f},
        { -1.f, -1.f}
    };
    const GLuint indices[6] = { 0, 3, 2, 2, 1, 0};
    GLuint vao, vbo_v, vbo_t, ebo;
    int position = state->stickfigureShader.locs[SHADER_LOC_VERTEX_POSITION];
    if(position == -1) position = 0;
    int texcoord = state->stickfigureShader.locs[SHADER_LOC_VERTEX_TEXCOORD01];
    if(texcoord == -1) texcoord = 1;
    glCreateVertexArrays(1, &vao);

    glCreateBuffers(1, &vbo_v);
    glNamedBufferStorage(vbo_v, sizeof(vertices), vertices, 0);
    glEnableVertexArrayAttrib(vao, position);
    glVertexArrayAttribFormat(vao, position, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, position, 0);
    glVertexArrayVertexBuffer(vao, 0, vbo_v, 0, sizeof(Vector2));

    glCreateBuffers(1, &vbo_t);
    glNamedBufferStorage(vbo_t, sizeof(wPositions), wPositions, 0);
    glEnableVertexArrayAttrib(vao, texcoord);
    glVertexArrayAttribFormat(vao, texcoord, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, texcoord, 1);
    glVertexArrayVertexBuffer(vao, 1, vbo_t, 0, sizeof(Vector2));

    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, sizeof(indices), indices, 0);
    glVertexArrayElementBuffer(vao, ebo);

    SetShaderValue(state->stickfigureShader, state->locations.joint_radius, &pivotRadius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(state->postprocessShader, state->locations.viewport, &state->worldViewport, SHADER_UNIFORM_VEC4);
    
    GLuint* ssbos;
    if(stickfigures.length > 0) {
        ssbos = malloc(stickfigures.length * sizeof(GLuint));
        glCreateBuffers(stickfigures.length, ssbos);
        foreach(stickfigures, stickfigure, Stickfigure) {
            glNamedBufferStorage(ssbos[index], stickfigure->edges.length * sizeof(SSBOStick), nullptr, GL_MAP_WRITE_BIT);
            SSBOStick* map = glMapNamedBuffer(ssbos[index], GL_WRITE_ONLY);
            for(unsigned i = 0; i < stickfigure->edges.length; i++) {
                map[i].start = stickfigure->joints.data[stickfigure->edges.data[i].from].pos;
                map[i].end = stickfigure->joints.data[stickfigure->edges.data[i].to].pos;
                map[i].type = stickfigure->edges.data[i].type;
                map[i].color = (Vector4) { 0.f, -1.f, 0.f, 1.f};
                map[i].thickness = 1.f;
            }
            glUnmapNamedBuffer(ssbos[index]);
        }
    }

    BeginTextureMode(state->rendertexture);
    ClearBackground(WHITE);
    glBindVertexArray(vao);
    for (unsigned i = 0; i < stickfigures.length; i++) {
        glUseProgram(state->stickfigureShader.id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbos[i]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    }
    glUseProgram(state->postprocessShader.id);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glUseProgram(0);
    glBindVertexArray(0);
    EndTextureMode();

    glDeleteBuffers(stickfigures.length, ssbos);
    glDeleteBuffers(1, &vbo_v);
    glDeleteBuffers(1, &vbo_t);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

Vector2 renderer_get_screen_position(RendererContext* context, Vector2 worldPosition, Rectangle screenViewport) {
    const Vector2 scale = {
        screenViewport.width / context->worldViewport.width,
        screenViewport.height / context->worldViewport.height
    };
    return (Vector2) {
        screenViewport.x + scale.x * (worldPosition.x - context->worldViewport.x),
        screenViewport.y + screenViewport.height - scale.y * (worldPosition.y - context->worldViewport.y),
    };
}

Vector2 renderer_get_world_position(RendererContext* context, Vector2 canvasPosition, Vector2 res) {
    const Rectangle viewport = renderer_get_effective_viewport(context->worldViewport, res);
    const Vector2 scale = {
        viewport.width / res.x,
        viewport.height / res.y
    };
    return (Vector2) {
        viewport.x + canvasPosition.x * scale.x,
        viewport.y + canvasPosition.y * scale.y,
    };
}

void renderer_deinit(RendererContext *state) {
    UnloadShader(state->stickfigureShader);
    UnloadRenderTexture(state->rendertexture);
    free(state);
}
