#pragma once
#include "asset-loader.hpp"
#include "mesh/mesh.hpp"
namespace portal {
    // Loading screen has
    class Application;
    class TexturedMaterial;
    class TintedMaterial;
    class Texture2D;
    class LoadingScreen {
        static inline TexturedMaterial *menuMaterial = nullptr;
        static inline Mesh *rectangle = nullptr;
        static inline TintedMaterial* progressMaterial = nullptr;
        static inline glm::ivec2 size;
        static inline glm::mat4 VP;
        static inline glm::mat4 menuModelMatrix;
        static inline Texture2D *menutexture = nullptr;
        // The load config lambda function 
        // This allows having a custom "Loading Function"
        // that will get executed in parallel
        static inline std::function<void()> multithreadedload = nullptr;
        // The callback lambda function
        // this will get called at the end of render function
        static inline std::function<void()> callback = nullptr;
        static inline Application* app = nullptr;

        // Holds mesh data of .obj files with its name as key
        static inline std::unordered_map<std::string, std::pair<std::vector<portal::Vertex>*, std::vector<GLuint>*>> meshData;
        // Loops on meshData and call new Mesh() to be added
        // to AssetLoader<Mesh>::assets
        static void fillAssetLoader();
        // Gets called at the end of render() to delete LoadingScreen assets
        static void cleanUp();
    public:
        static inline std::atomic<int> progress = 0; // Progress of the application (used by the loading screen)
        static inline std::atomic<int> total = 0; // Total number of assets to load (used by the loading screen)
        static inline std::atomic<bool> doneLoading = false; // Is the application done loading (used by the loading screen)

        // Initializes the loading screen
        // Loads screen, progress bar, and some assets to be displayed
        // - multithreadedload: lambda function to be called in parallel during loading screen
        //                      and is called at the beginning of LoadingScreen::render()
        //          o Should update LoadingScreen::progress
        //          o Should set LoadingScreen::doneLoading to true when done
        // - countAssets: lambda function to be called to calculate the LoadingScreen::total
        //                and is called at the end of LoadingScreen::init()
        //          o Should update LoadingScreen::total
        //          o Defaults to LoadingScreen::countTotalAssets(app->getConfig()["scene"]["assets"])
        // - callback: lambda function to be called at the end of LoadingScreen::render()
        //          o Defaults to moving loaded meshData to AssetLoader<Mesh>::assets
        static void init(Application* app, std::function<void()> multithreadedload, std::function<void()> computeTotal = nullptr, std::function<void()> callback = nullptr);
        // Handles loop to render loading screen
        // (gets called in main thread)
        static void render();

        // Default computeTotal
        // Pre-computes total of assets to load for progress bar
        static inline void countTotalAssets(const nlohmann::json& assetData) {
            if(!assetData.is_object()) return;
            if(assetData.contains("shaders"))
                total += (int)assetData["shaders"].size();
            if(assetData.contains("textures"))
                total += (int)assetData["textures"].size();
            if(assetData.contains("samplers"))
                total += (int)assetData["samplers"].size();
            if(assetData.contains("meshes"))
                total += (int)assetData["meshes"].size();
            if(assetData.contains("materials"))
                total += (int)assetData["materials"].size();
            if(assetData.contains("models")) 
                total += (int)assetData["models"].size();
        }
        // Loops and loads Obj data (gets called in separate thread in asset-loader.cpp)
        static void deserializeMesh(const nlohmann::json &data);
    };
}