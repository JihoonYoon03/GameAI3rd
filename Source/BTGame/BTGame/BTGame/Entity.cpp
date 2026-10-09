#include "common.h"
#include "Entity.h"
#include "GameConfig.h"

void CBaseEntity::Update()
{
    m_behaviorTree.Tick();
}

CMerchant::CMerchant(Blackboard& blackboard) : CBaseEntity(blackboard)
{
    BuildBehaviorTree();
}

void CMerchant::BuildBehaviorTree()
{
    // Selector
    //   ReactiveSequence: 도움 요청 중인가? -> Sequence(호출, 대기)
    //   판매
    auto root = std::make_unique<CSelector>();
    auto needsHelp = std::make_unique<CReactiveSequence>();
    needsHelp->AddChild(std::make_unique<CCondition>(
        [this] { return m_blackboard.IsGuardRequested(); }));

    auto callAndWait = std::make_unique<CSequence>();
    callAndWait->AddChild(std::make_unique<CAction>([this] { return CallGuard(); }));
    callAndWait->AddChild(std::make_unique<CAction>([this] { return WaitForGuard(); }));
    needsHelp->AddChild(std::move(callAndWait));

    root->AddChild(std::move(needsHelp));
    root->AddChild(std::make_unique<CAction>([this] { return Sell(); }));
    m_behaviorTree.SetRoot(std::move(root));
}

NodeStatus CMerchant::Sell()
{
    std::cout << "상인: 물건 사세요~ (남은 물건: " << GetSellingItems() << "개)\n";
    return NodeStatus::Success;
}

NodeStatus CMerchant::CallGuard()
{
    // 도움 요청 자체는 Tick 끝의 도난 인지 시점에 Blackboard에 반영되어 있다.
    std::cout << "상인: 도둑이야! 경비병님, 도와주세요!\n";
    return NodeStatus::Success;
}

NodeStatus CMerchant::WaitForGuard()
{
    std::cout << "상인: 경비병이 돌아오기를 기다리는 중...\n";
    return NodeStatus::Running;
}

CThief::CThief(Blackboard& blackboard) : CBaseEntity(blackboard)
{
    BuildBehaviorTree();
}

void CThief::Update()
{
    if (!IsCaught())
        CBaseEntity::Update();
}

void CThief::BuildBehaviorTree()
{
    // Selector
    //   ReactiveSequence: 경비병이 출동 중인가? -> 도주
    //   Sequence: 배회 -> 절도
    // 도주는 최우선 가지이므로 배회와 절도 어느 도중에도 끼어들 수 있다.
    auto root = std::make_unique<CSelector>();
    auto danger = std::make_unique<CReactiveSequence>();
    danger->AddChild(std::make_unique<CCondition>(
        [this] { return m_blackboard.IsGuardRequested(); }));
    danger->AddChild(std::make_unique<CAction>([this] { return Escape(); }));

    auto searchAndSteal = std::make_unique<CSequence>();
    searchAndSteal->AddChild(std::make_unique<CAction>(
        [this] { return Wander(); },
        [this] { m_wanderTicks = 0; }));
    searchAndSteal->AddChild(std::make_unique<CAction>([this] { return Steal(); }));

    root->AddChild(std::move(danger));
    root->AddChild(std::move(searchAndSteal));
    m_behaviorTree.SetRoot(std::move(root));
}

NodeStatus CThief::Wander()
{
    std::cout << "도둑: 훔칠만한 물건 탐색 중...\n";
    ++m_wanderTicks;
    return m_wanderTicks >= GameConfig::WanderTicks ? NodeStatus::Success : NodeStatus::Running;
}

NodeStatus CThief::Steal()
{
    if (m_blackboard.StealOneItem())
        std::cout << "도둑: 물건 하나를 훔쳤다! (훔친 물건: " << GetStolenItems() << "개)\n";
    else
        std::cout << "도둑: 더 이상 훔칠 물건이 없다...\n";
    return NodeStatus::Success;
}

NodeStatus CThief::Escape()
{
    std::cout << "도둑: 경비병을 피해 도망치는 중...\n";
    return NodeStatus::Running;
}

CGuard::CGuard(Blackboard& blackboard) : CBaseEntity(blackboard)
{
    BuildBehaviorTree();
}

void CGuard::BuildBehaviorTree()
{
    // Selector
    //   ReactiveSequence: 도움 요청이 있는가? -> Sequence(추격, 복귀)
    //   순찰
    auto root = std::make_unique<CSelector>();
    auto requested = std::make_unique<CReactiveSequence>();
    requested->AddChild(std::make_unique<CCondition>(
        [this] { return m_blackboard.IsGuardRequested(); }));

    auto chaseAndReturn = std::make_unique<CSequence>();
    chaseAndReturn->AddChild(std::make_unique<CAction>(
        [this] { return Chase(); },
        [this] { m_chaseTicks = 0; }));
    chaseAndReturn->AddChild(std::make_unique<CAction>(
        [this] { return ReturnToPatrol(); },
        [this] { m_returnTicks = 0; }));
    requested->AddChild(std::move(chaseAndReturn));

    root->AddChild(std::move(requested));
    root->AddChild(std::make_unique<CAction>([this] { return Patrol(); }));
    m_behaviorTree.SetRoot(std::move(root));
}

NodeStatus CGuard::Patrol()
{
    std::cout << "경비병: 순찰 중...\n";
    return NodeStatus::Success;
}

NodeStatus CGuard::Chase()
{
    ++m_chaseTicks;
    if (std::rand() % 100 < GameConfig::CatchThiefPercent)
    {
        std::cout << "경비병: 도둑을 잡았다!\n";
        m_blackboard.CatchThief();
        return NodeStatus::Success;
    }
    // Success는 추격 단계 완료를 의미하므로 시간 초과 뒤에도 복귀 노드를 실행한다.
    if (m_chaseTicks >= GameConfig::ChaseTicks)
    {
        std::cout << "경비병: 도둑을 놓쳤다. 추격을 중단하고 복귀한다...\n";
        return NodeStatus::Success;
    }

    std::cout << "경비병: 도둑을 추격 중... (" << m_chaseTicks << "/"
        << GameConfig::ChaseTicks << ")\n";
    return NodeStatus::Running;
}

NodeStatus CGuard::ReturnToPatrol()
{
    ++m_returnTicks;
    if (m_returnTicks >= GameConfig::ReturnTicks)
    {
        std::cout << "경비병: 복귀 완료. 다시 순찰을 시작한다.\n";
        m_blackboard.CompleteGuardReturn();
        return NodeStatus::Success;
    }

    std::cout << "경비병: 순찰 장소로 복귀 중...\n";
    return NodeStatus::Running;
}
