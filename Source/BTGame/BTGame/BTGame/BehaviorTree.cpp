#include "common.h"
#include "BehaviorTree.h"

NodeStatus CBehaviorNode::Tick(TickContext& context)
{
    m_active = true;
    const NodeStatus result = Execute(context);
    if (result != NodeStatus::Running)
        Reset();
    return result;
}

void CBehaviorNode::Reset()
{
    // 부모가 다시 Reset을 요청해도 이미 정리한 노드는 초기화하지 않는다.
    if (!m_active)
        return;
    m_active = false;
    OnReset();
}

void CComposite::AddChild(std::unique_ptr<CBehaviorNode> child)
{
    m_children.push_back(std::move(child));
}

void CComposite::OnReset()
{
    for (const auto& child : m_children)
        child->Reset();
}

NodeStatus CSelector::Execute(TickContext& context)
{
    for (const auto& child : m_children)
    {
        const NodeStatus result = child->Tick(context);
        if (result == NodeStatus::Failure)
            continue;

        // 우선순위가 바뀌었을 때 이전에 진행 중이던 가지만 취소한다.
        if (m_runningChild && m_runningChild != child.get())
            m_runningChild->Reset();
        m_runningChild = result == NodeStatus::Running ? child.get() : nullptr;
        return result;
    }
    return NodeStatus::Failure;
}

void CSelector::OnReset()
{
    if (m_runningChild)
        m_runningChild->Reset();
    m_runningChild = nullptr;
}

NodeStatus CSequence::Execute(TickContext& context)
{
    while (m_currentChild < m_children.size())
    {
        const NodeStatus result = m_children[m_currentChild]->Tick(context);
        if (result != NodeStatus::Success)
            return result;
        ++m_currentChild;
    }
    return NodeStatus::Success;
}

void CSequence::OnReset()
{
    m_currentChild = 0;
    CComposite::OnReset();
}

NodeStatus CReactiveSequence::Execute(TickContext& context)
{
    for (std::size_t i = 0; i < m_children.size(); ++i)
    {
        const NodeStatus result = m_children[i]->Tick(context);
        if (result == NodeStatus::Failure)
            return result;
        if (result == NodeStatus::Running)
        {
            for (std::size_t later = i + 1; later < m_children.size(); ++later)
                m_children[later]->Reset();
            return result;
        }
    }
    return NodeStatus::Success;
}

CCondition::CCondition(std::function<bool()> predicate)
    : m_predicate(std::move(predicate))
{
}

NodeStatus CCondition::Execute(TickContext&)
{
    return m_predicate() ? NodeStatus::Success : NodeStatus::Failure;
}

CAction::CAction(std::function<NodeStatus()> action, std::function<void()> onReset)
    : m_action(std::move(action)), m_onReset(std::move(onReset))
{
}

NodeStatus CAction::Execute(TickContext& context)
{
    // 한 인물은 기존과 같이 한 Tick에 행동을 한 번만 실행한다.
    if (context.actionExecuted)
        return NodeStatus::Running;

    context.actionExecuted = true;
    m_hasExecuted = true;
    return m_action();
}

void CAction::OnReset()
{
    // 실행 기회를 기다리기만 한 행동에는 정리 콜백이 필요하지 않다.
    if (!m_hasExecuted)
        return;
    m_hasExecuted = false;
    if (m_onReset)
        m_onReset();
}

void CBehaviorTree::SetRoot(std::unique_ptr<CBehaviorNode> root)
{
    m_root = std::move(root);
}

NodeStatus CBehaviorTree::Tick()
{
    TickContext context;
    return m_root ? m_root->Tick(context) : NodeStatus::Failure;
}
