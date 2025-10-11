#include "interface.h"
#include "arena.h"
#include "components/callback.h"
#include "components/components.h"
#include "components/utils.h"
#include "pivot.h"
#include "raylib.h"
#include "renderer/renderer.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern RendererContext* renderer_context;

InterfaceData* InterfaceInit() {
    const size_t COMPONENT_ARENA_SIZE = 4096;
    void *memory = malloc(COMPONENT_ARENA_SIZE);
    assert(memory);
    InterfaceData* data = malloc(sizeof(InterfaceData));
    *data = (InterfaceData){
        .arena = arena_create(COMPONENT_ARENA_SIZE),
        .rendererData = {
            .pivotRadius = 1.f
        },
        .context = {
            .arena = &data->arena,
            .clickableHovered = false,
            .theme = {
                .color = { PRIMARY_COLOR, (Clay_Color){}, SECONDARY_COLOR, (Clay_Color){} },
            }
        }
    };
    
    char filename[512];
    for(unsigned i = 0; i < ICON_COUNT; i++) {
        snprintf(filename, 512, RESOURCE_PATH "resources/icons/%s", ICON_FILENAMES[i]);
        data->icons[i] = LoadTexture(filename);
    }
    return data;
}

void HandleCreateStickfigure(RendererData* data) {
    Clay_ElementData canvas = Clay_GetElementData(Clay_GetElementId(CLAY_STRING("canvas")));
    printf("HandleCreateStickfigure (%f, %f, %f, %f)\n", canvas.boundingBox.x, canvas.boundingBox.y, canvas.boundingBox.width, canvas.boundingBox.height);
    Vector2 pivot = renderer_get_world_position(renderer_context, (Vector2){ canvas.boundingBox.width / 2.f, canvas.boundingBox.height / 2.f}, (Vector2) { canvas.boundingBox.width, canvas.boundingBox.height });
    PivotCreateStickfigure(&data->stickfigure, nullptr, data->stickType, (Vector2) { pivot.x, pivot.y - 5.f }, PI / 2, 10.f);
}

typedef struct {
    StickfigurePartType* stickType;
    StickfigurePartType stickTypeRequested;
} HandleChangeModeData;

void HandleChangeMode(void *data) {
    HandleChangeModeData *modedata = data;
    *modedata->stickType = modedata->stickTypeRequested;
}

void RenderFileMenu(void *priv, Callback_t* onMouseUp) {
    Arena *arena = priv;
    RenderDropdownMenuItem(
        CLAY_STRING("New"), (ItemData){ 3, 0 }, nullptr, nullptr
    );
    RenderDropdownMenuItem(
        CLAY_STRING("Open"), (ItemData){ 3, 1 }, nullptr, nullptr
    );
    RenderDropdownMenuItem(
        CLAY_STRING("Close"), (ItemData){ 3, 2 }, nullptr, nullptr
    );
}

void RenderCreateMenu(void *priv, Callback_t* onMouseUp) {
    InterfaceData *data = priv;
    HandleChangeModeData *modes =
        arena_allocate(&data->arena, 2, sizeof(HandleChangeModeData));
    for (int i = 0; i < 2; i++) {
        modes[i].stickType = &data->rendererData.stickType;
    }
    modes[0].stickTypeRequested = STICKFIGURE_STICK;
    RenderDropdownMenuItem(
        CLAY_STRING("Stick"),
        (ItemData){ 2, 0 },
        CallbackChain(&data->arena, onMouseUp, HandleChangeMode, &modes[0]),
        &data->context
    );
    modes[1].stickTypeRequested = STICKFIGURE_RING;
    RenderDropdownMenuItem(
        CLAY_STRING("Circle"),
        (ItemData){ 2, 1 },
        CallbackChain(&data->arena, onMouseUp, HandleChangeMode, &modes[1]),
        &data->context
    );
}

void HandleChangeStickType(StickfigurePartType requested, StickfigurePartType* type) {
    printf("Set %d -> %d\n", *type, requested);
    *type = requested;
}

void HandleSelectStickfigure(unsigned requested, unsigned* type) {
    printf("Select %d -> %d\n", *type, requested);
    *type = requested;
}


Clay_RenderCommandArray InterfaceLayout(InterfaceData *data) {
    arena_reset(&data->arena);
    Clay_BeginLayout();

    Clay_Sizing layoutExpand = { .width = CLAY_SIZING_GROW(0),
                                 .height = CLAY_SIZING_GROW(0) };

    // Build UI here
    CLAY({
        .id = CLAY_ID("OuterContainer"),
        .backgroundColor = { 43, 41, 51, 255 },
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = layoutExpand,
            /* .padding = CLAY_PADDING_ALL(16), */
            /* .childGap = 16, */
        },
    }) {
        // Child elements go inside braces
        CLAY({
            .id = CLAY_ID("HeaderBar"),
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .height = CLAY_SIZING_FIT(60),
                    .width = CLAY_SIZING_GROW(0),
                },
                .padding = { 16, 16, 0, 0 },
                .childGap = 16,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
             },
            .backgroundColor = SECONDARY_COLOR,
            .cornerRadius = {0.f, 0.f, 8.f, 8.f}
        }) {
            CLAY({
                .id = CLAY_ID("MenuBar"),
             }) {
                RenderMenuBarButton(
                    CLAY_STRING("File"),
                    CLAY_ID("FileButton"),
                    CLAY_ID("FileMenu"),
                    &data->isMenuBarButtonOpen[0],
                    RenderFileMenu,
                    &data->arena,
                    &data->context
                );
                RenderMenuBarButton(
                    CLAY_STRING("Create"),
                    CLAY_ID("CreateButton"),
                    CLAY_ID("CreateMenu"),
                    &data->isMenuBarButtonOpen[1],
                    RenderCreateMenu,
                    data,
                    &data->context
                );
            }

            CLAY({
                .id = CLAY_ID("ToolBar"),
                .layout = { .padding = CLAY_PADDING_ALL(8) }
            }) {
                RenderIconButton(
                    CLAY_ID("CreateStickfigureIcon"),
                    &data->icons[ICON_CREATE_STICKFIGURE],
                    CallbackCreate(&data->arena, (CallbackFn)HandleCreateStickfigure, &data->rendererData),
                    &data->context
                );
                RenderIconButtonGroup(
                    CLAY_ID("ShapeIconGroup"),
                    &data->icons[ICON_STICK],
                    CallbackCreateGroup(&data->arena, (CallbackIndexFn)HandleChangeStickType, 2, &data->rendererData.stickType),
                    2,
                    data->rendererData.stickType,
                    &data->context);
            }
        }


        CLAY(
            {
                .id = CLAY_ID("LowerContent"),
                .layout = { .sizing = layoutExpand },
            }
        ) {
            CLAY(
                { .id = CLAY_ID("Sidebar"),
                  .backgroundColor = SECONDARY_COLOR,
                  .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM,
                              .padding = CLAY_PADDING_ALL(16),
                              .childGap = 8,
                              .sizing = { .width = CLAY_SIZING_FIXED(250),
                                          .height = CLAY_SIZING_GROW(0) } } }
            ) {
                for (unsigned i = 0; i < data->rendererData.stickfigure.length; i++) {
                    Stickfigure* s = &data->rendererData.stickfigure.data[i];
                    Callback_t* cb = CallbackCreateGroup(&data->arena, (CallbackIndexFn)HandleSelectStickfigure, data->rendererData.stickfigure.length, &data->selectedStickfigure);
                    Clay_String name = { strlen(s->name), s->name };
                    bool selected = (int)i == data->selectedStickfigure;

                    CLAY({
                        .layout = {
                            .sizing = { .width = CLAY_SIZING_GROW(0) },
                            .padding = CLAY_PADDING_ALL(16)
                        },
                        .backgroundColor =
                            (Clay_Color){
                                120, 120, 120, selected ? 255 : (Clay_Hovered() ? 120 : 0) },
                        .cornerRadius = CLAY_CORNER_RADIUS(8) }
                    ) {
                        if(!selected)
                            SetButtonCallbacks(&data->arena, (ButtonData) { .onMouseUp = CALLBACK_INDEX(cb, i)} );
                        CLAY_TEXT(
                            name,
                            CLAY_TEXT_CONFIG(
                                { .fontId = FONT_ID_BODY_32,
                                    .fontSize = 20,
                                    .textColor = { 255, 255, 255, 255 } }
                            )
                        );
                    }
                }
            }
            CLAY(
                { .layout = {
                      .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                      .padding = CLAY_PADDING_ALL(16) } }
            ) {
                RenderCanvas(CLAY_ID("canvas"), CanvasEventHandler, &data->rendererData, renderer_get_frame(renderer_context));
            }
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y +=
            data->yOffset;
    }
    return renderCommands;
}

void InterfaceDeinit(InterfaceData* data) {
    for(unsigned i = 0; i < ICON_COUNT; i++) {
        UnloadTexture(data->icons[i]);
    }
    arena_free(&data->arena);
    free(data);
}
