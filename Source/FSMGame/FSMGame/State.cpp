#include "common.h"
#include "State.h"
#include "Entity.h"
#include "GameConfig.h"
#include "Telegram.h"

namespace
{
    bool Chance(int percent)
    {
        return std::rand() % 100 < percent;
    }
}

// Merchant states
SELLING* SELLING::Instance()
{
    static SELLING state;
    return &state;
}

void SELLING::Enter(CMerchant* owner) 
{ 
    owner->ResetStateTicks();
}

void SELLING::Exit(CMerchant*) 
{
}

void SELLING::Execute(CMerchant* owner)
{
    std::cout << "상인: 물건 사세요~ (남은 물건: " << owner->GetSellingItems() << "개)\n";
}

bool SELLING::OnMessage(CMerchant* owner, const Telegram& telegram)
{
    if (telegram.Msg != MSG_ITEM_STOLEN || telegram.Sender != Thief)
        return false;

    // Stock was already transferred, even if the merchant does not notice it.
    if (owner->GetSellingItems() > 0 && Chance(GameConfig::NoticeTheftPercent))
        owner->GetFSM().ChangeState(CALLING_GUARD::Instance());
    return true;
}

CALLING_GUARD* CALLING_GUARD::Instance()
{
    static CALLING_GUARD state;
    return &state;
}

void CALLING_GUARD::Enter(CMerchant* owner)
{
    owner->ResetStateTicks();
    owner->SendMessage(Guard, MSG_THIEF_SPOTTED);
}

void CALLING_GUARD::Exit(CMerchant*)
{
}

void CALLING_GUARD::Execute(CMerchant* owner)
{
    std::cout << "상인: 도둑이야! 경비병님, 도와주세요!\n";
    owner->GetFSM().ChangeState(WAITING::Instance());
}

bool CALLING_GUARD::OnMessage(CMerchant* owner, const Telegram& telegram)
{
    if (telegram.Msg == MSG_GUARD_RETURNING && telegram.Sender == Guard)
    {
        owner->GetFSM().ChangeState(SELLING::Instance());
        return true;
    }
    return false;
}

WAITING* WAITING::Instance()
{
    static WAITING state;
    return &state;
}

void WAITING::Enter(CMerchant* owner) 
{
    owner->ResetStateTicks();
}

void WAITING::Exit(CMerchant*)
{
}

void WAITING::Execute(CMerchant*)
{
    std::cout << "상인: 경비병이 돌아오기를 기다리는 중...\n";
}

bool WAITING::OnMessage(CMerchant* owner, const Telegram& telegram)
{
    if (telegram.Msg == MSG_GUARD_RETURNING && telegram.Sender == Guard)
    {
        owner->GetFSM().ChangeState(SELLING::Instance());
        return true;
    }
    return false;
}

// Thief states
WANDERING* WANDERING::Instance()
{
    static WANDERING state;
    return &state;
}

void WANDERING::Enter(CThief* owner) 
{
    owner->ResetStateTicks();
}

void WANDERING::Exit(CThief*)
{
}

void WANDERING::Execute(CThief* owner)
{
    std::cout << "도둑: 훔칠만한 물건 탐색 중...\n";
    if (owner->AdvanceStateTicks() >= GameConfig::WanderTicks)
        owner->GetFSM().ChangeState(STEALING::Instance());
}
bool WANDERING::OnMessage(CThief*, const Telegram&) { return false; }

STEALING* STEALING::Instance()
{
    static STEALING state;
    return &state;
}

void STEALING::Enter(CThief* owner)
{ 
    owner->ResetStateTicks(); 
}

void STEALING::Exit(CThief*) 
{
}

void STEALING::Execute(CThief* owner)
{
    if (owner->StealOneItem())
        std::cout << "도둑: 물건 하나를 훔쳤다! (훔친 물건: " << owner->GetStolenItems() << "개)\n";
    else
        std::cout << "도둑: 더 이상 훔칠 물건이 없다...\n";

    owner->GetFSM().ChangeState(WANDERING::Instance());
}

bool STEALING::OnMessage(CThief*, const Telegram&)
{ 
    return false; 
}

ESCAPING* ESCAPING::Instance()
{
    static ESCAPING state;
    return &state;
}

void ESCAPING::Enter(CThief* owner)
{
    owner->ResetStateTicks();
}

void ESCAPING::Exit(CThief*)
{
}

void ESCAPING::Execute(CThief*)
{
    std::cout << "도둑: 경비병을 피해 도망치는 중...\n";
}
bool ESCAPING::OnMessage(CThief*, const Telegram&) { return false; }

THIEF_GLOBAL_STATE* THIEF_GLOBAL_STATE::Instance()
{
    static THIEF_GLOBAL_STATE state;
    return &state;
}

void THIEF_GLOBAL_STATE::Enter(CThief*)
{
}

void THIEF_GLOBAL_STATE::Execute(CThief*)
{
}

void THIEF_GLOBAL_STATE::Exit(CThief*) 
{
}

bool THIEF_GLOBAL_STATE::OnMessage(CThief* owner, const Telegram& telegram)
{
    // A chase notification interrupts both wandering and stealing.
    if (telegram.Sender != Guard)
        return false;
    if (telegram.Msg == MSG_GUARD_CHASING)
    {
        owner->GetFSM().ChangeState(ESCAPING::Instance());
        return true;
    }
    if (telegram.Msg == MSG_GUARD_RETURNING)
    {
        if (owner->GetFSM().IsInState(ESCAPING::Instance()))
            owner->GetFSM().ChangeState(WANDERING::Instance());
        return true;
    }
    return false;
}

// Guard states
PATROLLING* PATROLLING::Instance()
{
    static PATROLLING state;
    return &state;
}

void PATROLLING::Enter(CGuard* owner)
{
    owner->ResetStateTicks();
}

void PATROLLING::Exit(CGuard*) 
{
}

void PATROLLING::Execute(CGuard*)
{
    std::cout << "경비병: 순찰 중...\n";
}

bool PATROLLING::OnMessage(CGuard* owner, const Telegram& telegram)
{
    if (telegram.Msg == MSG_THIEF_SPOTTED && telegram.Sender == Merchant)
    {
        owner->GetFSM().ChangeState(CHASING::Instance());
        return true;
    }
    return false;
}

CHASING* CHASING::Instance()
{
    static CHASING state;
    return &state;
}

void CHASING::Enter(CGuard* owner)
{
    owner->ResetStateTicks();
    owner->SendMessage(Thief, MSG_GUARD_CHASING);
}

void CHASING::Exit(CGuard*) 
{
}

void CHASING::Execute(CGuard* owner)
{
    const int chaseTicks = owner->AdvanceStateTicks();
    if (Chance(GameConfig::CatchThiefPercent))
    {
        std::cout << "경비병: 도둑을 잡았다!\n";
        owner->CatchThief();
        owner->GetFSM().ChangeState(RETURNING::Instance());
    }
    else if (chaseTicks >= GameConfig::ChaseTicks)
    {
        std::cout << "경비병: 도둑을 놓쳤다. 추격을 중단하고 복귀한다...\n";
        owner->GetFSM().ChangeState(RETURNING::Instance());
    }
    else
    {
        std::cout << "경비병: 도둑을 추격 중... (" << chaseTicks << "/"
            << GameConfig::ChaseTicks << ")\n";
    }
}

bool CHASING::OnMessage(CGuard*, const Telegram& telegram)
{
    // Repeated reports must not restart the chase timer.
    return telegram.Msg == MSG_THIEF_SPOTTED && telegram.Sender == Merchant;
}

RETURNING* RETURNING::Instance()
{
    static RETURNING state;
    return &state;
}

void RETURNING::Enter(CGuard* owner)
{
    owner->ResetStateTicks(); 
}

void RETURNING::Exit(CGuard*)
{
}

void RETURNING::Execute(CGuard* owner)
{
    if (owner->AdvanceStateTicks() >= GameConfig::ReturnTicks)
    {
        std::cout << "경비병: 복귀 완료. 다시 순찰을 시작한다.\n";
        owner->GetFSM().ChangeState(PATROLLING::Instance());
        owner->SendMessage(Thief, MSG_GUARD_RETURNING);
        // Also release the merchant from WAITING when the guard resumes patrol.
        owner->SendMessage(Merchant, MSG_GUARD_RETURNING);
    }
    else
    {
        std::cout << "경비병: 순찰 장소로 복귀 중...\n";
    }
}

bool RETURNING::OnMessage(CGuard*, const Telegram& telegram)
{
    return telegram.Msg == MSG_THIEF_SPOTTED && telegram.Sender == Merchant;
}
