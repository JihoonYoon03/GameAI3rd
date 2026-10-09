#include "common.h"
#include "Entity.h"
#include "GameConfig.h"
#include "MessageManager.h"

int CBaseEntity::m_iNextValidID = 0;

CBaseEntity::CBaseEntity(int id, MessageManager& messages) : m_messages(messages)
{
    SetID(id);
}

void CBaseEntity::SetID(int id)
{
    if (id < m_iNextValidID)
        throw std::invalid_argument("Entity IDs must be nonnegative and increasing.");
    m_ID = id;
    m_iNextValidID = id + 1;
}

void CBaseEntity::SendMessage(int receiver, msgTypes message)
{
    if (!m_messages.DispatchMessage(ID(), receiver, message))
        throw std::logic_error("Message sender or receiver is not registered.");
}

CMerchant::CMerchant(int id, MessageManager& messages)
    : CBaseEntity(id, messages), m_stateMachine(this), m_nSellingItems(GameConfig::InitialItems)
{
    m_stateMachine.SetCurrentState(SELLING::Instance());
}

void CMerchant::Update() { m_stateMachine.Update(); }
bool CMerchant::HandleMessage(const Telegram& telegram) { return m_stateMachine.HandleMessage(telegram); }

bool CMerchant::TakeOneItem()
{
    if (m_nSellingItems <= 0)
        return false;
    --m_nSellingItems;
    return true;
}

CThief::CThief(int id, MessageManager& messages, CMerchant& merchant)
    : CBaseEntity(id, messages), m_stateMachine(this), m_merchant(merchant)
{
    m_stateMachine.SetGlobalState(THIEF_GLOBAL_STATE::Instance());
    m_stateMachine.SetCurrentState(WANDERING::Instance());
}

void CThief::Update()
{
    if (!m_bCaught)
        m_stateMachine.Update();
}

bool CThief::HandleMessage(const Telegram& telegram)
{
    return !m_bCaught && m_stateMachine.HandleMessage(telegram);
}

bool CThief::StealOneItem()
{
    if (m_bCaught || !m_merchant.TakeOneItem())
        return false;
    ++m_nStolenItems;
    SendMessage(m_merchant.ID(), MSG_ITEM_STOLEN);
    return true;
}

CGuard::CGuard(int id, MessageManager& messages, CThief& thief)
    : CBaseEntity(id, messages), m_stateMachine(this), m_thief(thief)
{
    m_stateMachine.SetCurrentState(PATROLLING::Instance());
}

void CGuard::Update() { m_stateMachine.Update(); }
bool CGuard::HandleMessage(const Telegram& telegram) { return m_stateMachine.HandleMessage(telegram); }
