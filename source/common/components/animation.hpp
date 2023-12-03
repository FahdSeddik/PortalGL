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
        std::string name = "";
        std::function<void()> callback;
    public:
        // The ID of this component type is "Animation"
        static std::string getID() { return "Animation"; }

        // Reads animation data from the given json object
        void deserialize(const nlohmann::json& data) override;

        // Plays animation given delta time
        bool play(float deltaTime);

        // Starts playing the animation
        void startPlaying() { isPlaying = true; }

        // Resets the animation
        void reset();

        std::string getName() { return name; }
    };
}