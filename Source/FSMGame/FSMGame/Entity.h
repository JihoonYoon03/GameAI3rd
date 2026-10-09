#pragma once

#include "StateMachine.h"
#include "Telegram.h"

class MessageManager;

enum EntityType
{
    Merchant,
    Thief,
    Guard
};

class CBaseEntity
{
public:
    CBaseEntity(int id, MessageManager& messages);
    virtual ~CBaseEntity() = default;
    CBaseEntity(const CBaseEntity&) = delete;
    CBaseEntity& operator=(const CBaseEntity&) = delete;

    virtual void Update() = 0;
    virtual bool HandleMessage(const Telegram& telegram) = 0;
    int ID() const { return m_ID; }
    void SendMessage(int receiver, msgTypes message);
    void ResetStateTicks() { m_nStateTicks = 0; }
    int AdvanceStateTicks() { return ++m_nStateTicks; }

private:
    int m_ID = -1;
    static int m_iNextValidID;
    void SetID(int id);
    MessageManager& m_messages;
    int m_nStateTicks = 0;
};

class CMerchant : public CBaseEntity
{
public:
    CMerchant(int id, MessageManager& messages);
    void Update() override;
    bool HandleMessage(const Telegram& telegram) override;
    CStateMachine<CMerchant>& GetFSM() { return m_stateMachine; }
    int GetSellingItems() const { return m_nSellingItems; }
    bool TakeOneItem();

private:
    CStateMachine<CMerchant> m_stateMachine;
    int m_nSellingItems;
};

class CThief : public CBaseEntity
{
public:
    CThief(int id, MessageManager& messages, CMerchant& merchant);
    void Update() override;
    bool HandleMessage(const Telegram& telegram) override;
    CStateMachine<CThief>& GetFSM() { return m_stateMachine; }
    unsigned int GetStolenItems() const { return m_nStolenItems; }
    bool StealOneItem();
    bool IsCaught() const { return m_bCaught; }
    void SetCaught() { m_bCaught = true; }

private:
    CStateMachine<CThief> m_stateMachine;
    unsigned int m_nStolenItems = 0;
    bool m_bCaught = false;
    CMerchant& m_merchant;
};

class CGuard : public CBaseEntity
{
public:
    CGuard(int id, MessageManager& messages, CThief& thief);
    void Update() override;
    bool HandleMessage(const Telegram& telegram) override;
    CStateMachine<CGuard>& GetFSM() { return m_stateMachine; }
    void CatchThief() { m_thief.SetCaught(); }

private:
    CStateMachine<CGuard> m_stateMachine;
    CThief& m_thief;
};
