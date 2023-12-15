#include "loading-screen.hpp"
#include "application.hpp"
#include "asset-loader.hpp"
#include "mesh/mesh-utils.hpp"
#include "mesh/mesh.hpp"
#include "material/material.hpp"
#include "texture/texture2d.hpp"
#include "texture/sampler.hpp"
#include "texture/texture-utils.hpp"
#include "shader/shader.hpp"
#include <thread>

namespace portal {
    void LoadingScreen::deserializeMesh(const nlohmann::json& data) {
        if(data.is_object()){
            for(auto& [name, desc] : data.items()){
                progress++;
                std::string path = desc.get<std::string>();
                meshData[name] = mesh_utils::loadOBJData(path);
            }
        }
    }

    void LoadingScreen::fillAssetLoader() {
        // loop on meshData and call new Mesh()
        for(auto& [name, data] : meshData){
            AssetLoader<Mesh>::assets[name] = new Mesh(*data.first, *data.second);
        }
    }

    void LoadingScreen::init(Application* app, std::function<void()> multithreadedload, std::function<void()> computeTotal, std::function<void()> callback) {
        doneLoading = false;
        progress = 0;
        total = 0;
        // Load the loading screen material
        menuMaterial = new TexturedMaterial();
        menuMaterial->shader = new ShaderProgram();
        menuMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        menuMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        menuMaterial->shader->link();
        menuMaterial->texture = texture_utils::loadImage("assets/textures/Loading.png");
        menuMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        // Load the progress bar material
        progressMaterial = new TintedMaterial();
        progressMaterial->shader = new ShaderProgram();
        progressMaterial->shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER);
        progressMaterial->shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER);
        progressMaterial->shader->link();
        progressMaterial->tint = glm::vec4(0.6666f, 0.83529f, 0.97254f, 1.0f);
        // Load the progress bar mesh
        rectangle = new portal::Mesh({
            {{0.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{0.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        },{
            0, 1, 2, 2, 3, 0,
        });
        LoadingScreen::app = app;
        // Set the size of the screen
        size = app->getFrameBufferSize();
        // Set the projection matrix to make the origin at the top-left corner of the screen
        VP = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        menuModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
        // Set the function to be called when the loading screen is done
        LoadingScreen::multithreadedload = multithreadedload;
        // To compute LoadingScreen::total
        // Default to countTotalAssets(config["assets"])
        if(computeTotal) computeTotal();
        else countTotalAssets(app->getConfig()["scene"]["assets"]);
        LoadingScreen::callback = callback;
    }

    void LoadingScreen::render() {
        std::thread loadingThread(LoadingScreen::multithreadedload);
        // Idk these value were just trial and error
        float minWidth = 0.0f;
        float maxWidth = size.x * 0.7f;
        float height = size.y * 0.09f;
        float x = size.x / 2.0f;
        float y = size.y * 0.729f;
        float curWidth = minWidth;
        while(!doneLoading) {
            glfwMakeContextCurrent(app->getWindow());
            // Clear the screen
            glViewport(0, 0, size.x, size.y);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glStencilMask(0xFF);
            glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            // Draw the menu
            menuMaterial->setup();
            menuMaterial->shader->set("transform", VP * menuModelMatrix);
            rectangle->draw();
            // Draw the progress bar
            progressMaterial->setup();
            // calculate model matrix based on LoadingScreen::progress / LoadingScreen::total percentage
            curWidth = minWidth + (maxWidth - minWidth) * ((float)progress / (float)total);
            curWidth = std::min(curWidth, size.x * 0.465f);
            x = (size.x - curWidth) / 2.0f;
            glm::mat4 progressM = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            progressM = glm::scale(progressM, glm::vec3(curWidth, height, 1.0f));
            progressMaterial->shader->set("transform", VP * progressM);
            rectangle->draw();
            // Swap the buffers
            glfwSwapBuffers(app->getWindow());
            // Poll events
            glfwPollEvents();
        }
        loadingThread.join();
        if(callback) callback();
        else {
            fillAssetLoader();
        }
        cleanUp();
    }

    void LoadingScreen::cleanUp() {
        delete menuMaterial->shader;
        delete menuMaterial->texture;
        delete menuMaterial;
        delete progressMaterial->shader;
        delete progressMaterial;
        delete rectangle;
        for (auto& [name, data] : meshData) {
            delete data.first;
            delete data.second;
        }
        meshData.clear();
        delete multithreadedload.target<void(*)()>();
        if(callback) delete callback.target<void(*)()>();
    }
}