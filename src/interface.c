#include "interface.h"
#include "arena.h"
#include "components/components.h"
#include "components/utils.h"
#include "pivot.h"
#include "raylib.h"
#include "renderer/renderer.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern RendererContext* renderer_context;

typedef struct {
    Clay_String title;
    Clay_String contents;
} Document;

typedef struct {
    Document *documents;
    uint32_t length;
} DocumentArray;

Document documentsRaw[5];

DocumentArray documents = { .length = 5, .documents = documentsRaw };

typedef struct {
    int32_t requestedDocumentIndex;
    int32_t *selectedDocumentIndex;
} SidebarClickData;

void HandleSidebarInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    SidebarClickData *clickData = (SidebarClickData *)userData;
    // If this button was clicked
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (clickData->requestedDocumentIndex >= 0
            && clickData->requestedDocumentIndex < documents.length) {
            // Select the corresponding document
            *clickData->selectedDocumentIndex =
                clickData->requestedDocumentIndex;
        }
    }
}

MainLayoutData MainLayout_Initialize() {
    documents.documents[0] = (Document){
        .title = CLAY_STRING("Squirrels"),
        .contents = CLAY_STRING(
            "The Secret Life of Squirrels: Nature's Clever Acrobats\n"
            "Squirrels are often overlooked creatures, dismissed as mere park "
            "inhabitants or backyard nuisances. Yet, beneath their fluffy "
            "tails and twitching noses lies an intricate world of cunning, "
            "agility, and survival tactics that are nothing short of "
            "fascinating. As one of the most common mammals in North America, "
            "squirrels have adapted to a wide range of environments from "
            "bustling urban centers to tranquil forests and have developed a "
            "variety of unique behaviors that continue to intrigue scientists "
            "and nature enthusiasts alike.\n"
            "\n"
            "Master Tree Climbers\n"
            "At the heart of a squirrel's skill set is its impressive ability "
            "to navigate trees with ease. Whether they're darting from branch "
            "to branch or leaping across wide gaps, squirrels possess an "
            "innate talent for acrobatics. Their powerful hind legs, which are "
            "longer than their front legs, give them remarkable jumping power. "
            "With a tail that acts as a counterbalance, squirrels can leap "
            "distances of up to ten times the length of their body, making "
            "them some of the best aerial acrobats in the animal kingdom.\n"
            "But it's not just their agility that makes them exceptional "
            "climbers. Squirrels' sharp, curved claws allow them to grip tree "
            "bark with precision, while the soft pads on their feet provide "
            "traction on slippery surfaces. Their ability to run at high "
            "speeds and scale vertical trunks with ease is a testament to the "
            "evolutionary adaptations that have made them so successful in "
            "their arboreal habitats.\n"
            "\n"
            "Food Hoarders Extraordinaire\n"
            "Squirrels are often seen frantically gathering nuts, seeds, and "
            "even fungi in preparation for winter. While this behavior may "
            "seem like instinctual hoarding, it is actually a survival "
            "strategy that has been honed over millions of years. Known as "
            "\"scatter hoarding,\" squirrels store their food in a variety of "
            "hidden locations, often burying it deep in the soil or stashing "
            "it in hollowed-out tree trunks.\n"
            "Interestingly, squirrels have an incredible memory for the "
            "locations of their caches. Research has shown that they can "
            "remember thousands of hiding spots, often returning to them "
            "months later when food is scarce. However, they don't always "
            "recover every stash some forgotten caches eventually sprout into "
            "new trees, contributing to forest regeneration. This "
            "unintentional role as forest gardeners highlights the ecological "
            "importance of squirrels in their ecosystems.\n"
            "\n"
            "The Great Squirrel Debate: Urban vs. Wild\n"
            "While squirrels are most commonly associated with rural or wooded "
            "areas, their adaptability has allowed them to thrive in urban "
            "environments as well. In cities, squirrels have become adept at "
            "finding food sources in places like parks, streets, and even "
            "garbage cans. However, their urban counterparts face unique "
            "challenges, including traffic, predators, and the lack of natural "
            "shelters. Despite these obstacles, squirrels in urban areas are "
            "often observed using human infrastructure such as buildings, "
            "bridges, and power lines as highways for their acrobatic "
            "escapades.\n"
            "There is, however, a growing concern regarding the impact of "
            "urban life on squirrel populations. Pollution, deforestation, and "
            "the loss of natural habitats are making it more difficult for "
            "squirrels to find adequate food and shelter. As a result, "
            "conservationists are focusing on creating squirrel-friendly "
            "spaces within cities, with the goal of ensuring these resourceful "
            "creatures continue to thrive in both rural and urban landscapes.\n"
            "\n"
            "A Symbol of Resilience\n"
            "In many cultures, squirrels are symbols of resourcefulness, "
            "adaptability, and preparation. Their ability to thrive in a "
            "variety of environments while navigating challenges with agility "
            "and grace serves as a reminder of the resilience inherent in "
            "nature. Whether you encounter them in a quiet forest, a city "
            "park, or your own backyard, squirrels are creatures that never "
            "fail to amaze with their endless energy and ingenuity.\n"
            "In the end, squirrels may be small, but they are mighty in their "
            "ability to survive and thrive in a world that is constantly "
            "changing. So next time you spot one hopping across a branch or "
            "darting across your lawn, take a moment to appreciate the "
            "remarkable acrobat at work a true marvel of the natural world.\n"
        )
    };
    documents.documents[1] = (Document){
        .title = CLAY_STRING("Lorem Ipsum"),
        .contents = CLAY_STRING(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
            "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut "
            "enim ad minim veniam, quis nostrud exercitation ullamco laboris "
            "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
            "reprehenderit in voluptate velit esse cillum dolore eu fugiat "
            "nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
            "sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
    };
    documents.documents[2] = (Document){
        .title = CLAY_STRING("Vacuum Instructions"),
        .contents = CLAY_STRING(
            "Chapter 3: Getting Started - Unpacking and Setup\n"
            "\n"
            "Congratulations on your new SuperClean Pro 5000 vacuum cleaner! "
            "In this section, we will guide you through the simple steps to "
            "get your vacuum up and running. Before you begin, please ensure "
            "that you have all the components listed in the \"Package "
            "Contents\" section on page 2.\n"
            "\n"
            "1. Unboxing Your Vacuum\n"
            "Carefully remove the vacuum cleaner from the box. Avoid using "
            "sharp objects that could damage the product. Once removed, place "
            "the unit on a flat, stable surface to proceed with the setup. "
            "Inside the box, you should find:\n"
            "\n"
            "    The main vacuum unit\n"
            "    A telescoping extension wand\n"
            "    A set of specialized cleaning tools (crevice tool, upholstery "
            "brush, etc.)\n"
            "    A reusable dust bag (if applicable)\n"
            "    A power cord with a 3-prong plug\n"
            "    A set of quick-start instructions\n"
            "\n"
            "2. Assembling Your Vacuum\n"
            "Begin by attaching the extension wand to the main body of the "
            "vacuum cleaner. Line up the connectors and twist the wand into "
            "place until you hear a click. Next, select the desired cleaning "
            "tool and firmly attach it to the wand's end, ensuring it is "
            "securely locked in.\n"
            "\n"
            "For models that require a dust bag, slide the bag into the "
            "compartment at the back of the vacuum, making sure it is properly "
            "aligned with the internal mechanism. If your vacuum uses a "
            "bagless system, ensure the dust container is correctly seated and "
            "locked in place before use.\n"
            "\n"
            "3. Powering On\n"
            "To start the vacuum, plug the power cord into a grounded "
            "electrical outlet. Once plugged in, locate the power switch, "
            "usually positioned on the side of the handle or body of the unit, "
            "depending on your model. Press the switch to the \"On\" position, "
            "and you should hear the motor begin to hum. If the vacuum does "
            "not power on, check that the power cord is securely plugged in, "
            "and ensure there are no blockages in the power switch.\n"
            "\n"
            "Note: Before first use, ensure that the vacuum filter (if your "
            "model has one) is properly installed. If unsure, refer to "
            "\"Section 5: Maintenance\" for filter installation instructions."
        )
    };
    documents.documents[3] = (Document){ .title = CLAY_STRING("Article 4"),
                                         .contents = CLAY_STRING("Article 4") };
    documents.documents[4] = (Document){ .title = CLAY_STRING("Article 5"),
                                         .contents = CLAY_STRING("Article 5") };

    const size_t COMPONENT_ARENA_SIZE = 4096;
    void *memory = malloc(COMPONENT_ARENA_SIZE);
    assert(memory);
    MainLayoutData data = {
        .arena = arena_init(COMPONENT_ARENA_SIZE, memory),
        .rendererData = {
            .pivotRadius = 1.f
        }
    };
    
    const char filename[512];
    for(unsigned i = 0; i < ICON_COUNT; i++) {
        snprintf(filename, 512, RESOURCE_PATH "resources/icons/%s", ICON_FILENAMES[i]);
        data.icons[i] = LoadTexture(filename);
    }
    return data;
}

void HandleCreateStickfigure(RendererData* data) {
    Clay_ElementData canvas = Clay_GetElementData(Clay_GetElementId(CLAY_STRING("canvas")));
    printf("HandleCreateStickfigure (%f, %f, %f, %f)\n", canvas.boundingBox.x, canvas.boundingBox.y, canvas.boundingBox.width, canvas.boundingBox.height);
    Vector2 pivot = renderer_get_world_position(renderer_context, (Vector2){ canvas.boundingBox.width / 2.f, canvas.boundingBox.height / 2.f}, (Vector2) { canvas.boundingBox.width, canvas.boundingBox.height });
    PivotCreateStickfigure(&data->stickfigure, data->stickType, (Vector2) { pivot.x, pivot.y - 5.f }, (Vector2) { pivot.x, pivot.y + 5.f });
}

typedef struct {
    EditMode *mode;
    StickfigurePartType* stickType;
    EditMode modeRequested;
    StickfigurePartType stickTypeRequested;
} HandleChangeModeData;

void HandleChangeMode(void *data) {
    HandleChangeModeData *modedata = data;
    *modedata->mode = modedata->modeRequested;
    if(modedata->modeRequested == EDIT)
        *modedata->stickType = modedata->stickTypeRequested;
    printf("New mode: %d\n", *modedata->mode);
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
    MainLayoutData *data = priv;
    HandleChangeModeData *modes =
        arena_allocate(&data->arena, 2, sizeof(HandleChangeModeData));
    for (int i = 0; i < 2; i++) {
        modes[i].mode = &data->rendererData.mode;
        modes[i].stickType = &data->rendererData.stickType;
        modes[i].modeRequested = EDIT;
    }
    modes[0].stickTypeRequested = STICKFIGURE_STICK;
    RenderDropdownMenuItem(
        CLAY_STRING("Stick"),
        (ItemData){ 2, 0 },
        CallbackChain(&data->arena, onMouseUp, HandleChangeMode, &modes[0]),
        &data->arena
    );
    modes[1].stickTypeRequested = STICKFIGURE_RING;
    RenderDropdownMenuItem(
        CLAY_STRING("Circle"),
        (ItemData){ 2, 1 },
        CallbackChain(&data->arena, onMouseUp, HandleChangeMode, &modes[1]),
        &data->arena
    );
}

typedef struct {
    StickfigurePartType* type;
    StickfigurePartType requested;
} HandleChangeStickTypeData;

void HandleChangeStickType(HandleChangeStickTypeData* data) {
    *data->type = data->requested;
}

Clay_RenderCommandArray MainLayout_CreateLayout(MainLayoutData *data) {
    arena_reset(&data->arena);
    HandleChangeStickTypeData* modes = arena_allocate(&data->arena, 2, sizeof(HandleChangeStickTypeData));
    for (unsigned i = 0; i < 2; i++) {
        modes[i].type = &data->rendererData.stickType;
        modes[i].requested = i;
    }

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
                    &data->arena
                );
                RenderMenuBarButton(
                    CLAY_STRING("Create"),
                    CLAY_ID("CreateButton"),
                    CLAY_ID("CreateMenu"),
                    &data->isMenuBarButtonOpen[1],
                    RenderCreateMenu,
                    data,
                    &data->arena
                );
            }

            CLAY({
                .id = CLAY_ID("ToolBar"),
                .layout = { .padding = CLAY_PADDING_ALL(8) }
            }) {
                RenderIconButton(
                    CLAY_ID("CreateStickfigureIcon"),
                    &data->arena,
                    &data->icons[ICON_CREATE_STICKFIGURE],
                    CallbackCreate(&data->arena, (CallbackFn)HandleCreateStickfigure, &data->rendererData)
                );
                RenderIconButton(
                    CLAY_ID("StickIcon"),
                    &data->arena,
                    &data->icons[ICON_STICK],
                    CallbackCreate(&data->arena, (CallbackFn)HandleChangeStickType, &modes[0])
                );
                RenderIconButton(
                    CLAY_ID("RingIcon"),
                    &data->arena,
                    &data->icons[ICON_RING],
                    CallbackCreate(&data->arena, (CallbackFn)HandleChangeStickType, &modes[1])
                );
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
                for (int i = 0; i < documents.length; i++) {
                    Document document = documents.documents[i];
                    Clay_LayoutConfig sidebarButtonLayout = {
                        .sizing = { .width = CLAY_SIZING_GROW(0) },
                        .padding = CLAY_PADDING_ALL(16)
                    };

                    if (i == data->selectedDocumentIndex) {
                        CLAY(
                            { .layout = sidebarButtonLayout,
                              .backgroundColor = { 120, 120, 120, 255 },
                              .cornerRadius = CLAY_CORNER_RADIUS(8) }
                        ) {
                            CLAY_TEXT(
                                document.title,
                                CLAY_TEXT_CONFIG(
                                    { .fontId = FONT_ID_BODY_32,
                                      .fontSize = 20,
                                      .textColor = { 255, 255, 255, 255 } }
                                )
                            );
                        }
                    } else {
                        SidebarClickData *clickData = arena_allocate(
                            &data->arena, 1, sizeof(SidebarClickData)
                        );
                        *clickData = (SidebarClickData){
                            .requestedDocumentIndex = i,
                            .selectedDocumentIndex =
                                &data->selectedDocumentIndex
                        };
                        CLAY(
                            { .layout = sidebarButtonLayout,
                              .backgroundColor =
                                  (Clay_Color){
                                      120, 120, 120, Clay_Hovered() ? 120 : 0 },
                              .cornerRadius = CLAY_CORNER_RADIUS(8) }
                        ) {
                            Clay_OnHover(
                                HandleSidebarInteraction, (intptr_t)clickData
                            );
                            CLAY_TEXT(
                                document.title,
                                CLAY_TEXT_CONFIG(
                                    { .fontId = FONT_ID_BODY_32,
                                      .fontSize = 20,
                                      .textColor = { 255, 255, 255, 255 } }
                                )
                            );
                        }
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
