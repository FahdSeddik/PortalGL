#pragma once

#include <glad/gl.h>
#include <glm/vec4.hpp>
#include <json/json.hpp>

namespace our {
    // There are some options in the render pipeline that we cannot control via shaders
    // such as blending, depth testing and so on
    // Since each material could require different options (e.g. transparent materials usually use blending),
    // we will encapsulate all these options into a single structure that will also be responsible for configuring OpenGL's pipeline
    struct PipelineState {
        // This set of pipeline options specifies whether face culling will be used or not and how it will be configured
        struct {
            bool enabled = false;
            GLenum culledFace = GL_BACK;
            GLenum frontFace = GL_CCW;
        } faceCulling;

        // This set of pipeline options specifies whether depth testing will be used or not and how it will be configured
        struct {
            bool enabled = false;
            GLenum function = GL_LEQUAL;
        } depthTesting;

        // This set of pipeline options specifies whether blending will be used or not and how it will be configured
        struct {
            bool enabled = false;
            GLenum equation = GL_FUNC_ADD;
            GLenum sourceFactor = GL_SRC_ALPHA;
            GLenum destinationFactor = GL_ONE_MINUS_SRC_ALPHA;
            glm::vec4 constantColor = {0, 0, 0, 0};
        } blending;

        // These options specify the color and depth mask which can be used to
        // prevent the rendering/clearing from modifying certain channels of certain targets in the framebuffer
        glm::bvec4 colorMask = {true, true, true, true}; // To know how to use it, check glColorMask
        bool depthMask = true; // To know how to use it, check glDepthMask


        // This function should set the OpenGL options to the values specified by this structure
        // For example, if faceCulling.enabled is true, you should call glEnable(GL_CULL_FACE), otherwise, you should call glDisable(GL_CULL_FACE)
        void setup() const {
            //TODO: (Req 4) Write this function

            // Checking for faceculling
            if (faceCulling.enabled) {
                // Enabling faceculling
                glEnable(GL_CULL_FACE);
                // Setting which faces to cull could be GL_FRONT/GL_BACK/GL_FRONT_AND_BACK
                glCullFace(faceCulling.culledFace);
                // Setting the front face could be GL_CW/GL_CCW
                glFrontFace(faceCulling.frontFace);
            }
            else {
                glDisable(GL_CULL_FACE);
            }
            
            // Checking for depth testing
            if(depthTesting.enabled) {
                // Enabling depth testing
                glEnable(GL_DEPTH_TEST);
                // Depth functions could be:
                // GL_NEVER: The depth function never passes, so no drawing is done.
                // GL_LESS: The depth function passes if the incoming depth value is less than the stored depth value.
                // GL_EQUAL: The depth function passes if the incoming depth value is equal to the stored depth value.
                // GL_LEQUAL: The depth function passes if the incoming depth value is less than or equal to the stored depth value.
                // GL_GREATER: The depth function passes if the incoming depth value is greater than the stored depth value.
                // GL_NOTEQUAL: The depth function passes if the incoming depth value is not equal to the stored depth value.
                // GL_GEQUAL: The depth function passes if the incoming depth value is greater than or equal to the stored depth value.
                // GL_ALWAYS: The depth function always passes, so drawing is always done
                glDepthFunc(depthTesting.function);
            }
            else {
                glDisable(GL_DEPTH_TEST);
            }

            if(blending.enabled) {
                glEnable(GL_BLEND);
                // Could be:
                // GL_FUNC_ADD: The sum of the source and destination colors.
                // GL_FUNC_SUBTRACT: The difference of the source and destination colors.
                // GL_FUNC_REVERSE_SUBTRACT: The difference of the destination and source colors.
                // GL_MIN: The minimum color components of the source and destination colors.
                // GL_MAX: The maximum color components of the source and destination colors.
                glBlendEquation(blending.equation);
                // This function specifies how the blending factors are computed.
                // Source factor: is the factor by which the source color components are multiplied.
                // Destination factor: is the factor by which the destination color components are multiplied.
                // Some possible values are: GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, etc.
                glBlendFunc(blending.sourceFactor, blending.destinationFactor);
                // To set the constant color used in blending operations
                glBlendColor(blending.constantColor.r, blending.constantColor.g, blending.constantColor.b, blending.constantColor.a);
            }
            else {
                glDisable(GL_BLEND);
            }

            // The color mask is a 4-component boolean vector that controls which components of the color buffer are written.
            // The initial value is all TRUE, indicating that the color buffer is enabled for writing.
            // To enable or disable writing of individual color components, call glColorMask with the desired boolean values.
            // enabling red for example means that when we draw, only the red channel will be drawn
            glColorMask(colorMask.r, colorMask.g, colorMask.b, colorMask.a);
            // a depth buffer ensures that the closest object is drawn in front of the farthest object
            glDepthMask(depthMask);
        }

        // Given a json object, this function deserializes a PipelineState structure
        void deserialize(const nlohmann::json& data);
    };

}