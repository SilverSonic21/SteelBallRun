#pragma once

#include <Canis/Entity.hpp>

#include <SuperPupUtilities/StateMachine.hpp>

#include <string>

namespace AICombat
{
    class HealerStateMachine;

    class HealerStateMachine : public SuperPupUtilities::StateMachine
    {
    public:
        static constexpr const char* ScriptName = "AICombat::HealerStateMachine";

        std::string targetTag = "";
        float healRange = 5.0f;
        int healAmount = 10;
        bool logStateChanges = true;

        explicit HealerStateMachine(Canis::Entity& _entity) : SuperPupUtilities::StateMachine(_entity) {}

        void Create() override;
        void Ready() override;
        void Update(float _dt) override;
        void TakeHeal(int _heal);
        bool IsAlive() const;

    private:
        void CheckForHealTargets();
        };

    void RegisterHealerStateMachineScript(Canis::App& _app);
    void UnRegisterHealerStateMachineScript(Canis::App& _app);

    

}