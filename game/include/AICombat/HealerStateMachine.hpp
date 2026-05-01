#pragma once

#include <Canis/Entity.hpp>

#include <SuperPupUtilities/StateMachine.hpp>

#include <string>

namespace AICombat
{
    class HealerStateMachine;

    class HealStateU : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "HealStateMachine";
        std::string targetTag = "";
        float detectionRange = 20.0f;
        Canis::Vector3 bodyColliderSize = Canis::Vector3(1.0f);
        float healDuration = 0.75f;

        explicit HealStateU(HealerStateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
        void Exit() override;
    };

    class HealerStateMachine : public SuperPupUtilities::StateMachine
    {
    public:
        static constexpr const char* ScriptName = "AICombat::HealerStateMachine";

        std::string targetTag = "";
        float healRange = 5.0f;
        int healAmount = 10;
        bool logStateChanges = true;
        int health = 100;
        

        explicit HealerStateMachine(Canis::Entity& _entity) : SuperPupUtilities::StateMachine(_entity) {}

        void Create() override;
        
        void Ready() override;
        void Update(float _dt) override;
        void TakeHeal(int _heal);
        bool IsAlive() const;
        Canis::Entity* FindClosestTarget() const;
        void FaceTarget(const Canis::Entity& _target);
        void MoveTowards(const Canis::Entity& _target, float _speed, float _dt);
        void ChangeState(const std::string& _name);
        float GetStateTime() const;
        int GetCurrentHealth() const;


    private:
        void CheckForHealTargets();
        };

    void RegisterHealerStateMachineScript(Canis::App& _app);
    void UnRegisterHealerStateMachineScript(Canis::App& _app);

    

}