#include "common.h"
#include "BehaviorTree.h"

void CComposite::AddChild(std::unique_ptr<CBehaviorNode> child)
{
    m_children.push_back(std::move(child));
}

void CComposite::Reset()
{
    for (const auto& child : m_children)
        child->Reset();
}

NodeStatus CSelector::Tick(TickContext& context)
{
    for (std::size_t i = 0; i < m_children.size(); ++i)
    {
        const NodeStatus result = m_children[i]->Tick(context);
        if (result == NodeStatus::Failure)
        {
            m_children[i]->Reset();
            continue;
        }

        // 예: 도주 가지가 선택되면 배회/절도의 진행 상황을 버린다.
        for (std::size_t other = 0; other < m_children.size(); ++other)
        {
            if (other != i)
                m_children[other]->Reset();
        }
        if (result == NodeStatus::Success)
            m_children[i]->Reset();
        return result;
    }
    return NodeStatus::Failure;
}

NodeStatus CSequence::Tick(TickContext& context)
{
    while (m_currentChild < m_children.size())
    {
        const NodeStatus result = m_children[m_currentChild]->Tick(context);
        if (result == NodeStatus::Running)
            return result;
        if (result == NodeStatus::Failure)
        {
            Reset();
            return result;
        }
        ++m_currentChild;
    }

    Reset();
    return NodeStatus::Success;
}

void CSequence::Reset()
{
    m_currentChild = 0;
    CComposite::Reset();
}

NodeStatus CReactiveSequence::Tick(TickContext& context)
{
    for (std::size_t i = 0; i < m_children.size(); ++i)
    {
        const NodeStatus result = m_children[i]->Tick(context);
        if (result == NodeStatus::Failure)
        {
            Reset();
            return result;
        }
        if (result == NodeStatus::Running)
        {
            for (std::size_t later = i + 1; later < m_children.size(); ++later)
                m_children[later]->Reset();
            return result;
        }
    }

    Reset();
    return NodeStatus::Success;
}

CCondition::CCondition(std::function<bool()> predicate)
    : m_predicate(std::move(predicate))
{
}

NodeStatus CCondition::Tick(TickContext&)
{
    return m_predicate() ? NodeStatus::Success : NodeStatus::Failure;
}

CAction::CAction(std::function<NodeStatus()> action, std::function<void()> onReset)
    : m_action(std::move(action)), m_onReset(std::move(onReset))
{
}

NodeStatus CAction::Tick(TickContext& context)
{
    // Sequence에서 앞 행동이 끝나도 다음 행동/출력은 다음 Tick까지 기다린다.
    if (context.actionExecuted)
        return NodeStatus::Running;

    context.actionExecuted = true;
    const NodeStatus result = m_action();
    if (result != NodeStatus::Running)
        Reset();
    return result;
}

void CAction::Reset()
{
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
