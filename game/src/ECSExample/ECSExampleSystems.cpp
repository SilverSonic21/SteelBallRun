#include <ECSExample/ECSExampleSystems.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Scene.hpp>

#include <ECSExample/ECSExampleComponents.hpp>

#include <cmath>
#include <string>

namespace ECSExample
{
    namespace
    {
        int CountPickups(Canis::Scene& _scene)
        {
            return static_cast<int>(_scene.GetEntitiesWithTag("ExamplePickup").size());
        }
    }

    Canis::SystemConf samplePlayerSystemConf = {};
    Canis::SystemConf spinSystemConf = {};
    Canis::SystemConf hoverSystemConf = {};
    Canis::SystemConf collectibleSystemConf = {};
    Canis::SystemConf sampleHUDSystemConf = {};

    void RegisterSamplePlayerSystem(Canis::App& _app)
    {
        DEFAULT_SYSTEM_CONFIG(samplePlayerSystemConf, ECSExample::SamplePlayerSystem, Canis::SystemPipeline::Update);
        _app.RegisterSystem(samplePlayerSystemConf);
    }

    DEFAULT_UNREGISTER_SYSTEM(samplePlayerSystemConf, SamplePlayer)

    void RegisterSpinSystem(Canis::App& _app)
    {
        DEFAULT_SYSTEM_CONFIG(spinSystemConf, ECSExample::SpinSystem, Canis::SystemPipeline::Update);
        _app.RegisterSystem(spinSystemConf);
    }

    DEFAULT_UNREGISTER_SYSTEM(spinSystemConf, Spin)

    void RegisterHoverSystem(Canis::App& _app)
    {
        DEFAULT_SYSTEM_CONFIG(hoverSystemConf, ECSExample::HoverSystem, Canis::SystemPipeline::Update);
        _app.RegisterSystem(hoverSystemConf);
    }

    DEFAULT_UNREGISTER_SYSTEM(hoverSystemConf, Hover)

    void RegisterCollectibleSystem(Canis::App& _app)
    {
        DEFAULT_SYSTEM_CONFIG(collectibleSystemConf, ECSExample::CollectibleSystem, Canis::SystemPipeline::Update);
        _app.RegisterSystem(collectibleSystemConf);
    }

    DEFAULT_UNREGISTER_SYSTEM(collectibleSystemConf, Collectible)

    void RegisterSampleHUDSystem(Canis::App& _app)
    {
        DEFAULT_SYSTEM_CONFIG(sampleHUDSystemConf, ECSExample::SampleHUDSystem, Canis::SystemPipeline::Update);
        _app.RegisterSystem(sampleHUDSystemConf);
    }

    DEFAULT_UNREGISTER_SYSTEM(sampleHUDSystemConf, SampleHUD)

    void SamplePlayerSystem::Create()
    {
        m_name = "ECSExample::SamplePlayerSystem";
    }

    void SamplePlayerSystem::Update(entt::registry& _registry, float _deltaTime)
    {
        auto view = _registry.view<ECSExample::SamplePlayer, Canis::Transform, Canis::Rigidbody>();

        for (auto [entityHandle, player, transform, rigidbody] : view.each())
        {
            if (player.entity == nullptr || !player.entity->active)
                continue;

            if (!player.initialized)
            {
                player.totalPickups = CountPickups(GetScene());
                player.collectedPickups = 0;
                player.initialized = true;

                if (player.logInstructions)
                {
                    Canis::Debug::Log("ECS Sample: this scene uses native components + systems, not ScriptableEntity gameplay scripts.");
                    Canis::Debug::Log("ECS Sample: try modifying Spin, Hover, Collectible, or SamplePlayer data to change behavior.");
                }
            }

            Canis::RaycastHit groundHit = {};
            const Canis::Vector3 rayOrigin = transform.GetGlobalPosition() + Canis::Vector3(0.0f, 0.1f, 0.0f);
            player.grounded = GetScene().Raycast(
                rayOrigin,
                Canis::Vector3(0.0f, -1.0f, 0.0f),
                groundHit,
                player.groundCheckDistance,
                player.groundCollisionMask);

            Canis::Vector3 movement = Canis::Vector3(0.0f);
            Canis::InputManager& input = GetScene().GetInputManager();

            if (input.GetKey(Canis::Key::A) || input.GetKey(Canis::Key::LEFT))
                movement.x -= 1.0f;
            if (input.GetKey(Canis::Key::D) || input.GetKey(Canis::Key::RIGHT))
                movement.x += 1.0f;
            if (input.GetKey(Canis::Key::W) || input.GetKey(Canis::Key::UP))
                movement.z -= 1.0f;
            if (input.GetKey(Canis::Key::S) || input.GetKey(Canis::Key::DOWN))
                movement.z += 1.0f;

            if (movement != Canis::Vector3(0.0f))
                movement = glm::normalize(movement);

            if (player.grounded && input.JustPressedKey(Canis::Key::SPACE))
                rigidbody.AddForce(Canis::Vector3(0.0f, player.jumpImpulse, 0.0f), Canis::Rigidbody3DForceMode::IMPULSE);

            rigidbody.AddForce(movement * player.moveForce * _deltaTime, Canis::Rigidbody3DForceMode::FORCE);
        }
    }

    void SpinSystem::Create()
    {
        m_name = "ECSExample::SpinSystem";
    }

    void SpinSystem::Update(entt::registry& _registry, float _deltaTime)
    {
        auto view = _registry.view<ECSExample::Spin, Canis::Transform>();

        for (auto [entityHandle, spin, transform] : view.each())
        {
            if (spin.entity == nullptr || !spin.entity->active)
                continue;

            transform.rotation.y += spin.speedDegrees * Canis::DEG2RAD * _deltaTime;
        }
    }

    void HoverSystem::Create()
    {
        m_name = "ECSExample::HoverSystem";
    }

    void HoverSystem::Update(entt::registry& _registry, float _deltaTime)
    {
        auto view = _registry.view<ECSExample::Hover, Canis::Transform>();

        for (auto [entityHandle, hover, transform] : view.each())
        {
            if (hover.entity == nullptr || !hover.entity->active)
                continue;

            if (!hover.initialized)
            {
                hover.baseHeight = transform.position.y;
                hover.elapsed = 0.0f;
                hover.initialized = true;
            }

            hover.elapsed += _deltaTime;
            transform.position.y = hover.baseHeight + (std::sin(hover.elapsed * hover.speed) * hover.amplitude);
        }
    }

    void CollectibleSystem::Create()
    {
        m_name = "ECSExample::CollectibleSystem";
    }

    void CollectibleSystem::Update(entt::registry& _registry, float _deltaTime)
    {
        (void)_deltaTime;

        auto view = _registry.view<ECSExample::Collectible, Canis::BoxCollider>();

        for (auto [entityHandle, collectible, boxCollider] : view.each())
        {
            if (collectible.entity == nullptr || !collectible.entity->active)
                continue;

            for (Canis::Entity* other : boxCollider.entered)
            {
                if (other == nullptr || !other->active)
                    continue;

                if (other->tag != collectible.collectorTag || !other->HasComponent<ECSExample::SamplePlayer>())
                    continue;

                ECSExample::SamplePlayer& player = other->GetComponent<ECSExample::SamplePlayer>();
                player.collectedPickups++;
                collectible.entity->Destroy();
                break;
            }
        }
    }

    void SampleHUDSystem::Create()
    {
        m_name = "ECSExample::SampleHUDSystem";
    }

    void SampleHUDSystem::Update(entt::registry& _registry, float _deltaTime)
    {
        (void)_deltaTime;

        auto view = _registry.view<ECSExample::SampleHUD, Canis::Text>();

        for (auto [entityHandle, hud, text] : view.each())
        {
            if (hud.entity == nullptr || !hud.entity->active)
                continue;

            if (hud.player == nullptr || !hud.player->active)
            {
                continue;
            }

            std::string message = "Collected: -- / --";

            if (hud.player != nullptr && hud.player->HasComponent<ECSExample::SamplePlayer>())
            {
                const ECSExample::SamplePlayer& player = hud.player->GetComponent<ECSExample::SamplePlayer>();
                message = "Collected: " + std::to_string(player.collectedPickups);
                message += " / " + std::to_string(player.totalPickups);
            }

            text.SetText(message);
        }
    }
}
