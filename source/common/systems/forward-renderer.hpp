#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/mesh-renderer.hpp"
#include "../components/lighting.hpp"
#include "../asset-loader.hpp"

#include <glad/gl.h>
#include <vector>
#include <algorithm>

namespace portal
{
    
    // The render command stores command that tells the renderer that it should draw
    // the given mesh at the given localToWorld matrix using the given material
    // The renderer will fill this struct using the mesh renderer components
    struct RenderCommand {
        glm::mat4 localToWorld;
        glm::vec3 center;
        Mesh* mesh;
        Material* material;
    };

    // A forward renderer is a renderer that draw the object final color directly to the framebuffer
    // In other words, the fragment shader in the material should output the color that we should see on the screen
    // This is different from more complex renderers that could draw intermediate data to a framebuffer before computing the final color
    // In this project, we only need to implement a forward renderer
    class ForwardRenderer {
        // These window size will be used on multiple occasions (setting the viewport, computing the aspect ratio, etc.)
        glm::ivec2 windowSize;
        // These are two vectors in which we will store the opaque and the transparent commands.
        // We define them here (instead of being local to the "render" function) as an optimization to prevent reallocating them every frame
        std::vector<RenderCommand> opaqueCommands;
        std::vector<RenderCommand> transparentCommands;
        // Objects used for rendering a skybox
        Mesh* skySphere;
        TexturedMaterial* skyMaterial;
        // Objects used for Postprocessing
        GLuint postProcessVertexArray;
        // Texture2D *colorTarget, *depthTarget;
        TexturedMaterial* postprocessMaterial;
        // List of all the lights in the scene
        std::vector<LightComponent*> lights;
        bool firstFrame = true;

        // **********************//
        // **** Bloom & HDR **//
        // **********************//
        bool bloom;
        float bloomThreshold;
        float bloomIntensity;
        int bloomBlurIterations;
        float exposure;
        // Framebuffer used to store the bright color
        GLuint hdrFBO;
        // Texture used to store the normal color
        Texture2D *colorTexture, *depthTexture;
        // Texture used to store the bright color
        Texture2D *brightColorTexture;
        // Material used to render final frame
        MultiTextureMaterial* hdrMaterial;
        // pingpong variable to switch between the two postprocess materials
        // pingpong framebuffers
        GLuint pingpongFBO[2];
        // pingpong colorbuffers
        Texture2D *pingpongColorbuffers[2];
        // pingpong material
        // Material used to blur the bright color
        MultiTextureMaterial* pingpongMaterial;
        // TexturedMaterial* pingpongMaterial[2];

        // **********************//
        // **** Portal **//
        // **********************//
        std::vector<Entity *> portals;
        std::vector<glm::mat4> portalModelMats;
        void drawNonPortalObjects(glm::mat4 const& cameraModelMat,glm::mat4 const& viewMat, glm::mat4 const &projMat);
        void drawRecursivePortals(glm::mat4 const& modelMat, glm::mat4 const &viewMat, glm::mat4 const &projMat, size_t maxRecursionLevel, size_t recursionLevel = 0);
        void drawPortal(glm::mat4 const& modelMat, glm::mat4 const &viewMat, glm::mat4 const &projMat, Entity* curportal);
        glm::mat4 const getClippedProjMat(const r3d::Quaternion& quat, const r3d::Vector3& pos, glm::mat4 const& viewMat, glm::mat4 const& projMat);
        void ForwardRenderer::drawPortalsNonRecursive(glm::mat4 const& modelMat, glm::mat4 const &viewMat, 
                                glm::mat4 const &projMat, Entity* portal1, Entity* portal2);
        void setupLights(const std::vector<LightComponent*>& lights, ShaderProgram* shader);
    public:
        // Initialize the renderer including the sky and the Postprocessing objects.
        // windowSize is the width & height of the window (in pixels).
        void initialize(glm::ivec2 windowSize, const nlohmann::json& config);
        // Clean up the renderer
        void destroy();
        // This function should be called every frame to draw the given world
        void render(World* world);

        void setBloom(bool bloom);

    };

}