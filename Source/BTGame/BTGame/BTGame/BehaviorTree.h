#pragma once

enum class NodeStatus
{
    Success,
    Failure,
    Running
};

// 조건 검사는 자유롭게 하되, 인물의 실제 행동은 한 Tick에 한 번만 실행한다.
struct TickContext
{
    bool actionExecuted = false;
};

class CBehaviorNode
{
public:
    virtual ~CBehaviorNode() = default;
    // 완료와 취소 시의 초기화는 이 공통 함수에서 한 번만 처리한다.
    NodeStatus Tick(TickContext& context);
    void Reset();

protected:
    virtual NodeStatus Execute(TickContext& context) = 0;
    virtual void OnReset() {}

private:
    bool m_active = false;
};

class CComposite : public CBehaviorNode
{
public:
    void AddChild(std::unique_ptr<CBehaviorNode> child);
protected:
    void OnReset() override;
    std::vector<std::unique_ptr<CBehaviorNode>> m_children;
};

// 매 Tick 첫 자식부터 검사한다. 앞쪽 가지가 더 높은 우선순위다.
class CSelector : public CComposite
{
protected:
    NodeStatus Execute(TickContext& context) override;
    void OnReset() override;

private:
    CBehaviorNode* m_runningChild = nullptr;
};

// Running을 반환한 자식을 기억하고 다음 Tick에 그 위치부터 이어서 실행한다.
class CSequence : public CComposite
{
protected:
    NodeStatus Execute(TickContext& context) override;
    void OnReset() override;

private:
    std::size_t m_currentChild = 0;
};

// 조건을 매 Tick 다시 검사한다. 조건이 깨지면 진행 중이던 하위 행동도 취소한다.
class CReactiveSequence : public CComposite
{
protected:
    NodeStatus Execute(TickContext& context) override;
};

class CCondition : public CBehaviorNode
{
public:
    explicit CCondition(std::function<bool()> predicate);

protected:
    NodeStatus Execute(TickContext& context) override;

private:
    std::function<bool()> m_predicate;
};

class CAction : public CBehaviorNode
{
public:
    CAction(std::function<NodeStatus()> action, std::function<void()> onReset = {});

protected:
    NodeStatus Execute(TickContext& context) override;
    void OnReset() override;

private:
    std::function<NodeStatus()> m_action;
    std::function<void()> m_onReset;
    bool m_hasExecuted = false;
};

class CBehaviorTree
{
public:
    void SetRoot(std::unique_ptr<CBehaviorNode> root);
    NodeStatus Tick();

private:
    std::unique_ptr<CBehaviorNode> m_root;
};
