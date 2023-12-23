#pragma once
#include "asset-loader.hpp"
#include "mesh/mesh.hpp"
class Playstate;
// struct Button;
#include "../states/menu-state.hpp"

namespace portal {
    class Application;
    class TexturedMaterial;
    class TintedMaterial;
    class Texture2D;
    
    class ForwardRenderer;
    class World;
    // class Playstate;
    
    class PauseMenu {
        static inline TexturedMaterial *pauseMaterial = nullptr;
        static inline TexturedMaterial *optionsMaterial = nullptr;
        static inline TintedMaterial *highlightMaterial = nullptr;
        static inline Mesh *rectangle = nullptr;

        static inline glm::ivec2 size;
        static inline glm::mat4 VP;
        static inline Application* app = nullptr;
        static inline glm::mat4 menuModelMatrix;
        static inline glm::mat4 optionsModelMatrix;
        static inline Playstate* playstate = nullptr;

        static inline glm::vec2 mousePosition;

        static inline std::vector<Button> pauseButtons;
        static inline std::vector<Button> optionsButtons;

        static inline bool pause = false;
        static inline bool options = false;

        static void renderOptionsMenu();
        static void renderPauseMenu();


    public:

        static void init(Application* app, ForwardRenderer* renderer, Playstate* playstate, World* world);
        static bool render();
        static void cleanUp();
        static void unPause();
        static void pauseGame();
        static void quit();
        static void openOptions();
    };
}

