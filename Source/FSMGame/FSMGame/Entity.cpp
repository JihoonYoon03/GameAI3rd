#include "common.h"
#include "Entity.h"
#include "GameConfig.h"
#include "MessageManager.h"

CBaseEntity::CBaseEntity(int id, MessageManager& messages) : m_ID(id), m_messages(messages)
{
}

void CBaseEntity::MsgSend(int receiver, msgTypes message)
{
    if (!m_messages.MsgDispatch(ID(), receiver, message))
        throw std::logic_error("Message sender or receiver is not registered.");
}

CMerchant::CMerchant(int id, MessageManager& messages)
    : CBaseEntity(id, messages), m_stateMachine(this), m_nSellingItems(GameConfig::InitialItems)
{
    m_stateMachine.SetCurrentState(SELLING::Instance());
}

void CMerchant::Update() 
{ 
    m_stateMachine.Update(); 
}

bool CMerchant::HandleMessage(const Telegram& telegram) 
{
    return m_stateMachine.HandleMessage(telegram); 
}

bool CMerchant::TakeOneItem()
{
    if (m_nSellingItems <= 0)
        return false;
    --m_nSellingItems;
    return true;
}

CThief::CThief(int id, MessageManager& messages)
    : CBaseEntity(id, messages), m_stateMachine(this)
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
    if (m_bCaught)
        return false;
    //훔침 메시지 보내기
    MsgSend(Merchant, MSG_ITEM_STOLEN);
    return true;
}

CGuard::CGuard(int id, MessageManager& messages)
    : CBaseEntity(id, messages), m_stateMachine(this)
{
    m_stateMachine.SetCurrentState(PATROLLING::Instance());
}

void CGuard::Update()
{
    m_stateMachine.Update();
}

bool CGuard::HandleMessage(const Telegram& telegram)
{
    return m_stateMachine.HandleMessage(telegram); 
}
