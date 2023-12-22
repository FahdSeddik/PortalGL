#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.inl>

#define PI 3.14159265358979323846f
namespace portal {

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json& config){
        // First, we store the window size for later use
        this->windowSize = windowSize;
        portals.resize(2);
        portalModelMats.resize(2);
        // Then we check if there is a sky texture in the configuration
        if(config.contains("sky")){
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));
            
            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram* skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();
            
            //TODO: (Req 10) Pick the correct pipeline state to draw the sky
            // Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            // We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            skyPipelineState.depthTesting.enabled = true;
            skyPipelineState.depthTesting.function = GL_LEQUAL;
            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.faceCulling.culledFace = GL_FRONT;
            
            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D* skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky 
            Sampler* skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material 
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        bloom = config.value("bloom", false);
        // Then we check if there is a "bloom" is set to true in configuration
        if(bloom){
            // Load the bloom parameters from the configuration
            bloomThreshold = config.value("bloomThreshold", 1.0f);
            bloomIntensity = config.value("bloomIntensity", 1.0f);
            bloomBlurIterations = config.value("bloomBlurIterations", 10);
            exposure = config.value("exposure", 1.0f);

            // Create a framebuffer to render the scene to
            glGenFramebuffers(1, &hdrFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            // The depth format can be (Depth component with 24 bits).
            colorTexture = texture_utils::empty(GL_RGBA16F, windowSize);
            brightColorTexture = texture_utils::empty(GL_RGBA16F, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH24_STENCIL8, windowSize);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightColorTexture->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            
            unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
            glDrawBuffers(2, attachments);
            //TODO: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler* sampler = new Sampler();
            sampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            sampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            sampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            sampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram* bloomShader = new ShaderProgram();
            bloomShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            bloomShader->attach("assets/shaders/postprocess/bloom.frag", GL_FRAGMENT_SHADER);
            bloomShader->link();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ping-pong-framebuffer for blurring
            glGenFramebuffers(2, pingpongFBO);
            // glGenTextures(2, pingpongColorbuffers);
            for (unsigned int i = 0; i < 2; i++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
                pingpongColorbuffers[i] = texture_utils::empty(GL_RGBA16F, windowSize);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i]->getOpenGLName(), 0);
                // also check if framebuffers are complete (no need for depth buffer)
                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                    std::cout << "Framebuffer not complete!" << std::endl;
            }
            pingpongMaterial = new MultiTextureMaterial();
            ShaderProgram* blurShader = new ShaderProgram();
            blurShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            blurShader->attach("assets/shaders/postprocess/bloomBlur.frag", GL_FRAGMENT_SHADER);
            blurShader->link();
            pingpongMaterial->shader = blurShader;
            pingpongMaterial->sampler = sampler;
            // setting up the material texture to be the color buffer
            pingpongMaterial->texture1 = pingpongColorbuffers[1];
            pingpongMaterial->texture2 = pingpongColorbuffers[0];
            pingpongMaterial->pipelineState.depthMask = false;

            // Create a post processing material
            hdrMaterial = new MultiTextureMaterial();
            hdrMaterial->shader = bloomShader;
            hdrMaterial->texture1 = colorTexture;
            hdrMaterial->texture2 = pingpongColorbuffers[0];
            hdrMaterial->sampler = sampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            hdrMaterial->pipelineState.depthMask = false;

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            if(config.contains("postprocess")){
                // Create the post processing shader
                ShaderProgram* postprocessShader = new ShaderProgram();
                postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
                postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
                postprocessShader->link();

                // Create a post processing material
                postprocessMaterial = new TexturedMaterial();
                postprocessMaterial->shader = postprocessShader;
                postprocessMaterial->texture = colorTexture;
                postprocessMaterial->sampler = sampler;
                // The default options are fine but we don't need to interact with the depth buffer
                // so it is more performant to disable the depth mask
                postprocessMaterial->pipelineState.depthMask = false;
            }
        } else if(config.contains("postprocess")){
            // Then we check if there is a postprocessing shader in the configuration
            //TODO: (Req 11) Create a framebuffer
            glGenFramebuffers(1, &postprocessFrameBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);
            //TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            // Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            // The depth format can be (Depth component with 24 bits).
            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);
            //TODO: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler* postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram* postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy(){
        // Delete all objects related to the sky
        if(skyMaterial){
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if(postprocessMaterial){
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
        // Delete all objects related to bloom
        if(bloom){
            glDeleteFramebuffers(1, &hdrFBO);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTexture;
            delete brightColorTexture;
            if(!postprocessMaterial) delete hdrMaterial->sampler;
            delete hdrMaterial->shader;
            delete hdrMaterial;
            glDeleteFramebuffers(2, pingpongFBO);
            delete pingpongColorbuffers[0];
            delete pingpongColorbuffers[1];
            delete pingpongMaterial->shader;
            delete pingpongMaterial;

        }
    }

    // Utility function to add a lights to the shader and set the "lightCount" uniform
    inline void setupLights(const std::vector<LightComponent*>& lights, ShaderProgram* shader){
        for(int i = 0; i < lights.size(); i++){
            if(lights[i]->type == LightComponent::Type::Directional){ // Directional
                shader->set("lights[" + std::to_string(i) + "].type", 0);
                shader->set("lights[" + std::to_string(i) + "].color", lights[i]->color);
                shader->set("lights[" + std::to_string(i) + "].direction", lights[i]->direction);
            } else if(lights[i]->type == LightComponent::Type::Point){ // Point
                shader->set("lights[" + std::to_string(i) + "].type", 1);
                shader->set("lights[" + std::to_string(i) + "].color", lights[i]->color);
                shader->set("lights[" + std::to_string(i) + "].position", lights[i]->worldSpacePosition);
                shader->set("lights[" + std::to_string(i) + "].attenuation", lights[i]->attenuation);
            } else if(lights[i]->type == LightComponent::Type::Spot){ // Spot
                shader->set("lights[" + std::to_string(i) + "].type", 2);
                shader->set("lights[" + std::to_string(i) + "].color", lights[i]->color);
                shader->set("lights[" + std::to_string(i) + "].position", lights[i]->worldSpacePosition);
                shader->set("lights[" + std::to_string(i) + "].direction",  lights[i]->direction);
                shader->set("lights[" + std::to_string(i) + "].innerCutOff", lights[i]->innerCutOff);
                shader->set("lights[" + std::to_string(i) + "].outerCutOff", lights[i]->outerCutOff);
                shader->set("lights[" + std::to_string(i) + "].attenuation", lights[i]->attenuation);
            }
        }
        shader->set("numLights", (int)lights.size());
    }

    void ForwardRenderer::drawNonPortalObjects(glm::mat4 const& modelMat, glm::mat4 const& viewMat, glm::mat4 const &projMat){
        //TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        // HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        //camera forward is the third row of the view matrix
        glm::vec3 eye = glm::vec3(modelMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glm::vec3 cameraForward = glm::vec3(viewMat[2][0], viewMat[2][1], viewMat[2][2]);
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand& first, const RenderCommand& second){
            //TODO: (Req 9) Finish this function
            // HINT: the following return should return true "first" should be drawn before "second".
            //calculating the magntiudes from the center of the object to the camera
            //the closer the object the smaller the magnitude
            float distanceToFirst = glm::dot(first.center, cameraForward);
            float distanceToSecond = glm::dot(second.center, cameraForward);
              // The command with the larger magnitude should be drawn first
            return distanceToFirst > distanceToSecond;
        });

        //TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP camera->getProjectionMatrix(windowSize)
        glm::mat4 VP = projMat * viewMat;
        //TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        // glViewport(0, 0, windowSize.x, windowSize.y);
        // //TODO: (Req 9) Set the clear color to black and the clear depth to 1
        // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // glClearDepth(1.0f);
        // //TODO: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // glDepthMask(GL_TRUE);

        
        

        //TODO: (Req 9) Clear the color and depth buffers
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //TODO: (Req 9) Draw all the opaque commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (auto &command : opaqueCommands){
            //Set the "transform" uniform to be equal the model-view-projection matrix for each render command
            command.material->setup();

            // check if the material is of type LitMaterial
            if (auto litMaterial = dynamic_cast<LitMaterial*>(command.material); litMaterial){
                // set the lights in the shader
                setupLights(lights, litMaterial->shader);
                // set the model, view, projection matrices in the shader
                litMaterial->shader->set("model", command.localToWorld);
                litMaterial->shader->set("VP", VP);
                litMaterial->shader->set("viewPos", eye);
            }
            else{
                glm::mat4 MVP = VP * command.localToWorld;
                command.material->shader->set("transform", MVP);
            }
            command.mesh->draw();
        }
        // If there is a sky material, draw the sky
        if(this->skyMaterial){
            //TODO: (Req 10) setup the sky material
            skyMaterial->setup();
            //TODO: (Req 10) Get the camera position
            // glm::vec3 cameraPosition = glm::vec3(modelMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            //TODO: (Req 10) Create a model matrix for the sy such that it always follows the camera (sky sphere center = camera position)
            glm::mat4 skyModelMatrix = glm::translate(glm::mat4(1.0f), eye);
            //TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            // We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f
            );
            //TODO: (Req 10) set the "transform" uniform
            skyMaterial->shader->set("transform", alwaysBehindTransform * VP * skyModelMatrix);
            //TODO: (Req 10) draw the sky sphere")
            skySphere->draw();
        }
        //TODO: (Req 9) Draw all the transparent commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (auto &command : transparentCommands){
            // Set the "transform" uniform to be equal the model-view-projection matrix for each render command
            command.material->setup();

            // check if the material is of type LitMaterial
            if (auto litMaterial = dynamic_cast<LitMaterial*>(command.material); litMaterial){
                // set the lights in the shader
                setupLights(lights, litMaterial->shader);
                // set the model, view, projection matrices in the shader
                litMaterial->shader->set("model", command.localToWorld);
                litMaterial->shader->set("VP", VP);
                litMaterial->shader->set("viewPos", eye);
            }
            else{
                glm::mat4 MVP = VP * command.localToWorld;
                command.material->shader->set("transform", MVP);
            }
            command.mesh->draw();
        }
    }

    void ForwardRenderer::drawPortal(glm::mat4 const& modelMat, glm::mat4 const &viewMat, glm::mat4 const &projMat, Entity* curportal) {
        RenderCommand command;
        MeshRendererComponent* meshRenderer = curportal->getComponent<MeshRendererComponent>();
        command.localToWorld = modelMat;
        command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
        command.mesh = meshRenderer->mesh;
        command.material = meshRenderer->material;
        glm::mat4 MVP = projMat * viewMat * command.localToWorld;
        command.material->setup();
        command.material->shader->set("transform", MVP);
        command.mesh->draw();
    }

    glm::mat4 const ForwardRenderer::getClippedProjMat(const r3d::Quaternion& quat, const r3d::Vector3& pos, glm::mat4 const& viewMat, glm::mat4 const& projMat) {
        glm::vec3 d_position(pos.x, pos.y, pos.z);
        glm::fquat d_orientation(quat.w, quat.x, quat.y, quat.z);
        // Assuming the plane normal is the direction the object is facing after rotation
        glm::vec3 normal = d_orientation * glm::vec3(0, 0, 1) ; // Assuming z-axis is the facing direction
        // Normalize the normal vector
        normal = glm::normalize(normal);
        // Calculate the distance from the origin along the normal
        float distance = -glm::dot(normal, d_position);
        glm::vec4 clipPlane(normal, distance);
        clipPlane = glm::inverse(glm::transpose(viewMat)) * clipPlane;

        glm::vec4 q;
        q.x = (glm::sign(clipPlane.x) + projMat[2][0]) / projMat[0][0];
        q.y = (glm::sign(clipPlane.y) + projMat[2][1]) / projMat[1][1];
        q.z = -1.0f;
        q.w = (1.0f + projMat[2][2]) / projMat[3][2];

        glm::vec4 c = clipPlane * (2.0f / (glm::dot(clipPlane, q)));

        glm::mat4 newProj = projMat;
        newProj[0][2] = c.x;
        newProj[1][2] = c.y;
        newProj[2][2] = c.z + 1.0f;
        newProj[3][2] = c.w;

        return newProj;
    }

    void ForwardRenderer::drawRecursivePortals(glm::mat4 const& modelMat, glm::mat4 const &viewMat, glm::mat4 const &projMat, size_t maxRecursionLevel, size_t recursionLevel) {
        for(int i = 0; i < 2; i++) {
            Entity* curportal = portals[i];
            // Disable color and depth drawing
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);

            // Disable depth test
            glDisable(GL_DEPTH_TEST);

            // Enable stencil test, to prevent drawing outside
            // region of current portal depth
            glEnable(GL_STENCIL_TEST);

            // Fail stencil test when inside of outer portal
            // (fail where we should be drawing the inner portal)
            glStencilFunc(GL_NOTEQUAL, (GLint)recursionLevel, (GLuint)0xFF);

            // Increment stencil value on stencil fail
            // (on area of inner portal)
            glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);

            // Enable (writing into) all stencil bits
            glStencilMask(0xFF);

            drawPortal(portalModelMats[i], viewMat, projMat, curportal);
            const r3d::Quaternion& temprot = curportal->localTransform.getRotation();
            glm::fquat rot(temprot.w, temprot.x, temprot.y, temprot.z);
            glm::mat4 destView = viewMat * portalModelMats[i]
            * glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0.0f, 1.0f, 0.0f) * rot)
            * glm::inverse(portalModelMats[1-i]);
            // Base case, render inside of inner portal
            if (recursionLevel == maxRecursionLevel)
            {
                // Enable color and depth drawing
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glDepthMask(GL_TRUE);
                
                // Clear the depth buffer so we don't interfere with stuff
                // outside of this inner portal
                glClear(GL_DEPTH_BUFFER_BIT);

                // Enable the depth test
                // So the stuff we render here is rendered correctly
                glEnable(GL_DEPTH_TEST);

                // Enable stencil test
                // So we can limit drawing inside of the inner portal
                glEnable(GL_STENCIL_TEST);

                // Disable drawing into stencil buffer
                glStencilMask(0x00);

                // Draw only where stencil value == recursionLevel + 1
                // which is where we just drew the new portal
                glStencilFunc(GL_EQUAL, (GLint)recursionLevel + 1, 0xFF);

                // Draw scene objects with destView, limited to stencil buffer
                // use an edited projection matrix to set the near plane to the portal plane
                drawNonPortalObjects(portalModelMats[1-i], destView, getClippedProjMat(portals[1-i]->localTransform.getRotation(), portals[1-i]->localTransform.getPosition(), destView, projMat));
                //drawNonPortals(destView, projMat);
            }
            else
            {
                // Recursion case
                // Pass our new view matrix and the clipped projection matrix (see above)
                drawRecursivePortals(portalModelMats[1-i], destView, getClippedProjMat(portals[1-i]->localTransform.getRotation(), portals[1-i]->localTransform.getPosition(), destView, projMat), maxRecursionLevel, recursionLevel + 1);
            }
            // Disable color and depth drawing
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);

            // Enable stencil test and stencil drawing
            glEnable(GL_STENCIL_TEST);
            glStencilMask(static_cast<GLuint>(0xFF));

            // Fail stencil test when inside of our newly rendered
            // inner portal
            glStencilFunc(GL_NOTEQUAL, (GLint)recursionLevel + 1, static_cast<GLuint>(0xFF));

            // Decrement stencil value on stencil fail
            // This resets the incremented values to what they were before,
            // eventually ending up with a stencil buffer full of zero's again
            // after the last (outer) step.
            glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

            // Draw portal into stencil buffer
            drawPortal(portalModelMats[i], viewMat, projMat, curportal);
        }
        // Disable the stencil test and stencil writing
        glDisable(GL_STENCIL_TEST);
        glStencilMask(0x00);

        // Disable color writing
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // Enable the depth test, and depth writing.
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        // Make sure we always write the data into the buffer
        glDepthFunc(GL_ALWAYS);

        // Clear the depth buffer
        glClear(GL_DEPTH_BUFFER_BIT);

        // Draw portals into depth buffer
        for(int i = 0; i < 2; i++) {
            drawPortal(portalModelMats[i], viewMat, projMat, portals[i]);
        }

        // Reset the depth function to the default
        glDepthFunc(GL_LESS);

        // Enable stencil test and disable writing to stencil buffer
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);

        // Draw at stencil >= recursionlevel
        // which is at the current level or higher (more to the inside)
        // This basically prevents drawing on the outside of this level.
        glStencilFunc(GL_LEQUAL, (GLint)recursionLevel, 0xFF);

        // Enable color and depth drawing again
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // And enable the depth test
        glEnable(GL_DEPTH_TEST);

        // Draw scene objects normally, only at recursionLevel
        drawNonPortalObjects(modelMat, viewMat, projMat);
    }
    
    void ForwardRenderer::render(World* World){
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent* camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        lights.clear();
        Entity* portal1 = world->getEntityByName("Portal_1");
        Entity* portal2 = world->getEntityByName("Portal_2");
        for(const auto& [name, entity] : world->getEntities()){
            if(entity == portal1 || entity == portal2) 
                continue;
            // If we hadn't found a camera yet, we look for a camera in this entity
            if(!camera) camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if(auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer){
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if(command.material->transparent){
                    transparentCommands.push_back(command);
                } else {
                // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }

            // Add all the lights in the scene to the lights vector
            if(auto light = entity->getComponent<LightComponent>(); light){
                lights.push_back(light);
            }
        }

        if(bloom){
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        }
        // If there is a postprocess material, bind the framebuffer
        else if(postprocessMaterial){
            //TODO: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);
        }

        // CLear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, windowSize.x, windowSize.y);
        // If there is no camera, we return (we cannot render without a camera)
        if(camera == nullptr) return;
        if(!portal1 || !portal2) {
            drawNonPortalObjects(camera->getOwner()->getLocalToWorldMatrix(), camera->getViewMatrix(), camera->getProjectionMatrix(windowSize));
            // return;
        } else{
            portals[0] = portal1, portals[1] = portal2;
            portalModelMats[0] = portal1->getLocalToWorldMatrix(), portalModelMats[1] = portal2->getLocalToWorldMatrix();
            size_t maxRecursionLevel = 1;
            // drawRecursivePortals(camera->getOwner()->getLocalToWorldMatrix(),camera->getViewMatrix(), camera->getProjectionMatrix(windowSize), maxRecursionLevel);
            drawPortalsNonRecursive(camera->getOwner()->getLocalToWorldMatrix(),camera->getViewMatrix(), camera->getProjectionMatrix(windowSize), portal1, portal2);
        }

        if(bloom){ 
            // apply the pingpong blur
            bool horizontal = true, first_iteration = true;
            for (int i = 0; i < bloomBlurIterations; i++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                pingpongMaterial->setup();
                pingpongMaterial->shader->set("horizontal", horizontal);
                if (first_iteration){
                    glActiveTexture(GL_TEXTURE1);
                    brightColorTexture->bind();
                    if(pingpongMaterial->sampler)
                        pingpongMaterial->sampler->bind(1);
                    pingpongMaterial->shader->set("tex2", 1);
                } 
                glBindVertexArray(postProcessVertexArray);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                horizontal = !horizontal;
                if (first_iteration)
                    first_iteration = false;
            }

           if(postprocessMaterial){
                // if there is a postprocess material, draw the scene to the framebuffer after applying bloom
                // and then draw the framebuffer to the screen using the postprocess material
                glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
                // glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindVertexArray(postProcessVertexArray);
                hdrMaterial->setup();
                hdrMaterial->shader->set("bloomIntensity", bloomIntensity);
                hdrMaterial->shader->set("exposure", exposure);       
                glDrawArrays(GL_TRIANGLES, 0, 3);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindVertexArray(postProcessVertexArray);
                postprocessMaterial->setup();
                glDrawArrays(GL_TRIANGLES, 0, 3);

            } else {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindVertexArray(postProcessVertexArray);
                hdrMaterial->setup();
                hdrMaterial->shader->set("bloomIntensity", bloomIntensity);
                hdrMaterial->shader->set("exposure", exposure);       
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }

        }
        // If there is a postprocess material, draw the scene to the framebuffer
        else  if(postprocessMaterial){
            //TODO: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            //TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            glBindVertexArray(postProcessVertexArray);
            postprocessMaterial->setup();
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

    }

    void ForwardRenderer::drawPortalsNonRecursive(glm::mat4 const& modelMat, glm::mat4 const &viewMat, 
                                    glm::mat4 const &projMat, Entity* portal1, Entity* portal2) {

        /*
        * 1-Draw the normal scene without portals.
        * 2-Draw the portals, writing a different value to the stencil buffer for each portal.
        * 3-For each portal:
        *    a. Position the camera facing out of the other portal.
        *    b. Render the scene, with the stencil buffer condition set to only allow fragments corresponding to this portal.
        */

        // 1-Draw the normal scene without portals.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // Enable the depth test
        glEnable(GL_DEPTH_TEST);
        drawNonPortalObjects(modelMat, viewMat, projMat);

        // 2-Draw the portals, writing a different value to the stencil buffer for each portal.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        // Disable depth test
        glDisable(GL_DEPTH_TEST);


        glEnable(GL_STENCIL_TEST);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        drawPortal(portalModelMats[0], viewMat, projMat, portal1);

        glStencilFunc(GL_ALWAYS, 2, 0xFF);
        drawPortal(portalModelMats[1], viewMat, projMat, portal2);

        // 3-For each portal:
        //     a. Position the camera facing out of the other portal.
        //     b. Render the scene, with the stencil buffer condition set to only allow fragments corresponding to this portal.

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);

        // Draw scene objects with destView, limited to stencil buffer
        // use an edited projection matrix to set the near plane to the portal plane
       
        // get dest view
        const r3d::Quaternion& temprot = portal1->localTransform.getRotation();
        glm::fquat rot(temprot.w, temprot.x, temprot.y, temprot.z);
        const r3d::Quaternion& temprot2 = portal2->localTransform.getRotation();
        glm::fquat rot2(temprot2.w, temprot2.x, temprot2.y, temprot2.z);
        glm::vec3 norm1 = rot * glm::vec3(0, 0, 1);
        glm::vec3 norm2 = rot2 * glm::vec3(0, 0, 1);
        glm::vec3 adjuster;

        // check if cos the angle between the normal and the y axis is small (the angle is close to 90 degrees), 
        // then the portal is vertical ex. on the wall. So, rotate along the y axis 
        if(std::abs(glm::dot(norm1, glm::vec3(0, 1, 0))) < 0.01f) {
            adjuster = glm::vec3(0, 1, 0);
        } else { // otherwise if on the ceiling rotate along the z axis 
            adjuster = glm::vec3(0, 0, 1);
        }
        glm::mat4 destView = viewMat * portalModelMats[0]
        * glm::rotate(glm::mat4(1.0f), PI, adjuster * rot)
        * glm::inverse(portalModelMats[1]);
        
        // check if cos the angle between the normal and the y axis is small (the angle is close to 90 degrees), 
        // then the portal is vertical ex. on the wall. So, rotate along the y axis
        if (std::abs(glm::dot(norm2, glm::vec3(0, 1, 0))) < 0.01f) {
            adjuster = glm::vec3(0, 1, 0);
        } else { // otherwise if on the ceiling rotate along the z axis 
            adjuster = glm::vec3(0, 0, 1);
        }
        glm::mat4 destView2 = viewMat * portalModelMats[1]
        * glm::rotate(glm::mat4(1.0f), PI, adjuster * rot2)
        * glm::inverse(portalModelMats[0]);


        // Draw scene objects with destView, limited to stencil buffer
        // use an edited projection matrix to set the near plane to the portal plane
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        drawNonPortalObjects(portalModelMats[1], destView, getClippedProjMat(portal2->localTransform.getRotation(), portal2->localTransform.getPosition(), destView, projMat));

        glStencilFunc(GL_EQUAL, 2, 0xFF);
        drawNonPortalObjects(portalModelMats[0], destView2, getClippedProjMat(portal1->localTransform.getRotation(), portal1->localTransform.getPosition(), destView2, projMat));

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glDisable(GL_STENCIL_TEST);
    }

}