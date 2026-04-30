#include <AICombat/HealWand.hpp>

#include <AICombat/HealerStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

#include <algorithm>
#include <limits>

namespace AICombat
{
    namespace
    {
        ScriptConf healWandConf = {};
    }

    void RegisterHealWandScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(healWandConf, AICombat::Heal, owner);
        REGISTER_PROPERTY(healWandConf, AICombat::Heal, sensorSize);
        REGISTER_PROPERTY(healWandConf, AICombat::Heal, healAmount);
        REGISTER_PROPERTY(healWandConf, AICombat::Heal, targetTag);


        DEFAULT_CONFIG_AND_REQUIRED(
            healWandConf,
            AICombat::Heal,
            Canis::Transform,
            Canis::Rigidbody,
            Canis::BoxCollider);

        healWandConf.DEFAULT_DRAW_INSPECTOR(AICombat::Heal);

        _app.RegisterScript(healWandConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(healWandConf, Heal)

    void Heal::Create()
    {
        entity.GetComponent<Canis::Transform>();

        Canis::Rigidbody& rigidbody = entity.GetComponent<Canis::Rigidbody>();
        rigidbody.motionType = Canis::RigidbodyMotionType::STATIC;
        rigidbody.useGravity = false;
        rigidbody.isSensor = true;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Canis::Vector3(0.0f);
        rigidbody.angularVelocity = Canis::Vector3(0.0f);

        entity.GetComponent<Canis::BoxCollider>().size = sensorSize;
    }

    void Heal::Ready()
    {
        if (owner == nullptr)
            owner = FindOwnerFromHierarchy();

        if (targetTag.empty())
        {
            if (HealerStateMachine* ownerStateMachine = GetOwnerStateMachine())
                targetTag = ownerStateMachine->targetTag;
        }
    }

    void Heal::Update(float)
    {
        CheckSensorEnter();
    }

    void Heal::CheckSensorEnter()
    {
        if (!entity.HasComponents<Canis::BoxCollider, Canis::Rigidbody>())
            return;

        HealerStateMachine* ownerStateMachine = GetOwnerStateMachine();
        if (ownerStateMachine == nullptr || !ownerStateMachine->IsAlive())
        {
            m_healedTargetsThisSwing.clear();
            return;
        }


        std::vector<HealerStateMachine*> candidates;

        for (Canis::Entity* other : entity.GetComponent<Canis::BoxCollider>().entered)
        {
            if (other == nullptr || !other->active || other == owner || HasHealedThisSwing(*other))
                continue;

            HealerStateMachine* targetStateMachine = other->GetScript<HealerStateMachine>();
            if (targetStateMachine == nullptr || !targetStateMachine->IsAlive())
                continue;

            if (other->tag != targetTag)
                continue;

            candidates.push_back(targetStateMachine);
        }

        if (candidates.empty())
            return;

        // Find the one with least health
        HealerStateMachine* targetToHeal = nullptr;
        int minHealth = std::numeric_limits<int>::max();
        for (HealerStateMachine* candidate : candidates)
        {
            int health = candidate->GetCurrentHealth();
            if (health < minHealth)
            {
                minHealth = health;
                targetToHeal = candidate;
            }
        }

        if (targetToHeal != nullptr)
        {
            targetToHeal->TakeHeal(healAmount);
            m_healedTargetsThisSwing.push_back(&targetToHeal->entity);
        }
    }

    HealerStateMachine* Heal::GetOwnerStateMachine()
    {
        if (owner == nullptr)
            owner = FindOwnerFromHierarchy();

        if (owner == nullptr || !owner->active)
            return nullptr;

        return owner->GetScript<HealerStateMachine>();
    }

    Canis::Entity* Heal::FindOwnerFromHierarchy() const
    {
        if (!entity.HasComponent<Canis::Transform>())
            return nullptr;

        Canis::Entity* current = entity.GetComponent<Canis::Transform>().parent;
        while (current != nullptr)
        {
            if (current->HasScript<HealerStateMachine>())
                return current;

            if (!current->HasComponent<Canis::Transform>())
                break;

            current = current->GetComponent<Canis::Transform>().parent;
        }

        return nullptr;
    }

    bool Heal::HasHealedThisSwing(Canis::Entity& _target) const
    {
        return std::find(m_healedTargetsThisSwing.begin(), m_healedTargetsThisSwing.end(), &_target) != m_healedTargetsThisSwing.end();
    }

}