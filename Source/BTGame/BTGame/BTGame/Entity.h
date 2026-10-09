#pragma once

#include "BehaviorTree.h"
#include "Blackboard.h"

class CBaseEntity
{
public:
    explicit CBaseEntity(Blackboard& blackboard) : m_blackboard(blackboard) {}
    virtual ~CBaseEntity() = default;
    CBaseEntity(const CBaseEntity&) = delete;
    CBaseEntity& operator=(const CBaseEntity&) = delete;

    virtual void Update();

protected:
    Blackboard& m_blackboard;
    CBehaviorTree m_behaviorTree;
};

class CMerchant : public CBaseEntity
{
public:
    explicit CMerchant(Blackboard& blackboard);
    int GetSellingItems() const { return m_blackboard.GetMerchantItems(); }

private:
    void BuildBehaviorTree();
    NodeStatus Sell();
    NodeStatus CallGuard();
    NodeStatus WaitForGuard();
};

class CThief : public CBaseEntity
{
public:
    explicit CThief(Blackboard& blackboard);
    void Update() override;
    unsigned int GetStolenItems() const { return m_blackboard.GetStolenItems(); }
    bool IsCaught() const { return m_blackboard.IsThiefCaught(); }

private:
    void BuildBehaviorTree();
    NodeStatus Wander();
    NodeStatus Steal();
    NodeStatus Escape();

    int m_wanderTicks = 0;
};

class CGuard : public CBaseEntity
{
public:
    explicit CGuard(Blackboard& blackboard);

private:
    void BuildBehaviorTree();
    NodeStatus Patrol();
    NodeStatus Chase();
    NodeStatus ReturnToPatrol();

    int m_chaseTicks = 0;
    int m_returnTicks = 0;
};
