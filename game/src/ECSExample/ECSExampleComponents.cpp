#include <ECSExample/ECSExampleComponents.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

namespace ECSExample
{
    Canis::ComponentConf samplePlayerComponentConf = {};
    Canis::ComponentConf spinComponentConf = {};
    Canis::ComponentConf hoverComponentConf = {};
    Canis::ComponentConf collectibleComponentConf = {};
    Canis::ComponentConf sampleHUDComponentConf = {};

    void RegisterSamplePlayerComponent(Canis::App& _app)
    {
        REGISTER_PROPERTY(samplePlayerComponentConf, ECSExample::SamplePlayer, moveForce);
        REGISTER_PROPERTY(samplePlayerComponentConf, ECSExample::SamplePlayer, jumpImpulse);
        REGISTER_PROPERTY(samplePlayerComponentConf, ECSExample::SamplePlayer, groundCheckDistance);
        REGISTER_PROPERTY(samplePlayerComponentConf, ECSExample::SamplePlayer, groundCollisionMask);
        REGISTER_PROPERTY(samplePlayerComponentConf, ECSExample::SamplePlayer, logInstructions);

        DEFAULT_COMPONENT_CONFIG_AND_REQUIRED(
            samplePlayerComponentConf,
            ECSExample::SamplePlayer,
            Canis::Transform,
            Canis::Rigidbody,
            Canis::SphereCollider);

        samplePlayerComponentConf.DEFAULT_DRAW_COMPONENT_INSPECTOR(ECSExample::SamplePlayer,
            ImGui::Text("Grounded: %s", component->grounded ? "Yes" : "No");
            ImGui::Text("Collected: %d / %d", component->collectedPickups, component->totalPickups);
        );

        _app.RegisterComponent(samplePlayerComponentConf);
    }

    DEFAULT_UNREGISTER_COMPONENT(samplePlayerComponentConf, SamplePlayer)

    void RegisterSpinComponent(Canis::App& _app)
    {
        REGISTER_PROPERTY(spinComponentConf, ECSExample::Spin, speedDegrees);

        DEFAULT_COMPONENT_CONFIG_AND_REQUIRED(spinComponentConf, ECSExample::Spin, Canis::Transform);

        spinComponentConf.DEFAULT_DRAW_COMPONENT_INSPECTOR(ECSExample::Spin);

        _app.RegisterComponent(spinComponentConf);
    }

    DEFAULT_UNREGISTER_COMPONENT(spinComponentConf, Spin)

    void RegisterHoverComponent(Canis::App& _app)
    {
        REGISTER_PROPERTY(hoverComponentConf, ECSExample::Hover, amplitude);
        REGISTER_PROPERTY(hoverComponentConf, ECSExample::Hover, speed);

        DEFAULT_COMPONENT_CONFIG_AND_REQUIRED(hoverComponentConf, ECSExample::Hover, Canis::Transform);

        hoverComponentConf.DEFAULT_DRAW_COMPONENT_INSPECTOR(ECSExample::Hover);

        _app.RegisterComponent(hoverComponentConf);
    }

    DEFAULT_UNREGISTER_COMPONENT(hoverComponentConf, Hover)

    void RegisterCollectibleComponent(Canis::App& _app)
    {
        REGISTER_PROPERTY(collectibleComponentConf, ECSExample::Collectible, collectorTag);

        DEFAULT_COMPONENT_CONFIG_AND_REQUIRED(
            collectibleComponentConf,
            ECSExample::Collectible,
            Canis::Transform,
            Canis::Rigidbody,
            Canis::BoxCollider);

        collectibleComponentConf.DEFAULT_DRAW_COMPONENT_INSPECTOR(ECSExample::Collectible);

        _app.RegisterComponent(collectibleComponentConf);
    }

    DEFAULT_UNREGISTER_COMPONENT(collectibleComponentConf, Collectible)

    void RegisterSampleHUDComponent(Canis::App& _app)
    {
        REGISTER_PROPERTY(sampleHUDComponentConf, ECSExample::SampleHUD, player);

        DEFAULT_COMPONENT_CONFIG_AND_REQUIRED(sampleHUDComponentConf, ECSExample::SampleHUD, Canis::RectTransform, Canis::Text);

        sampleHUDComponentConf.DEFAULT_DRAW_COMPONENT_INSPECTOR(ECSExample::SampleHUD);

        _app.RegisterComponent(sampleHUDComponentConf);
    }

    DEFAULT_UNREGISTER_COMPONENT(sampleHUDComponentConf, SampleHUD)
}
