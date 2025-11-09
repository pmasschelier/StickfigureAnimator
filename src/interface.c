#include "interface.h"
#include <cutils/arena.h>
#include "components/callback.h"
#include "components/components.h"
#include "components/utils.h"
#include "pivot.h"
#include "raylib.h"
#include "renderer/renderer.h"

#include <alloca.h>
#include <commands.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern RendererContext* renderer_context;

InterfaceData* InterfaceInit() {
    InterfaceData* data = malloc(sizeof(InterfaceData));
    *data = (InterfaceData){
        .arena = ARENA_INIT,
        .rendererData = {
            .thickness = 1.f,
            .pivotRadius = 6.f,
            .color = &data->colorPicker.color,
        },
        .colorPicker = {
            .color = {0.f, 0.f, 0.f}
        },
        .context = {
            .allocator = arena_get_allocator(&data->arena),
            .pointer = POINTER_DEFAULT,
            .theme = {
                .color = { PRIMARY_COLOR, (Clay_Color){}, SECONDARY_COLOR, (Clay_Color){} },
            },
            .selected_font = 0
        }
    };
    
    char filename[512];
    for(unsigned i = 0; i < ICON_COUNT; i++) {
        snprintf(filename, 512, RESOURCE_PATH "resources/icons/%s", ICON_FILENAMES[i]);
        data->icons[i] = LoadTexture(filename);
    }
    return data;
}

void HandleCreateStickfigure(CanvasData* data) {
    Clay_ElementData canvas = Clay_GetElementData(Clay_GetElementId(CLAY_STRING("canvas")));
    printf("HandleCreateStickfigure (%f, %f, %f, %f)\n", canvas.boundingBox.x, canvas.boundingBox.y, canvas.boundingBox.width, canvas.boundingBox.height);
    Vector2 pivot = renderer_get_world_position(renderer_context, (Vector2){ canvas.boundingBox.width / 2.f, canvas.boundingBox.height / 2.f}, (Vector2) { canvas.boundingBox.width, canvas.boundingBox.height });
    PivotEdgeData edgeData = {
        .type = data->stickType,
        .selected = false,
        .angle = PI / 2,
        .length = 10.f,
        .color = ColorFromHSV(data->color->hue, data->color->saturation, data->color->value),
        .thickness = data->thickness
    };
    Stickfigure* figure = alloca(SizeofStickfigure);
    char name[STICKFIGURE_NAME_LENGTH];
    snprintf(name, STICKFIGURE_NAME_LENGTH, "Stickfigure nº%d", data->stickfigure.length);
    if (PivotCreateFigure(figure, name, (Vector2) { pivot.x, pivot.y - 5.f }, edgeData)) {
        CommandPushCreateFigure(&data->stickfigure, figure, PivotTopZIndex(data->stickfigure));
        PivotClearFigure(figure);
    }
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
    InterfaceData *data = priv;
    RenderDropdownMenuItem(
        CLAY_STRING("New"), (ItemData){ 3, 0 }, nullptr, &data->context
    );
    RenderDropdownMenuItem(
        CLAY_STRING("Open"), (ItemData){ 3, 1 }, nullptr, &data->context
    );
    RenderDropdownMenuItem(
        CLAY_STRING("Close"), (ItemData){ 3, 2 }, nullptr, &data->context
    );
}

void RenderCreateMenu(void *priv, Callback_t* onMouseUp) {
    InterfaceData *data = priv;
    HandleChangeModeData *modes =
        arena_allocate_array(&data->arena, 2, sizeof(HandleChangeModeData));
    auto allocator = arena_get_allocator(&data->arena);
    for (int i = 0; i < 2; i++) {
        modes[i].stickType = &data->rendererData.stickType;
    }
    modes[0].stickTypeRequested = STICKFIGURE_STICK;
    RenderDropdownMenuItem(
        CLAY_STRING("Stick"),
        (ItemData){ 2, 0 },
        CallbackChain(&allocator, onMouseUp, HandleChangeMode, &modes[0]),
        &data->context
    );
    modes[1].stickTypeRequested = STICKFIGURE_RING;
    RenderDropdownMenuItem(
        CLAY_STRING("Circle"),
        (ItemData){ 2, 1 },
        CallbackChain(&allocator, onMouseUp, HandleChangeMode, &modes[1]),
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

typedef struct {
    Stickfigure_array_t* stickfigures;
} HandleChangeColorData;
void HandleChangeColor(HandleChangeColorData* data, ColorHSV color) {
    printf("Change color (%f, %f, %f)\n", color.hue, color.saturation, color.value);
    const Color rgb = ColorFromHSV(color.hue, color.saturation, color.value);
    PivotSetColorSelection(data->stickfigures, rgb);
}

Clay_RenderCommandArray InterfaceLayout(InterfaceData *data) {
    arena_reset(&data->arena);
    Clay_BeginLayout();

    const Clay_Sizing layoutExpand = { .width = CLAY_SIZING_GROW(0),
                                 .height = CLAY_SIZING_GROW(0) };

    allocator_t allocator = arena_get_allocator(&data->arena);

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
                    data,
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
                    CallbackCreate(&allocator, (CallbackFn)HandleCreateStickfigure, &data->rendererData),
                    &data->context
                );
                RenderIconButtonGroup(
                    CLAY_ID("ShapeIconGroup"),
                    &data->icons[ICON_STICK],
                    CallbackCreateGroup(&allocator, (CallbackIndexFn)HandleChangeStickType, 2, &data->rendererData.stickType),
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
            CLAY({
                .id = CLAY_ID("left-sidebar"),
                .backgroundColor = SECONDARY_COLOR,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 8,
                    .sizing = { CLAY_SIZING_FIXED(250), CLAY_SIZING_GROW(0) }
                }
            }) {
                for (unsigned i = 0; i < data->rendererData.stickfigure.length; i++) {
                    Stickfigure* s = get_figure(data->rendererData.stickfigure, i);
                    Callback_t* cb = CallbackCreateGroup(&allocator, (CallbackIndexFn)HandleSelectStickfigure, data->rendererData.stickfigure.length, &data->selectedStickfigure);
                    const char* name = PivotStickfigureName(s);
                    const Clay_String label = { (int32_t)strlen(name), name };
                    const bool selected = (int)i == data->selectedStickfigure;

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
                            SetButtonCallbacks(&allocator, (ButtonData) { .onMouseUp = CALLBACK_INDEX(cb, i)} );
                        CLAY_TEXT(
                            label,
                            CLAY_TEXT_CONFIG({
                                    .fontId = FONT_ID_BODY_32,
                                    .fontSize = 20,
                                    .textColor = { 255, 255, 255, 255 }
                            })
                        );
                    }
                }
            }
            CLAY(
                { .layout = {
                      .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                      .padding = CLAY_PADDING_ALL(16) } }
            ) {
                RenderCanvas(CLAY_ID("canvas"), CanvasEventHandler, &data->rendererData, renderer_get_frame(renderer_context), &data->context);
            }
            CLAY({
                .id = CLAY_ID("right-sidebar"),
                .backgroundColor = SECONDARY_COLOR,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 8,
                    .sizing = { CLAY_SIZING_FIT(250), CLAY_SIZING_GROW(0) }
                }
            }) {
                HandleChangeColorData* handleChangeColorData = arena_allocate(&data->arena, sizeof(HandleChangeColorData));
                handleChangeColorData->stickfigures = &data->rendererData.stickfigure;
                RenderColorPicker(
                    CLAY_STRING("ColorPicker"),
                    &data->colorPicker,
                    (OnChangeColor) { (OnChangeColorFn)HandleChangeColor, handleChangeColorData},
                    &data->context
                );
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
