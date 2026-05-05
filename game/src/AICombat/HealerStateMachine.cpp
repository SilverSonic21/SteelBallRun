#include <AICombat/HealerStateMachine.hpp>


#include <SuperPupUtilities/SimpleObjectPool.hpp>
#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace AICombat
{
    namespace{
        ScriptConf healStateMachineConf = {};
    }

    SupIdleState::SupIdleState(SuperPupUtilities::StateMachine& _stateMachine) : State(Name, _stateMachine) {}

    void SupIdleState::Enter() {
        if (HealStateMachine* healStatMachine = dynamic_cast<HealStateMachine*>(m_stateMachine))
            healStatMachine->ResetHammerPose();
    }

    void SupIdleState::Update(float)
     {
        if (HealStateMachine* healStatMachine = dynamic_cast<HealStateMachine*>(m_stateMachine))
        {
            if(healStatMachine->FindClosestTarget() != nullptr)
                healStatMachine->ChangeState(SupChaseState::Name);
        }
            
    }
    SupChaseState::SupChaseState(SuperPupUtilities::StateMachine& _stateMachine) : State(Name, _stateMachine){}

    void SupChaseState::Enter()
    {
        if(HealStateMachine* healStatMachine = dynamic_cast<HealStateMachine*>(m_stateMachine))  
            healStatMachine->ResetHammerPose();
    }

    void SupChaseState::Update(float _dt)
    {
        HealStateMachine* healStatMachine = dynamic_cast<HealStateMachine*>(m_stateMachine);
        if (healStatMachine == nullptr)
            return;
        Canis::Entity* target = healStatMachine->FindClosestTarget();

        if(target == nullptr)
        {
            healStatMachine->ChangeState(SupIdleState::Name);
            return;
        }
        healStatMachine->FaceTarget(*target);

        if(healStatMachine->DistanceTo(*target) <= healStatMachine->GetAttackRange())
        {
            healStatMachine->ChangeState(HealState::Name);
            return;
        }
        healStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }
    HealState::HealState(SuperPupUtilities::StateMachine& _stateMachine) : State(Name, _stateMachine){}

    void HealState::Enter()
    {
        if (HealStateMachine* healStatMachine = dynamic_cast<HealStateMachine*>(m_stateMachine))
            healStatMachine->SetHammerSwing(0.0f);
    }

    void HealState::Update(float _dt)
    {
        HealStateMachine* healStatMachine = dynamic_cast<HealStateMachine*>(m_stateMachine);
        if(healStatMachine == nullptr)
            return;
        
        if (Canis::Entity* target = healStatMachine->FindClosestTarget())
            healStatMachine->FaceTarget(*target);
        
        const float duration = std::max(castDuration, 0.001f);
        healStatMachine->SetHammerSwing(healStatMachine->GetStateTime() / duration);

        if(healStatMachine->GetStateTime() >= HealTime && healStatMachine->GetStateTime() < HealTime + _dt)
        {
            healStatMachine->FireProjectile();
        }

        if(healStatMachine->GetStateTime() < duration)
            return;
        
        if(healStatMachine->FindClosestTarget() != nullptr)
            healStatMachine->ChangeState(SupChaseState::Name);
        else
            healStatMachine->ChangeState(SupIdleState::Name);
        
        
    }

    void HealState::Exit()
    {
        if(HealStateMachine* healStatMachine = dynamic_cast<HealStateMachine*>(m_stateMachine))  
            healStatMachine->ResetHammerPose();
    }
        
    HealStateMachine::HealStateMachine(Canis::Entity& _entity) :
        SuperPupUtilities::StateMachine(_entity),
        supidleState(*this),
        supchaseState(*this),
        healState(*this) {}

    void RegisterHealStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, targetTag);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, detectionRange);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, bodyColliderSize);
        RegisterAccessorProperty(healStateMachineConf, AICombat::HealStateMachine, supchaseState, moveSpeed);
        RegisterAccessorProperty(healStateMachineConf, AICombat::HealStateMachine, healState, hammerRestDegrees);
        RegisterAccessorProperty(healStateMachineConf, AICombat::HealStateMachine, healState, hammerSwingDegrees);
        RegisterAccessorProperty(healStateMachineConf, AICombat::HealStateMachine, healState, attackRange);
        RegisterAccessorProperty(healStateMachineConf, AICombat::HealStateMachine, healState, castDuration);
        RegisterAccessorProperty(healStateMachineConf, AICombat::HealStateMachine, healState, HealTime);
        RegisterAccessorProperty(healStateMachineConf, AICombat::HealStateMachine, healState, spellHeal);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, maxHealth);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, logStateChanges);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, hammerVisual);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, hitSfxPath1);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, hitSfxPath2);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, hitSfxVolume);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, deathEffectPrefab);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, projectileCode);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, projectileSpeed);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, projectileLifetime);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, projectileHeal);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, projectileHitImpulse);
        REGISTER_PROPERTY(healStateMachineConf, AICombat::HealStateMachine, allyTag);
        

        DEFAULT_CONFIG_AND_REQUIRED(
            healStateMachineConf,
            AICombat::HealStateMachine,
            Canis::Transform,
            Canis::Material,
            Canis::Model,
            Canis::Rigidbody,
            Canis::BoxCollider);

        healStateMachineConf.DEFAULT_DRAW_INSPECTOR(AICombat::HealStateMachine);

        _app.RegisterScript(healStateMachineConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(healStateMachineConf, HealStateMachine)

    void HealStateMachine::Create()
    {
        entity.GetComponent<Canis::Transform>();

        Canis::Rigidbody& rigidbody = entity.GetComponent<Canis::Rigidbody>();
        rigidbody.motionType = Canis::RigidbodyMotionType::KINEMATIC;
        rigidbody.useGravity = false;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Canis::Vector3(0.0f);
        rigidbody.angularVelocity = Canis::Vector3(0.0f);

        entity.GetComponent<Canis::BoxCollider>().size = bodyColliderSize;

        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }
    }

    void HealStateMachine::Ready()
    {
        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }

        m_currentHealth = std::max(maxHealth, 1);
        m_stateTime = 0.0f;
        m_useFirstHitSfx = true;

        ClearStates();
        AddState(supidleState);
        AddState(supchaseState);
        AddState(healState);

        ResetHammerPose();
        ChangeState(SupIdleState::Name);
    }

    void HealStateMachine::Destroy()
    {
        hammerVisual = nullptr;
        SuperPupUtilities::StateMachine::Destroy();
    }

    void HealStateMachine::Update(float _dt)
    {
        if (!IsAlive())
            return;

        m_stateTime += _dt;
        SuperPupUtilities::StateMachine::Update(_dt);
    }

    Canis::Entity* HealStateMachine::FindClosestTarget() const
    {
        if (targetTag.empty() || !entity.HasComponent<Canis::Transform>())
            return nullptr;

        const Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 origin = transform.GetGlobalPosition();
        Canis::Entity* closestTarget = nullptr;
        float closestDistance = detectionRange;

        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag(targetTag))
        {
            if (candidate == nullptr || candidate == &entity || !candidate->active)
                continue;

            if (!candidate->HasComponent<Canis::Transform>())
                continue;

            if (const HealStateMachine* other = candidate->GetScript<HealStateMachine>())
            {
                if (!other->IsAlive())
                    continue;
            }

            const Canis::Vector3 candidatePosition = candidate->GetComponent<Canis::Transform>().GetGlobalPosition();
            const float distance = glm::distance(origin, candidatePosition);

            if (distance > detectionRange || distance >= closestDistance)
                continue;

            closestDistance = distance;
            closestTarget = candidate;
        }

        return closestTarget;
    }

    float HealStateMachine::DistanceTo(const Canis::Entity& _other) const
    {
        if (!entity.HasComponent<Canis::Transform>() || !_other.HasComponent<Canis::Transform>())
            return std::numeric_limits<float>::max();

        const Canis::Vector3 selfPosition = entity.GetComponent<Canis::Transform>().GetGlobalPosition();
        const Canis::Vector3 targetPosition = _other.GetComponent<Canis::Transform>().GetGlobalPosition();
        return glm::distance(selfPosition, targetPosition);
    }

    void HealStateMachine::FaceTarget(const Canis::Entity& _target)
    {
        if (!entity.HasComponent<Canis::Transform>() || !_target.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 selfPosition = transform.GetGlobalPosition();
        Canis::Vector3 direction = _target.GetComponent<Canis::Transform>().GetGlobalPosition() - selfPosition;
        direction.y = 0.0f;

        if (glm::dot(direction, direction) <= 0.0001f)
            return;

        direction = glm::normalize(direction);
        transform.rotation.y = std::atan2(-direction.x, -direction.z);
    }

    void HealStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
    {
        if (!entity.HasComponent<Canis::Transform>() || !_target.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 selfPosition = transform.GetGlobalPosition();
        Canis::Vector3 direction = _target.GetComponent<Canis::Transform>().GetGlobalPosition() - selfPosition;
        direction.y = 0.0f;

        if (glm::dot(direction, direction) <= 0.0001f)
            return;

        direction = glm::normalize(direction);
        transform.position += direction * _speed * _dt;
    }

    void HealStateMachine::ChangeState(const std::string& _stateName)
    {
        if (SuperPupUtilities::StateMachine::GetCurrentStateName() == _stateName)
            return;

        if (!SuperPupUtilities::StateMachine::ChangeState(_stateName))
            return;

        m_stateTime = 0.0f;

        if (logStateChanges)
            Canis::Debug::Log("%s -> %s", entity.name.c_str(), _stateName.c_str());
    }

    const std::string& HealStateMachine::GetCurrentStateName() const
    {
        return SuperPupUtilities::StateMachine::GetCurrentStateName();
    }

    float HealStateMachine::GetStateTime() const
    {
        return m_stateTime;
    }

    float HealStateMachine::GetAttackRange() const
    {
        return healState.attackRange;
    }

    int HealStateMachine::GetCurrentHealth() const
    {
        return m_currentHealth;
    }

    void HealStateMachine::ResetHammerPose()
    {
        SetHammerSwing(0.0f);
    }

    void HealStateMachine::SetHammerSwing(float _normalized)
    {
        if (hammerVisual == nullptr || !hammerVisual->HasComponent<Canis::Transform>())
            return;

        Canis::Transform& hammerTransform = hammerVisual->GetComponent<Canis::Transform>();
        const float normalized = Clamp01(_normalized);
        const float swingBlend = (normalized <= 0.5f)
            ? normalized * 2.0f
            : (1.0f - normalized) * 2.0f;

        hammerTransform.rotation.x = DEG2RAD *
            (healState.hammerRestDegrees + (healState.hammerSwingDegrees * swingBlend));
    }

    void HealStateMachine::TakeHeal(int _heal)
    {
        if (!IsAlive())
            return;

        const int healToApply = std::max(_heal, 0);
        if (healToApply <= 0)
            return;

        m_currentHealth = std::min(maxHealth, m_currentHealth + healToApply);
        PlayHitSfx();

        if (m_hasBaseColor && entity.HasComponent<Canis::Material>())
        {
            Canis::Material& material = entity.GetComponent<Canis::Material>();
            const float healthRatio = (maxHealth > 0)
                ? (static_cast<float>(m_currentHealth) / static_cast<float>(maxHealth))
                : 0.0f;

            material.color = Canis::Vector4(
                m_baseColor.x * (0.5f + (0.5f * healthRatio)),
                m_baseColor.y * (0.5f + (0.5f * healthRatio)),
                m_baseColor.z * (0.5f + (0.5f * healthRatio)),
                m_baseColor.w);
        }

        if (m_currentHealth > 0)
            return;

        if (logStateChanges)
            Canis::Debug::Log("%s was defeated.", entity.name.c_str());

        SpawnDeathEffect();
        entity.Destroy();
    }

    void HealStateMachine::FireProjectile()
    {
        auto* pool = SuperPupUtilities::SimpleObjectPool::Instance;
        if (!entity.HasComponent<Canis::Transform>() || pool == nullptr)
            return;
        
        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        Canis::Vector3 position = transform.GetGlobalPosition();

        Canis::Vector3 forward = transform.GetForward();
        
        position += forward * 1.5f;

        Canis::Vector3 rotation = Canis::Vector3(0.0f, transform.rotation.y, 0.0f);
        
        Canis::Entity* projectile = pool->Spawn(projectileCode, position, rotation);
        if(projectile == nullptr)
            return;  
        
        // handel 
        /*if (auto* vac = projectile->GetScript<SuperPupUtilities::Vac>())
        {
            vac->heal = static_cast<int>(projectileHeal);
            vac->speed = projectileSpeed;
            vac->lifeTime = projectileLifetime;
            vac->hitImpulse = projectileHitImpulse;
            vac->collisionMask = static_cast<Canis::Mask>(-1);
            if (!allyTag.empty())
                vac->targetTags.push_back(allyTag);
            vac->Launch();
        }*/
      if(projectile != nullptr){
            return;
        }

    }

    void HealStateMachine::PlayHitSfx()
    {
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;

        Canis::AudioManager::PlaySFX(selectedSfx, std::clamp(hitSfxVolume, 0.0f, 1.0f));
    }

    void HealStateMachine::SpawnDeathEffect()
    {
        if (deathEffectPrefab.Empty() || !entity.HasComponent<Canis::Transform>())
            return;

        const Canis::Transform& sourceTransform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 spawnPosition = sourceTransform.GetGlobalPosition();
        const Canis::Vector3 spawnRotation = sourceTransform.GetGlobalRotation();

        for (Canis::Entity* spawnedEntity : entity.scene.Instantiate(deathEffectPrefab))
        {
            if (spawnedEntity == nullptr || !spawnedEntity->HasComponent<Canis::Transform>())
                continue;

            Canis::Transform& spawnedTransform = spawnedEntity->GetComponent<Canis::Transform>();
            spawnedTransform.position = spawnPosition;
            spawnedTransform.rotation = spawnRotation;
        }
    }

    bool HealStateMachine::IsAlive() const
    {
        return m_currentHealth > 0;
    }
}
