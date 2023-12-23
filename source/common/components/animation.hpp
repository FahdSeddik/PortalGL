#pragma once
#include "../ecs/component.hpp"
#include "../ecs/transform.hpp"

namespace portal {

    // WARNING: IT IS PROHIBITED TO USE THIS WITH AN OBJECT THAT HAS A "RigidBody" COMPONENT
    class AnimationComponent : public Component {
        Transform start;
        Transform end;
        float duration = 1.0f;
        float accumulatedTime = 0.0f;
        bool isPlaying = false;
        bool isStarted = false;
        bool isReversed = false;
        std::string name = "";
        // Callback lambda function to be called when animation ends
        std::function<void()> callback;
        std::function<void()> reverseCallback;

    public:
        // The ID of this component type is "Animation"
        static std::string getID() { return "Animation"; }

        // Reads animation data from the given json object
        void deserialize(const nlohmann::json& data) override;
        void deserializeCallback(const nlohmann::json& data, bool reverse = false);

        // Plays animation given delta time
        bool play(float deltaTime);

        // Starts playing the animation
        void startPlaying(bool reverse) { reset(), isPlaying = true, isReversed = reverse; }

        // Resets the animation
        void reset();

        void envokeCallback() {
            if (isReversed) {
                if(reverseCallback) reverseCallback();
            } else {
                if(callback) callback();
            }
            reset();
        }

        bool getIsReversed() const { return isReversed; }

        std::string getName() { return name; }
    };
}