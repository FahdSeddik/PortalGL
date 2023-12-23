#include "pause-menu.hpp"
#include "application.hpp"
#include "asset-loader.hpp"
#include "mesh/mesh-utils.hpp"
#include "mesh/mesh.hpp"
#include "material/material.hpp"
#include "texture/texture2d.hpp"
#include "texture/sampler.hpp"
#include "texture/texture-utils.hpp"
#include "shader/shader.hpp"
#include "ecs/world.hpp"
#include "systems/forward-renderer.hpp"
// #include "states/menu-state.hpp"
#include "../states/play-state.hpp"

namespace portal {
    void PauseMenu::init(Application* app, ForwardRenderer* renderer, Playstate* playstate, World* world) {
        PauseMenu::app = app;
        PauseMenu::playstate = playstate;

        pause = true;
        options = false;      
        
        // Load the pause menu material
        pauseMaterial = new TexturedMaterial();
        pauseMaterial->shader = new ShaderProgram();
        pauseMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        pauseMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        pauseMaterial->shader->link();
        pauseMaterial->texture = texture_utils::loadImage("assets/textures/pause_menu.png");
        pauseMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        pauseMaterial->pipelineState.blending.enabled = true;

        // Load the options menu material
        optionsMaterial = new TexturedMaterial();
        optionsMaterial->shader = new ShaderProgram();
        optionsMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        optionsMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        optionsMaterial->shader->link();
        optionsMaterial->texture = texture_utils::loadImage("assets/textures/options_menu.png");
        optionsMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        optionsMaterial->pipelineState.blending.enabled = true;

        // create a material to highlight the hovered buttons
        highlightMaterial = new portal::TintedMaterial();
        // Since the highlight is not textured, we used the tinted material shaders
        highlightMaterial->shader = new portal::ShaderProgram();
        highlightMaterial->shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER);
        highlightMaterial->shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER);
        highlightMaterial->shader->link();
        // The tint is white since we will subtract the background color from it to create a negative effect.
        highlightMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        // To create a negative effect, we enable blending, set the equation to be subtract,
        // and set the factors to be one for both the source and the destination. 
        highlightMaterial->pipelineState.blending.enabled = true;
        highlightMaterial->pipelineState.blending.equation = GL_FUNC_SUBTRACT;
        highlightMaterial->pipelineState.blending.sourceFactor = GL_ONE;
        highlightMaterial->pipelineState.blending.destinationFactor = GL_ONE;

        // Load the pause menu mesh
        rectangle = new portal::Mesh({
            {{0.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{0.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        },{
            0, 1, 2, 2, 3, 0,
        });
        // Set the size of the screen
        size = app->getFrameBufferSize();
        // Set the projection matrix to make the origin at the top-left corner of the screen
        VP = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        menuModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        optionsModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(size.x/3, size.y/2, 1.0f));
        // translate the options menu to the center of the screen
        optionsModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(size.x/3.0f, size.y/4.0f, 0.0f)) * optionsModelMatrix;

        // draw the scene on both buffers so that if there was a quick movement
        // the screen won't flicker with each buffer swap 
        renderer->render(world);
        glfwSwapBuffers(app->getWindow());
        renderer->render(world);

        pauseMaterial->setup();
        pauseMaterial->shader->set("transform", VP * menuModelMatrix);
        rectangle->draw();

        // Set the pauseButtons
        // Resume button
        pauseButtons.push_back(Button{
            glm::vec2(110.0f, 364.0f),
            glm::vec2(173.0f, 25.0f),
            [](){
                portal::PauseMenu::unPause();
            }
        });
        pauseButtons.push_back(Button{
            glm::vec2(110.0f, 396.0f),
            glm::vec2(173.0f, 25.0f),
            [](){
                portal::PauseMenu::openOptions();
            }
        });
        pauseButtons.push_back(Button{
            glm::vec2(110.0f, 428.0f),
            glm::vec2(173.0f, 26.0f),
            [](){
                portal::PauseMenu::quit();
            }
        });

        // // Set the options buttons
        // optionsButtons = {
        //     Button{
        //         glm::vec2(0.0f, 0.0f),
        //         glm::vec2(0.5f, 0.5f),
        //         [app](){
        //             app->changeState("menu");
        //         }
        //     },
        //     Button{
        //         glm::vec2(0.5f, 0.0f),
        //         glm::vec2(0.5f, 0.5f),
        //         [app](){
        //             app->changeState("options");
        //         }
        //     },
        // };
    }

    bool PauseMenu::render() {
        // Set the viewport to the size of the screen
        // Render the pause menu
        renderPauseMenu();
        // Render the options menu
        if(options){
            renderOptionsMenu();
        }
        return pause;
    }

    void PauseMenu::renderPauseMenu() {
        // Draw the pause menu
        pauseMaterial->setup();
        pauseMaterial->shader->set("transform", VP * menuModelMatrix);
        rectangle->draw();
        mousePosition = app->getMouse().getMousePosition();

        if(app->getMouse().justPressed(GLFW_MOUSE_BUTTON_LEFT)){
            // Check if any of the buttons are pressed
            for(auto& button: pauseButtons){
                if(button.isInside(mousePosition)){
                    button.action();
                }
            }
            std::cout << "mouse position: " << mousePosition.x << ", " << mousePosition.y << std::endl;
        }
        // Draw the buttons
        for(auto& button: pauseButtons){
            if(button.isInside(mousePosition)){
                highlightMaterial->setup();
                highlightMaterial->shader->set("transform", VP*button.getLocalToWorld());
                rectangle->draw();
            }
        }
    }

    void PauseMenu::renderOptionsMenu() {
        // Draw the options menu
        optionsMaterial->setup();
        optionsMaterial->shader->set("transform", VP * optionsModelMatrix);
        rectangle->draw();
        // Draw the buttons
        for(auto& button: optionsButtons){
            if(button.isInside(mousePosition)){
                highlightMaterial->setup();
                highlightMaterial->shader->set("transform", VP*button.getLocalToWorld());
                rectangle->draw();
            }
        }
    }

    void PauseMenu::cleanUp() {
        options = false;
        pause = false;
        // Delete all the allocated resources
        delete rectangle;
        delete pauseMaterial->texture;
        delete pauseMaterial->shader;
        delete pauseMaterial;
        delete optionsMaterial->shader;
        delete optionsMaterial;
        pauseButtons.clear();
        app->getMouse().unlockMouse(app->getWindow());
    }

    void PauseMenu::pauseGame() {
        pause = true;
        options = false;
        app->getMouse().unlockMouse(app->getWindow());
    }

    void PauseMenu::unPause() {
        pause = false; 
        app->getMouse().lockMouse(app->getWindow());
        // cleanUp();
    }

    void PauseMenu::quit() {
        pause = false;
        // cleanUp();
        app->changeState("menu");
    }

    void PauseMenu::openOptions() {
        options = true;
    }
}
