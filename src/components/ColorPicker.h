#ifndef STICKFIGUREANIMATOR_COLORPICKER_H
#define STICKFIGUREANIMATOR_COLORPICKER_H

#include "raylib.h"
#include "utils.h"


typedef struct {
    float hue, saturation, value;
} ColorHSV;

typedef struct {
    RenderTexture sv_image, h_image;
    Shader sv_shader, h_shader;
    ColorHSV color;
} ColorPickerState;

typedef void (*OnChangeColorFn)(void*, ColorHSV);

typedef struct {
    OnChangeColorFn fn;
    void* data;
} OnChangeColor;

void RenderColorPicker(Clay_String id, ColorPickerState* state, OnChangeColor onChangeColor, ComponentContext* context);

#endif // STICKFIGUREANIMATOR_COLORPICKER_H
