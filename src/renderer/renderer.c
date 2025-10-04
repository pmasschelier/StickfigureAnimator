#include "renderer.h"
#include "raymath.h"
#include "rlgl.h"
#include "src/pivot.h"
#include "src/raylib.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

RendererError renderer_init(RendererState *state) {
    state->stickfigureShader = LoadShader(RESOURCE_PATH "resources/shaders/stick.vert", RESOURCE_PATH "resources/shaders/stick.frag");
    if (state->stickfigureShader.id == 0) {
        return RENDERER_FAILED_TO_LOAD_SHADER;
    }

    if(glewInit() != GLEW_OK) {
        fprintf(stderr, "FATAL ERROR: Failed to initialize GLEW\n");
        exit(-1);
    }

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );

    return RENDERER_NOERROR;
}
void renderer_render(RendererState *state, Stickfigure* stickfigure, Rectangle viewport) {
    RenderTexture2D rendertexture = LoadRenderTexture(viewport.width, viewport.height);
    if(rendertexture.id == 0) {
        fprintf(stderr, "Error: Failed to create render texture of size (%f, %f)\n", viewport.width, viewport.height);
        return;
    }

    const Vector2 wPositions[4] = {
        {0.f, 0.f },
        { viewport.width, 0.f },
        { viewport.width, viewport.height},
        { 0.f, viewport.height}
    };
    const Vector2 vertices[4] = {
        { -1.f, 1.f },
        { 1.f, 1.f },
        { 1.f, -1.f},
        { -1.f, -1.f}
    };
    const GLuint indices[6] = { 0, 3, 2, 2, 1, 0};
    GLuint vao, vbo, ebo;
    int position = state->stickfigureShader.locs[SHADER_LOC_VERTEX_POSITION];
    if(position == -1) position = 0;
    int texcoord = state->stickfigureShader.locs[SHADER_LOC_VERTEX_TEXCOORD01];
    if(texcoord == -1) texcoord = 1;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(wPositions), nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(position);

        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(wPositions), wPositions);
        glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 0, (char*)(4 * sizeof(Vector2)));
        glEnableVertexAttribArray(texcoord);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    typedef struct {
        Vector2 start;
        Vector2 end;
        Vector4 color;
        float thickness;
    } SSBOStick;
    
    GLuint ssbo;
    if(stickfigure) {
        glCreateBuffers(1, &ssbo);
        glNamedBufferStorage(ssbo, stickfigure->sticks.length * sizeof(SSBOStick), nullptr, GL_MAP_WRITE_BIT);
        SSBOStick* map = glMapNamedBuffer(ssbo, GL_WRITE_ONLY);
        for(unsigned i = 0; i < stickfigure->sticks.length; i++) {
            fprintf(stderr, "Stick nº%d\n", i);
            map->start = stickfigure->sticks.data[i].pivot;
            map->end = stickfigure->sticks.data[i].handle;
            map->color = (Vector4) { 1.f, 0.f, 0.f, 1.f};
            map->thickness = 10.f;
        }
        glUnmapNamedBuffer(ssbo);
    }

    /* unsigned pos1 = GetShaderLocation(state->stickfigureShader, "start"); */
    /* SetShaderValue(state->stickfigureShader, pos1, &stickfigure->sticks.data[0].pivot, SHADER_UNIFORM_VEC2); */
    /**/
    /* unsigned pos2 = GetShaderLocation(state->stickfigureShader, "end"); */
    /* SetShaderValue(state->stickfigureShader, pos2, &stickfigure->sticks.data[0].handle, SHADER_UNIFORM_VEC2); */
    /**/
    /* float thickness = 10.f; */
    /* unsigned t = GetShaderLocation(state->stickfigureShader, "thickness"); */
    /* SetShaderValue(state->stickfigureShader, t, &thickness, SHADER_UNIFORM_FLOAT); */
    /**/
    /* Vector4 color = {1.0, 0.0, 0.0, 1.0}; */
    /* unsigned c = GetShaderLocation(state->stickfigureShader, "color"); */
    /* SetShaderValue(state->stickfigureShader, c, &color, SHADER_UNIFORM_VEC4); */
    /**/
    /* unsigned v = GetShaderLocation(state->stickfigureShader, "viewport"); */
    /* SetShaderValue(state->stickfigureShader, v, &viewport, SHADER_UNIFORM_VEC4); */

    /* rlViewport(viewport.x, viewport.y, viewport.width, viewport.height); */
    /* DrawRectangleRec((Rectangle){0.f, 0.f, viewport.width, viewport.height}, RED); */
    /* DrawRectangleRec(viewport, RED); */

    BeginTextureMode(rendertexture);
    /* rlViewport(viewport.x, viewport.y, viewport.width, viewport.height); */
    ClearBackground(WHITE);
    if(stickfigure) {
        glUseProgram(state->stickfigureShader.id);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
            glBindVertexArray(vao);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glUseProgram(0);
    }
    /* rlViewport(0, 0, GetScreenWidth(), GetScreenHeight()); */
    /* DrawRectangleRec((Rectangle) {0.f, 0.f, viewport.width, viewport.height}, MAGENTA); */
    EndTextureMode();

    if(state->rendertexture.id != 0) {
        UnloadRenderTexture(state->rendertexture);
    }
    state->rendertexture = rendertexture;

    glDeleteBuffers(1, &ssbo);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    /* DrawTexture(state->rendertexture.texture, viewport.x, viewport.y, WHITE); */
}

void renderer_deinit(RendererState *state);
