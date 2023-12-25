#pragma once
#include "../states/menu-state.hpp"

namespace portal {
    class ForwardRenderer;
    
    class PauseMenu {
        static inline Application* app = nullptr;
        static inline ForwardRenderer* renderer = nullptr;

        static inline TexturedMaterial *pauseMaterial = nullptr;
        static inline TexturedMaterial *optionsMaterial = nullptr;
        static inline TintedMaterial *highlightMaterial = nullptr;
        static inline Mesh *rectangle = nullptr;

        static inline glm::ivec2 size;
        static inline glm::mat4 VP;
        static inline glm::mat4 menuModelMatrix;
        static inline glm::mat4 optionsModelMatrix;

        static inline glm::vec2 mousePosition;

        static inline std::vector<Button> pauseButtons;
        static inline std::vector<Button> optionsButtons;
        static inline std::vector<bool> optionsStates;

        static inline bool pause = false;
        static inline bool options = false;

        static void renderOptionsMenu();
        static void renderPauseMenu();


    public:
        static void init(Application* app, ForwardRenderer* renderer);
        static bool render();
        static void cleanUp();
        static void unPause();
        static void pauseGame();
        static void quit();
        static void openOptions();
        static void toggleBloom();
    };
}

