#ifndef INTERFACE_H
#define INTERFACE_H

#include "arena.h"
#include "clay/clay.h"
#include "components/utils.h"
#include "raylib.h"
#include "src/pivot.h"
#include <stdint.h>

constexpr int FONT_ID_BODY_32 = 0;
constexpr int FONT_ID_BODY_24 = 1;

constexpr Clay_Color PRIMARY_COLOR = { 120, 120, 120, 255 };
constexpr Clay_Color SECONDARY_COLOR = { 90, 90, 90, 255 };

typedef enum {
    NORMAL,
    CREATE_STICK,
    MOVE_STICK,
    MOVE_STICKFIGURE,
} EditMode;

constexpr unsigned MENUBAR_BUTTON_COUNT = 5;

typedef struct {
    double angle;
    double length;
} PolarCoords;

typedef struct {
    enum {
        HAND_EMPTY,
        HAND_HOLDING_STICK,
        HAND_HOLDING_STICKFIGURE,
        HAND_SELECT_EDGES
    } status;
    union {
        struct {
            PivotEdgeIndex edge;
            PolarCoords initial;
            PolarCoords pointerOffset;
        } edge;
        struct {
            unsigned int figure;
            Vector2 initialStickfigure;
            Vector2 initialPointer;
        } figure;
        struct {
            Vector2 start;
        } selection;
    };
} HandData;

typedef struct {
    Stickfigure_array_t stickfigure;
    HandData hand;
    EditMode mode;
    StickfigurePartType stickType;
    Vector2 offset;
    float zoom;
    float pivotRadius;
} RendererData;


typedef enum {
    ICON_CREATE_STICKFIGURE,
    ICON_STICK,
    ICON_RING,
    ICON_COUNT,
} IconName;

constexpr char ICON_FILENAMES[ICON_COUNT][128] = {
    "create-stickfigure.png",
    "stick.png",
    "ring.png",
};

typedef struct {
    int selectedStickfigure;
    float yOffset;
    Arena arena;
    bool isMenuBarButtonOpen[MENUBAR_BUTTON_COUNT];
    RendererData rendererData;
    Texture2D icons[ICON_COUNT];
    ComponentContext context;
} InterfaceData;

void CanvasEventHandler(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

InterfaceData* InterfaceInit();
void InterfaceDeinit(InterfaceData* data);
Clay_RenderCommandArray InterfaceLayout(InterfaceData *data);

#endif // !INTERFACE_H
