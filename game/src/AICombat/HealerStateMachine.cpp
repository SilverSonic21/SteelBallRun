#include <AICombat/HealerStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace AICombat
{
    namespace
    {
        ScriptConf healerStateMachineConf = {};
    }

    void RegisterHealerStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(healerStateMachineConf, AICombat::HealerStateMachine, targetTag);
        REGISTER_PROPERTY(healerStateMachineConf, AICombat::HealerStateMachine, healRange);
        REGISTER_PROPERTY(healerStateMachineConf, AICombat::HealerStateMachine, healAmount);
        REGISTER_PROPERTY(healerStateMachineConf, AICombat::HealerStateMachine, logStateChanges);

        DEFAULT_CONFIG_AND_REQUIRED(
            healerStateMachineConf,
            AICombat::HealerStateMachine,
            Canis::Transform);

        healerStateMachineConf.DEFAULT_DRAW_INSPECTOR(AICombat::HealerStateMachine);

        _app.RegisterScript(healerStateMachineConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(healerStateMachineConf, HealerStateMachine)

    void HealerStateMachine::Create()
    {
        entity.GetComponent<Canis::Transform>();
    }

    void HealerStateMachine::Ready()
    {
        

    }

    void HealerStateMachine::Update(float)
    {
        CheckForHealTargets();
    }

    void HealerStateMachine::CheckForHealTargets()
    {
        if (targetTag.empty())
            return;

        std::vector<HealerStateMachine*> candidates;

        for (Canis::Entity* other : entity.scene.GetAllEntitiesWithComponent<Canis::Transform>())
        {
            if (other == &entity)
                continue;

            if (!other->HasScript<HealerStateMachine>())
                continue;

            HealerStateMachine* targetStateMachine = other->GetScript<HealerStateMachine>();

            if (targetStateMachine == nullptr || !targetStateMachine->IsAlive())
                continue;

            if (HasHealedThisSwing(*other))
                continue;

             if (glm::distance(entity.GetComponent<Canis::Transform>().GetGlobalPosition(),
                               other->GetComponent<Canis::Transform>().GetGlobalPosition()) > healRange)
                continue;

            candidates.push_back(targetStateMachine);
        }
    

        if (candidates.empty())
            return; 
    }
        
    


   void HealerStateMachine::TakeHeal(int _heal)
    {
        if (!IsAlive())
            return;

        const int healToApply = std::max(_heal, 0);
        if (healToApply <= 0)
            return;

        m_currentHealth += healToApply;
    }

    bool HealerStateMachine::IsAlive() const
    {
        return m_currentHealth > 0;
    }
}