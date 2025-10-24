#include "clay/clay.h"
#include "components.h"
#include "raylib.h"
#include "raymath.h"
#include "utils.h"

#include <stdio.h>

RenderTexture sv_image, h_image;
Shader sv_shader, h_shader;
ColorHSV color;

static void HandleHoverHPicker(
    [[maybe_unused]] Clay_ElementId elementId,
    Clay_PointerData pointerInfo,
    intptr_t userData
) {
    Clay_ElementData data = Clay_GetElementData(elementId);
    ColorHSV* color = (void*)userData;
    float offset;
    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_PRESSED:
        offset = pointerInfo.position.x - data.boundingBox.x;
        color->hue = offset / data.boundingBox.width * 360.f;
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
    Clay_ElementData data = Clay_GetElementData(elementId);
    ColorHSV* color = (void*)userData;
    Vector2 offset;
    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_PRESSED:
        offset = (Vector2) {
            pointerInfo.position.x - data.boundingBox.x,
            pointerInfo.position.y - data.boundingBox.y,
        };
        color->saturation = offset.x / data.boundingBox.width;
        color->value = 1.f - (offset.y / data.boundingBox.height);
        printf("saturation: %f, value %f\n", color->saturation, color->value);
        break;
    default:
        break;
    }
}


void RenderColorPicker(Clay_String id, ComponentContext* context) {
    if (sv_shader.id == 0) sv_shader = LoadShader(nullptr, RESOURCE_PATH "resources/shaders/colorpicker.frag");
    if (h_shader.id == 0) h_shader = LoadShader(nullptr, RESOURCE_PATH "resources/shaders/huepicker.frag");

    const Clay_ElementId sv_pickerId = Clay_GetElementIdWithIndex(id, 1);
    SetShaderValue(sv_shader, GetShaderLocation(sv_shader, "color"), &color, SHADER_UNIFORM_VEC3);
    SetShaderValue(h_shader, GetShaderLocation(h_shader, "color"), &color, SHADER_UNIFORM_VEC3);

    const Clay_ElementId h_pickerId = Clay_GetElementIdWithIndex(id, 2);

    CLAY({
        .id = Clay_GetElementId(id),
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 16,
            .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}
        },
    }) {
        CLAY() {
            RenderShaderImage(sv_pickerId, (Clay_Sizing) { CLAY_SIZING_GROW(200, 400), CLAY_SIZING_FIXED(200) }, &sv_image, sv_shader);
            Clay_OnHover(HandleHoverSVPicker, (intptr_t)&color);
        }
        CLAY() {
            RenderShaderImage(h_pickerId, (Clay_Sizing) { CLAY_SIZING_GROW(200, 400), CLAY_SIZING_FIXED(40) }, &h_image, h_shader);
            Clay_OnHover(HandleHoverHPicker, (intptr_t)&color);
        }
    }
}