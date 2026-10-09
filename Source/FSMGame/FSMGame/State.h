#pragma once

struct Telegram;
class CMerchant;
class CThief;
class CGuard;

template<class Type>
class CState
{
public:
    virtual ~CState() = default;
    virtual void Enter(Type*) = 0;
    virtual void Execute(Type*) = 0;
    virtual void Exit(Type*) = 0;
    virtual bool OnMessage(Type*, const Telegram&) = 0;
};

// State objects have no per-character data; timers belong to their owners.
class SELLING : public CState<CMerchant>
{
public:
    static SELLING* Instance();
    void Enter(CMerchant* owner) override;
    void Execute(CMerchant* owner) override;
    void Exit(CMerchant* owner) override;
    bool OnMessage(CMerchant* owner, const Telegram& telegram) override;

private:
    SELLING() = default;
};
class CALLING_GUARD : public CState<CMerchant>
{
public:
    static CALLING_GUARD* Instance();
    void Enter(CMerchant* owner) override;
    void Execute(CMerchant* owner) override;
    void Exit(CMerchant* owner) override;
    bool OnMessage(CMerchant* owner, const Telegram& telegram) override;

private:
    CALLING_GUARD() = default;
};
class WAITING : public CState<CMerchant>
{
public:
    static WAITING* Instance();
    void Enter(CMerchant* owner) override;
    void Execute(CMerchant* owner) override;
    void Exit(CMerchant* owner) override;
    bool OnMessage(CMerchant* owner, const Telegram& telegram) override;

private:
    WAITING() = default;
};
class WANDERING : public CState<CThief>
{
public:
    static WANDERING* Instance();
    void Enter(CThief* owner) override;
    void Execute(CThief* owner) override;
    void Exit(CThief* owner) override;
    bool OnMessage(CThief* owner, const Telegram& telegram) override;

private:
    WANDERING() = default;
};
class STEALING : public CState<CThief>
{
public:
    static STEALING* Instance();
    void Enter(CThief* owner) override;
    void Execute(CThief* owner) override;
    void Exit(CThief* owner) override;
    bool OnMessage(CThief* owner, const Telegram& telegram) override;

private:
    STEALING() = default;
};
class ESCAPING : public CState<CThief>
{
public:
    static ESCAPING* Instance();
    void Enter(CThief* owner) override;
    void Execute(CThief* owner) override;
    void Exit(CThief* owner) override;
    bool OnMessage(CThief* owner, const Telegram& telegram) override;

private:
    ESCAPING() = default;
};
class THIEF_GLOBAL_STATE : public CState<CThief>
{
public:
    static THIEF_GLOBAL_STATE* Instance();
    void Enter(CThief* owner) override;
    void Execute(CThief* owner) override;
    void Exit(CThief* owner) override;
    bool OnMessage(CThief* owner, const Telegram& telegram) override;

private:
    THIEF_GLOBAL_STATE() = default;
};
class PATROLLING : public CState<CGuard>
{
public:
    static PATROLLING* Instance();
    void Enter(CGuard* owner) override;
    void Execute(CGuard* owner) override;
    void Exit(CGuard* owner) override;
    bool OnMessage(CGuard* owner, const Telegram& telegram) override;

private:
    PATROLLING() = default;
};
class CHASING : public CState<CGuard>
{
public:
    static CHASING* Instance();
    void Enter(CGuard* owner) override;
    void Execute(CGuard* owner) override;
    void Exit(CGuard* owner) override;
    bool OnMessage(CGuard* owner, const Telegram& telegram) override;

private:
    CHASING() = default;
};
class RETURNING : public CState<CGuard>
{
public:
    static RETURNING* Instance();
    void Enter(CGuard* owner) override;
    void Execute(CGuard* owner) override;
    void Exit(CGuard* owner) override;
    bool OnMessage(CGuard* owner, const Telegram& telegram) override;

private:
    RETURNING() = default;
};
