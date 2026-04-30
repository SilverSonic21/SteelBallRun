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
        ScriptConf HealerStateMachineConf = {};
    }

    HealStateU::HealStateU(SuperPupUtilities::HealerStateMachine& _HealerStateMachine) :
        State(Name, _HealerStateMachine) {}

    void HealStateU::Enter()
    {
        if (HealerStateMachine* healerStateMachine = dynamic_cast<HealerStateMachine*>(m_HealerStateMachine))
            healerStateMachine->ResetHealPose();
    }

    void HealStateU::Update(float)
    {
        if (HealerStateMachine* healerStateMachine = dynamic_cast<HealerStateMachine*>(m_HealerStateMachine))
        {
            if (healerStateMachine->FindClosestTarget() != nullptr)
                healerStateMachine->ChangeState(ChaseState::Name);
        }
    }

    ChaseState::ChaseState(SuperPupUtilities::HealerStateMachine& _HealerStateMachine) :
        State(Name, _HealerStateMachine) {}

    void ChaseState::Enter()
    {
        if (HealerStateMachine* healerStateMachine = dynamic_cast<HealerStateMachine*>(m_HealerStateMachine))
            healerStateMachine->ResetHealPose();
    }

    void ChaseState::Update(float _dt)
    {
        HealerStateMachine* healerStateMachine = dynamic_cast<HealerStateMachine*>(m_HealerStateMachine);
        if (healerStateMachine == nullptr)
            return;

        Canis::Entity* target = healerStateMachine->FindClosestTarget();

        if (target == nullptr)
        {
            healerStateMachine->ChangeState(HealStateU::Name);
            return;
        }

        healerStateMachine->FaceTarget(*target);

        if (healerStateMachine->DistanceTo(*target) <= healerStateMachine->GetAttackRange())
        {
            healerStateMachine->ChangeState(HealStateU::Name);
            return;
        }

        healerStateMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    HealStateU::HealStateU(SuperPupUtilities::HealerStateMachine& _HealerStateMachine) :
        State(Name, _HealerStateMachine) {}

    void HealStateU::Enter()
    {
        if (HealerStateMachine* healerStateMachine = dynamic_cast<HealerStateMachine*>(m_HealerStateMachine))
            healerStateMachine->SetHealSwing(0.0f);
    }

    void HealStateU::Update(float)
    {
        HealerStateMachine* healerStateMachine = dynamic_cast<HealerStateMachine*>(m_HealerStateMachine);
        if (healerStatMachine == nullptr)
            return;

        if (Canis::Entity* target = healerStateMachine->FindClosestTarget())
            healerStatMachine->FaceTarget(*target);

        const float duration = std::max(attackDuration, 0.001f);
        healerStateMachine->SetHealSwing(healerStateMachine->GetStateTime() / duration);

        if (healerStateMachine->GetStateTime() < duration)
            return;

        if (healerStateMachine->FindClosestTarget() != nullptr)
            healerStateMachine->ChangeState(ChaseState::Name);
        else
            healerStateMachine->ChangeState(HealStateU::Name);
    }

    void HealStateU::Exit()
    {
        if (HealerStateMachine* healerStateMachine = dynamic_cast<HealerStateMachine*>(m_HealerStateMachine))
            healerStateMachine->ResetHealPose();
    }

    HealerStateMachine::HealerStateMachine(Canis::Entity& _entity) :
        SuperPupUtilities::HealerStateMachine(_entity),
        HealStateU(*this),
        chaseState(*this),
        HealStateU(*this) {}

    void RegisterHealStateUScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, targetTag);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, detectionRange);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, bodyColliderSize);
        RegisterAccessorProperty(HealStateUConf, AICombat::HealStateU, chaseState, moveSpeed);
        RegisterAccessorProperty(HealStateUConf, AICombat::HealStateU, HealStateU, HealRestDegrees);
        RegisterAccessorProperty(HealStateUConf, AICombat::HealStateU, HealStateU, HealSwingDegrees);
        RegisterAccessorProperty(HealStateUConf, AICombat::HealStateU, HealStateU, attackRange);
        RegisterAccessorProperty(HealStateUConf, AICombat::HealStateU, HealStateU, attackDuration);
        RegisterAccessorProperty(HealStateUConf, AICombat::HealStateU, HealStateU, attackDamageTime);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, maxHealth);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, logStateChanges);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, HealVisual);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, hitSfxPath1);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, hitSfxPath2);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, hitSfxVolume);
        REGISTER_PROPERTY(HealStateUConf, AICombat::HealStateU, deathEffectPrefab);

        DEFAULT_CONFIG_AND_REQUIRED(
            HealStateUConf,
            AICombat::HealHealerStateMachine,
            Canis::Transform,
            Canis::Material,
            Canis::Model,
            Canis::Rigidbody,
            Canis::BoxCollider);

        HealStateUConf.DEFAULT_DRAW_INSPECTOR(AICombat::HealHealerStateMachine);

        _app.RegisterScript(HealStateUConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(HealStateUConf, AICombat::HealHealerStateMachine)

    void AICombat::HealHealerStateMachine::Create()
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

    void AICombat::HealHealerStateMachine::Ready()
    {
        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }

        m_currenthealTimeStateth = std::max(maxhealTimeStateth, 1);
        m_stateTime = 0.0f;
        m_useFirstHitSfx = true;

        ClearStates();
        AddState(HealStateU);
        AddState(chaseState);
        AddState(healTimeStateTimeState);

        ResetHealPose();
        ChangeState(HealStateU::Name);
    }

    void AICombat::HealHealerStateMachine::Destroy()
    {
        HealVisual = nullptr;
        SuperPupUtilities::HealerStateMachine::Destroy();
    }

    void AICombat::HealHealerStateMachine::Update(float _dt)
    {
        if (!IsAlive())
            return;

        m_stateTime += _dt;
        SuperPupUtilities::HealerStateMachine::Update(_dt);
    }

    Canis::Entity* AICombat::HealHealerStateMachine::FindClosestTarget() const
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

            if (const AICombat::HealHealerStateMachine* other = candidate->GetScript<AICombat::HealHealerStateMachine>())
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

    float AICombat::HealHealerStateMachine::DistanceTo(const Canis::Entity& _other) const
    {
        if (!entity.HasComponent<Canis::Transform>() || !_other.HasComponent<Canis::Transform>())
            return std::numeric_limits<float>::max();

        const Canis::Vector3 selfPosition = entity.GetComponent<Canis::Transform>().GetGlobalPosition();
        const Canis::Vector3 targetPosition = _other.GetComponent<Canis::Transform>().GetGlobalPosition();
        return glm::distance(selfPosition, targetPosition);
    }

    void AICombat::HealHealerStateMachine::FaceTarget(const Canis::Entity& _target)
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

    void AICombat::HealHealerStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
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

    void AICombat::HealHealerStateMachine::ChangeState(const std::string& _stateName)
    {
        if (SuperPupUtilities::HealerStateMachine::GetCurrentStateName() == _stateName)
            return;

        if (!SuperPupUtilities::HealerStateMachine::ChangeState(_stateName))
            return;

        m_stateTime = 0.0f;

        if (logStateChanges)
            Canis::Debug::Log("%s -> %s", entity.name.c_str(), _stateName.c_str());
    }

    const std::string& AICombat::HealHealerStateMachine::GetCurrentStateName() const
    {
        return SuperPupUtilities::HealerStateMachine::GetCurrentStateName();
    }

    float AICombat::HealHealerStateMachine::GetStateTime() const
    {
        return m_stateTime;
    }

    float AICombat::HealHealerStateMachine::GetAttackRange() const
    {
        return healTimeStateTimeState.attackRange;
    }

    int AICombat::HealHealerStateMachine::GetCurrenthealTimeStateth() const
    {
        return m_currenthealTimeStateth;
    }

    int AICombat::HealHealerStateMachine::GetCurrentHealth() const
    {
        return health;
    }

    void AICombat::HealHealerStateMachine::ResetHealPose()
    {
        SetHealSwing(0.0f);
    }

    void AICombat::HealHealerStateMachine::SetHealSwing(float _normalized)
    {
        if (HealVisual == nullptr || !HealVisual->HasComponent<Canis::Transform>())
            return;

        Canis::Transform& HealTransform = HealVisual->GetComponent<Canis::Transform>();
        const float normalized = Clamp01(_normalized);
        const float swingBlend = (normalized <= 0.5f)
            ? normalized * 2.0f
            : (1.0f - normalized) * 2.0f;

        HealTransform.rotation.x = DEG2RAD *
            (healTimeStateTimeState.HealRestDegrees + (healTimeStateTimeState.HealSwingDegrees * swingBlend));
    }

    void AICombat::HealHealerStateMachine::TakeDamage(int _damage)
    {
        if (!IsAlive())
            return;

        const int damageToApply = std::max(_damage, 0);
        if (damageToApply <= 0)
            return;

        m_currenthealTimeStateth = std::max(0, m_currenthealTimeStateth - damageToApply);
        PlayHitSfx();

        if (m_hasBaseColor && entity.HasComponent<Canis::Material>())
        {
            Canis::Material& material = entity.GetComponent<Canis::Material>();
            const float healTimeStatethRatio = (maxhealTimeStateth > 0)
                ? (static_cast<float>(m_currenthealTimeStateth) / static_cast<float>(maxhealTimeStateth))
                : 0.0f;

            material.color = Canis::Vector4(
                m_baseColor.x * (0.5f + (0.5f * healTimeStatethRatio)),
                m_baseColor.y * (0.5f + (0.5f * healTimeStatethRatio)),
                m_baseColor.z * (0.5f + (0.5f * healTimeStatethRatio)),
                m_baseColor.w);
        }

        if (m_currenthealTimeStateth > 0)
            return;

        if (logStateChanges)
            Canis::Debug::Log("%s was defeated.", entity.name.c_str());

        SpawnDeathEffect();
        entity.Destroy();
    }

    void AICombat::HealHealerStateMachine::PlayHitSfx()
    {
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;

        Canis::AudioManager::PlaySFX(selectedSfx, std::clamp(hitSfxVolume, 0.0f, 1.0f));
    }

    void AICombat::HealHealerStateMachine::SpawnDeathEffect()
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

    bool AICombat::HealHealerStateMachine::IsAlive() const
    {
        return m_currenthealTimeStateth > 0;
    }

    void AICombat::HealHealerStateMachine::TakehealTimeState(int _healTimeState)
    {
        if (!IsAlive())
            return;

        const int healTimeStateToApply = std::max(_healTimeState, 0);
        if (healTimeStateToApply <= 0)
            return;

        m_currenthealTimeStateth = std::min(maxhealTimeStateth, m_currenthealTimeStateth + healTimeStateToApply);
    }
        
}