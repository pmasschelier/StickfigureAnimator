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

typedef struct {
    Rectangle selection;
    bool active;
} Selection;

struct RendererContext {
    Arena arena;
    RenderTexture2D rendertexture;
    Rectangle worldViewport;
    struct {
        Shader shader;
        struct {
            int jointRadius;
            int texel;
        } loc;
    } stickfigure;
    struct {
        Shader shader;
        struct {
            int worldViewport, cameraViewport, selection, selectionThickness, resolution;
        } loc;
    } postprocess;
    struct {
        GLuint vao;
        GLuint vbo_v, vbo_t, ebo;
    } gl;
    float joint_radius;
    Selection selection;
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

void renderer_set_selection(RendererContext* context, Rectangle selection) {
    // fprintf(stderr, "Selection: (%f, %f, %f, %f)\n", context->selection.selection.x, context->selection.selection.y, context->selection.selection.width, context->selection.selection.height);
    context->selection = (Selection) {
        .active = true,
        .selection = selection
    };
}

void renderer_reset_selection(RendererContext* context) {
    context->selection = (Selection) { .active = false, .selection = {}};
}

RendererContext* renderer_init(Rectangle worldViewport) {
    constexpr size_t ARENA_SIZE = 4096;
    Shader stickfigureShader = LoadShader(RESOURCE_PATH "resources/shaders/stick.vert", RESOURCE_PATH "resources/shaders/stick.frag");
    if (stickfigureShader.id == 0)
        return nullptr;
    Shader postprocessShader = LoadShader(RESOURCE_PATH "resources/shaders/stick.vert", RESOURCE_PATH "resources/shaders/postprocess.frag");
    if (stickfigureShader.id == 0)
        return nullptr;

    if(glewInit() != GLEW_OK) {
        fprintf(stderr, "FATAL ERROR: Failed to initialize GLEW\n");
        exit(-1);
    }

    const Vector2 vertices[4] = {
        { -1.f, 1.f },
        { 1.f, 1.f },
        { 1.f, -1.f},
        { -1.f, -1.f}
    };
    const GLuint indices[6] = { 0, 3, 2, 2, 1, 0};
    int position = stickfigureShader.locs[SHADER_LOC_VERTEX_POSITION];
    if(position == -1) position = 0;

    GLuint vao, vbo_v, ebo;
    glCreateVertexArrays(1, &vao);

    glCreateBuffers(1, &vbo_v);
    glNamedBufferStorage(vbo_v, sizeof(vertices), vertices, 0);
    glEnableVertexArrayAttrib(vao, position);
    glVertexArrayAttribFormat(vao, position, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, position, 0);
    glVertexArrayVertexBuffer(vao, 0, vbo_v, 0, sizeof(Vector2));
    
    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, sizeof(indices), indices, 0);
    glVertexArrayElementBuffer(vao, ebo);
    
    RendererContext* state = calloc(1, sizeof(RendererContext));
    *state = (RendererContext) {
        .arena = arena_create(ARENA_SIZE),
        .worldViewport = worldViewport,
        .stickfigure = {
            .shader = stickfigureShader,
            .loc = {
                .jointRadius = GetShaderLocation(stickfigureShader, "joint_radius"),
                .texel = GetShaderLocation(stickfigureShader, "texel"),
            },
        },
        .postprocess = {
            .shader = postprocessShader,
            .loc = {
                .worldViewport = GetShaderLocation(postprocessShader, "worldViewport"),
                .cameraViewport = GetShaderLocation(postprocessShader, "cameraViewport"),
                .selection = GetShaderLocation(postprocessShader, "selection"),
                .selectionThickness = GetShaderLocation(postprocessShader, "selectionThickness"),
                .resolution = GetShaderLocation(postprocessShader, "resolution"),
            },
        },
        .joint_radius = 1.f,
        .gl = { .vao = vao, .vbo_v = vbo_v, .vbo_t = 0, .ebo = ebo },
        .selection = { .active = false, .selection = {} }
    };

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );

    return state;
}

void renderer_deinit(RendererContext *state) {
    arena_free(&state->arena);
    UnloadShader(state->stickfigure.shader);
    UnloadShader(state->postprocess.shader);
    UnloadRenderTexture(state->rendertexture);
    glDeleteBuffers(1, &state->gl.vbo_v);
    glDeleteBuffers(1, &state->gl.vbo_t);
    glDeleteBuffers(1, &state->gl.ebo);
    glDeleteVertexArrays(1, &state->gl.vao);
    free(state);
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
    Vector4 color;
    GLuint start;
    GLuint end;
    GLuint type;
    GLfloat thickness;
    GLuint selected;
    GLuint padding[3];
} SSBOStick;

static void renderer_update_screen(RendererContext* state, Vector2 res) {
    bool update = false;
    if(state->rendertexture.id == 0) {
        update = true;
    } else if(state->rendertexture.texture.width != (int)res.x
        || state->rendertexture.texture.height != (int)res.y) {
        update = true;
        UnloadRenderTexture(state->rendertexture);
    }
    if(!update)
        return;

    state->rendertexture = LoadRenderTexture(res.x, res.y);
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

    int texcoord = state->stickfigure.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01];
    if(texcoord == -1) texcoord = 1;

    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(wPositions), wPositions, 0);
    glEnableVertexArrayAttrib(state->gl.vao, texcoord);
    glVertexArrayAttribFormat(state->gl.vao, texcoord, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(state->gl.vao, texcoord, 1);
    glVertexArrayVertexBuffer(state->gl.vao, 1, vbo, 0, sizeof(Vector2));
    glDeleteBuffers(1, &state->gl.vbo_t);
    state->gl.vbo_t = vbo;

    // GLfloat selectionThickness = 1.f / effectiveViewport.width;
    SetShaderValue(state->postprocess.shader, state->postprocess.loc.resolution, &res, SHADER_UNIFORM_VEC2);
    SetShaderValue(state->postprocess.shader, state->postprocess.loc.worldViewport, &effectiveViewport, SHADER_UNIFORM_VEC4);
    GLfloat texel = res.x / effectiveViewport.width;
    SetShaderValue(state->stickfigure.shader, state->stickfigure.loc.texel, &texel, SHADER_UNIFORM_FLOAT);
}

void renderer_render(RendererContext *state, Stickfigure_array_t stickfigures, PivotEdgeIndex_array_t selected, Vector2 res, float pivotRadius) {
    renderer_update_screen(state, res);
    GLuint *edgesSSBO = nullptr, *jointsSSBO = nullptr;
    if(stickfigures.length > 0) {
        jointsSSBO = malloc(stickfigures.length * sizeof(GLuint));
        glCreateBuffers(stickfigures.length, jointsSSBO);
        edgesSSBO = malloc(stickfigures.length * sizeof(GLuint));
        glCreateBuffers(stickfigures.length, edgesSSBO);
        foreach(stickfigures, s, Stickfigure) {
            unsigned s_index = index;
            glNamedBufferStorage(jointsSSBO[index], s->joints.length * sizeof(Vector2), nullptr, GL_MAP_WRITE_BIT);
            Vector2* joints = glMapNamedBuffer(jointsSSBO[index], GL_WRITE_ONLY);
            foreach(s->joints, j, StickfigureJoint) {
                joints[index] = Vector2Add(j->pos, s->position);
            }
            glUnmapNamedBuffer(jointsSSBO[index]);
            glNamedBufferStorage(edgesSSBO[index], s->edges.length * sizeof(SSBOStick), nullptr, GL_MAP_WRITE_BIT);
            SSBOStick* edges = glMapNamedBuffer(edgesSSBO[index], GL_WRITE_ONLY);
            foreach(s->edges, e, StickfigureEdge) {
                edges[index] = (SSBOStick) {
                    .start = e->from,
                    .end = e->to,
                    .type = e->type,
                    .color = (Vector4) {
                        (float)e->data.color.r / 255.f,
                        (float)e->data.color.g / 255.f,
                        (float)e->data.color.b / 255.f,
                        1.f
                    },
                    .thickness = e->thickness,
                    .selected = false
                };
            }
            foreach(selected, s, PivotEdgeIndex) {
                if (s->figure == s_index) {
                    edges[s->edge].selected = true;
                }
            }
            glUnmapNamedBuffer(edgesSSBO[index]);
        }
    }

    SetShaderValue(state->stickfigure.shader, state->stickfigure.loc.jointRadius, &pivotRadius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(state->postprocess.shader, state->postprocess.loc.cameraViewport, &state->worldViewport, SHADER_UNIFORM_VEC4);
    SetShaderValue(state->postprocess.shader, state->postprocess.loc.selection, &state->selection.selection, SHADER_UNIFORM_VEC4);

    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    BeginTextureMode(state->rendertexture);
    ClearBackground(WHITE);
    glBindVertexArray(state->gl.vao);
    for (unsigned i = 0; i < stickfigures.length; i++) {
        glUseProgram(state->stickfigure.shader.id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, edgesSSBO[i]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, jointsSSBO[i]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
    }
    glUseProgram(state->postprocess.shader.id);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glUseProgram(0);
    glBindVertexArray(0);
    EndTextureMode();
    EndBlendMode();

    glDeleteBuffers(stickfigures.length, edgesSSBO);
    free(edgesSSBO);
    glDeleteBuffers(stickfigures.length, jointsSSBO);
    free(jointsSSBO);
    arena_reset(&state->arena);
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
