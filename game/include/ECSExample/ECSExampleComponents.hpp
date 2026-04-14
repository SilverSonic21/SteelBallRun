#pragma once

#include <Canis/Entity.hpp>

#include <string>

namespace ECSExample
{
    struct SamplePlayer
    {
    public:
        static constexpr const char* ScriptName = "ECSExample::SamplePlayer";

        SamplePlayer() = default;
        explicit SamplePlayer(Canis::Entity& _entity) : entity(&_entity) {}
        Canis::Entity* entity = nullptr;

        void Create() {}

        float moveForce = 2400.0f;
        float jumpImpulse = 6.5f;
        float groundCheckDistance = 0.8f;
        Canis::Mask groundCollisionMask = Canis::Rigidbody::DefaultLayer;
        bool logInstructions = true;

        int totalPickups = 0;
        int collectedPickups = 0;
        bool grounded = false;
        bool initialized = false;
    };

    struct Spin
    {
    public:
        static constexpr const char* ScriptName = "ECSExample::Spin";

        Spin() = default;
        explicit Spin(Canis::Entity& _entity) : entity(&_entity) {}
        Canis::Entity* entity = nullptr;

        void Create() {}

        float speedDegrees = 120.0f;
    };

    struct Hover
    {
    public:
        static constexpr const char* ScriptName = "ECSExample::Hover";

        Hover() = default;
        explicit Hover(Canis::Entity& _entity) : entity(&_entity) {}
        Canis::Entity* entity = nullptr;

        void Create() {}

        float amplitude = 0.18f;
        float speed = 2.2f;

        float baseHeight = 0.0f;
        float elapsed = 0.0f;
        bool initialized = false;
    };

    struct Collectible
    {
    public:
        static constexpr const char* ScriptName = "ECSExample::Collectible";

        Collectible() = default;
        explicit Collectible(Canis::Entity& _entity) : entity(&_entity) {}
        Canis::Entity* entity = nullptr;

        void Create() {}

        std::string collectorTag = "ExamplePlayer";
    };

    struct SampleHUD
    {
    public:
        static constexpr const char* ScriptName = "ECSExample::SampleHUD";

        SampleHUD() = default;
        explicit SampleHUD(Canis::Entity& _entity) : entity(&_entity) {}
        Canis::Entity* entity = nullptr;

        void Create() {}
        
        Canis::Entity* player = nullptr;
    };

    void RegisterSamplePlayerComponent(Canis::App& _app);
    void UnRegisterSamplePlayerComponent(Canis::App& _app);

    void RegisterSpinComponent(Canis::App& _app);
    void UnRegisterSpinComponent(Canis::App& _app);

    void RegisterHoverComponent(Canis::App& _app);
    void UnRegisterHoverComponent(Canis::App& _app);

    void RegisterCollectibleComponent(Canis::App& _app);
    void UnRegisterCollectibleComponent(Canis::App& _app);

    void RegisterSampleHUDComponent(Canis::App& _app);
    void UnRegisterSampleHUDComponent(Canis::App& _app);
}
