#pragma once

#include <Canis/Entity.hpp>

#include <vector>

namespace AICombat
{
    class HealerStateMachine;

    class Heal : public Canis::ScriptableEntity
    {
    public:
        static constexpr const char* ScriptName = "AICombat::Heal";

        Canis::Entity* owner = nullptr;
        Canis::Vector3 sensorSize = Canis::Vector3(1.0f);
        int healAmount = 10;
        std::string targetTag = "";

        explicit Heal(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Update(float _dt) override;

        void CheckSensorEnter();

    private:
        HealerStateMachine* GetOwnerStateMachine();
        Canis::Entity* FindOwnerFromHierarchy() const;
        bool HasHealedThisSwing(Canis::Entity& _target) const;

        std::vector<Canis::Entity*> m_healedTargetsThisSwing = {};
    };

    void RegisterHealWandScript(Canis::App& _app);
    void UnRegisterHealWandScript(Canis::App& _app);
}
    