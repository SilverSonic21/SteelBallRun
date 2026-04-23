#include <AICombat/HealWand.hpp>

#include <AICombat/BrawlerStateMachine.hpp>

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
        REGISTER_PROPERTY(healWandConf, AICombat::HealWand, owner);
        REGISTER_PROPERTY(healWandConf, AICombat::HealWand, sensorSize);
        REGISTER_PROPERTY(healWandConf, AICombat::HealWand, healAmount);
        REGISTER_PROPERTY(healWandConf, AICombat::HealWand, targetTag);

        DEFAULT_CONFIG_AND_REQUIRED(
            healWandConf,
            AICombat::HealWand,
            Canis::Transform,
            Canis::Rigidbody,
            Canis::BoxCollider);

        healWandConf.DEFAULT_DRAW_INSPECTOR(AICombat::HealWand);

        _app.RegisterScript(healWandConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(healWandConf, HealWand)

    void HealWand::Create()
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

    void HealWand::Ready()
    {
        if (owner == nullptr)
            owner = FindOwnerFromHierarchy();

        if (targetTag.empty())
        {
            if (BrawlerStateMachine* ownerStateMachine = GetOwnerStateMachine())
                targetTag = ownerStateMachine->targetTag;
        }
    }

    void HealWand::Update(float)
    {
        CheckSensorEnter();
    }

    void HealWand::CheckSensorEnter()
    {
        if (!entity.HasComponents<Canis::BoxCollider, Canis::Rigidbody>())
            return;

        BrawlerStateMachine* ownerStateMachine = GetOwnerStateMachine();
        if (ownerStateMachine == nullptr || !ownerStateMachine->IsAlive())
        {
            m_healedTargetsThisSwing.clear();
            return;
        }


        std::vector<BrawlerStateMachine*> candidates;

        for (Canis::Entity* other : entity.GetComponent<Canis::BoxCollider>().entered)
        {
            if (other == nullptr || !other->active || other == owner || HasHealedThisSwing(*other))
                continue;

            BrawlerStateMachine* targetStateMachine = other->GetScript<BrawlerStateMachine>();
            if (targetStateMachine == nullptr || !targetStateMachine->IsAlive())
                continue;

            if (other->tag != targetTag)
                continue;

            candidates.push_back(targetStateMachine);
        }

        if (candidates.empty())
            return;

        // Find the one with least health
        BrawlerStateMachine* targetToHeal = nullptr;
        int minHealth = std::numeric_limits<int>::max();
        for (BrawlerStateMachine* candidate : candidates)
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

    BrawlerStateMachine* HealWand::GetOwnerStateMachine()
    {
        if (owner == nullptr)
            owner = FindOwnerFromHierarchy();

        if (owner == nullptr || !owner->active)
            return nullptr;

        return owner->GetScript<BrawlerStateMachine>();
    }

    Canis::Entity* HealWand::FindOwnerFromHierarchy() const
    {
        if (!entity.HasComponent<Canis::Transform>())
            return nullptr;

        Canis::Entity* current = entity.GetComponent<Canis::Transform>().parent;
        while (current != nullptr)
        {
            if (current->HasScript<BrawlerStateMachine>())
                return current;

            if (!current->HasComponent<Canis::Transform>())
                break;

            current = current->GetComponent<Canis::Transform>().parent;
        }

        return nullptr;
    }

    bool HealWand::HasHealedThisSwing(Canis::Entity& _target) const
    {
        return std::find(m_healedTargetsThisSwing.begin(), m_healedTargetsThisSwing.end(), &_target)
            != m_healedTargetsThisSwing.end();
    }
}