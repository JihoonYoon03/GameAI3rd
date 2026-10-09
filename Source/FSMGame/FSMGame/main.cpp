#include "common.h"
#include "Entity.h"
#include "GameConfig.h"
#include "MessageManager.h"

namespace
{
    bool IsGameOver(const CMerchant& merchant, const CThief& thief)
    {
        return merchant.GetSellingItems() == 0 || thief.IsCaught();
    }

    bool Tick(unsigned int tickNumber, const std::array<CBaseEntity*, 3>& entities,
        MessageManager& messages, const CMerchant& merchant, const CThief& thief)
    {
        std::cout << "\n[Tick " << tickNumber << "]\n";
        messages.Update(tickNumber * (GameConfig::TickIntervalMs / 1000.0));

        // Each active character executes once. Stop immediately at an ending.
        for (CBaseEntity* entity : entities)
        {
            if (IsGameOver(merchant, thief))
                break;
            entity->Update();
        }

        // Messages and resulting Enter/Exit calls are also processed inside Tick.
        // State Execute calls never nest: a newly entered state acts next Tick.
        messages.DispatchPendingMessages();
        std::cout << std::flush;
        return !IsGameOver(merchant, thief);
    }

    void PrintResult(const CMerchant& merchant, const CThief& thief)
    {
        std::cout << "\n========== 게임 결과 ==========\n";
        std::cout << "종료 원인: "
            << (thief.IsCaught() ? "경비병이 도둑을 잡았습니다." : "상인의 물건이 모두 없어졌습니다.")
            << '\n';
        std::cout << "상인에게 남은 물건: " << merchant.GetSellingItems() << "개\n";
        std::cout << "도둑이 훔친 물건: " << thief.GetStolenItems() << "개\n";
        std::cout << "===============================\n";
    }
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(949);
#endif
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    MessageManager messages;
    CMerchant merchant(Merchant, messages);
    CThief thief(Thief, messages, merchant);
    CGuard guard(Guard, messages, thief);
    const std::array<CBaseEntity*, 3> entities = { &merchant, &thief, &guard };
    for (CBaseEntity* entity : entities)
    {
        messages.RegisterEntity(*entity);
    }

    std::cout << "FSM With Messaging - 상인, 도둑, 경비병\n";
    std::cout << "초기 물건: " << GameConfig::InitialItems << "개 / Tick 간격: "
        << GameConfig::TickIntervalMs << "ms\n";

    unsigned int tickNumber = 0;
    while (true)
    {
        Sleep(GameConfig::TickIntervalMs);
        if (!Tick(++tickNumber, entities, messages, merchant, thief))
            break;
    }

    PrintResult(merchant, thief);
    for (CBaseEntity* entity : entities)
    {
        messages.RemoveEntity(entity->ID());
    }
    return 0;
}
