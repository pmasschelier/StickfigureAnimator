#include "clay/clay.h"
#include "components.h"
#include "raylib.h"
#include "raymath.h"
#include "utils.h"

#include <stdio.h>

typedef struct {
    OnChangeColor onChangeColor;
    ColorHSV* color;
} ColorPickerHandlerData;

static void HandleHoverHPicker(
    [[maybe_unused]] Clay_ElementId elementId,
    Clay_PointerData pointerInfo,
    intptr_t userData
) {
    const Clay_BoundingBox bb = Clay_GetElementData(elementId).boundingBox;
    const ColorPickerHandlerData* data = (void*)userData;
    float offset;
    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_PRESSED:
        offset = pointerInfo.position.x - bb.x;
        data->color->hue = offset / bb.width * 360.f;
        data->onChangeColor.fn(*data->color, data->onChangeColor.data);
        break;
    default:
        break;
    }
}

static void HandleHoverSVPicker(
    [[maybe_unused]] Clay_ElementId elementId,
    Clay_PointerData pointerInfo,
    intptr_t userData
) {
    const Clay_BoundingBox bb = Clay_GetElementData(elementId).boundingBox;
    const ColorPickerHandlerData* data = (void*)userData;
    Vector2 offset;
    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_PRESSED:
        offset = (Vector2) {
            pointerInfo.position.x - bb.x,
            pointerInfo.position.y - bb.y,
        };
        data->color->saturation = offset.x / bb.width;
        data->color->value = 1.f - (offset.y / bb.height);
        printf("saturation: %f, value %f\n", data->color->saturation, data->color->value);
        data->onChangeColor.fn(*data->color, data->onChangeColor.data);
        break;
    default:
        break;
    }
}

void RenderColorPicker(Clay_String id, ColorPickerState* state, OnChangeColor onChangeColor, ComponentContext* context) {
    if (state->sv_shader.id == 0) state->sv_shader = LoadShader(nullptr, RESOURCE_PATH "resources/shaders/colorpicker.frag");
    if (state->h_shader.id == 0) state->h_shader = LoadShader(nullptr, RESOURCE_PATH "resources/shaders/huepicker.frag");

    const Clay_ElementId sv_pickerId = Clay_GetElementIdWithIndex(id, 1);
    SetShaderValue(state->sv_shader, GetShaderLocation(state->sv_shader, "color"), &state->color, SHADER_UNIFORM_VEC3);
    SetShaderValue(state->h_shader, GetShaderLocation(state->h_shader, "color"), &state->color, SHADER_UNIFORM_VEC3);

    const Clay_ElementId h_pickerId = Clay_GetElementIdWithIndex(id, 2);

    ColorPickerHandlerData* data = arena_allocate(context->arena, 1, sizeof(ColorPickerHandlerData));
    data->color = &state->color;
    data->onChangeColor = onChangeColor;

    CLAY({
        .id = Clay_GetElementId(id),
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 16,
            .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}
        },
    }) {
        CLAY() {
            RenderShaderImage(sv_pickerId, (Clay_Sizing) { CLAY_SIZING_GROW(200, 400), CLAY_SIZING_FIXED(200) }, &state->sv_image, state->sv_shader);
            Clay_OnHover(HandleHoverSVPicker, (intptr_t)data);
        }
        CLAY() {
            RenderShaderImage(h_pickerId, (Clay_Sizing) { CLAY_SIZING_GROW(200, 400), CLAY_SIZING_FIXED(40) }, &state->h_image, state->h_shader);
            Clay_OnHover(HandleHoverHPicker, (intptr_t)data);
        }
    }
}