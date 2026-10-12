#include "common.h"
#include "Entity.h"
#include "GameConfig.h"
#include "Blackboard.h"

namespace
{
    bool IsGameOver(const CMerchant& merchant, const CThief& thief)
    {
        return merchant.GetSellingItems() == 0 || thief.IsCaught();
    }

    bool Tick(unsigned int tickNumber, const std::array<CBaseEntity*, 3>& entities,
        Blackboard& blackboard, CMerchant& merchant, const CThief& thief)
    {
        std::cout << "\n[Tick " << tickNumber << "]\n";

        // Each active character executes once. Stop immediately at an ending.
        for (CBaseEntity* entity : entities)
        {
            if (IsGameOver(merchant, thief))
                break;
            entity->Update();
        }

        // Publish observations after all characters have acted this Tick.
        // Trees react to the updated shared information on the next Tick.
        merchant.ObserveTheft();
        blackboard.CommitTick();
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
    // Match the Korean ANSI encoding used by the source and string literals.
    SetConsoleOutputCP(949);
#endif
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Blackboard blackboard;
    CMerchant merchant(blackboard);
    CThief thief(blackboard);
    CGuard guard(blackboard);
    const std::array<CBaseEntity*, 3> entities = { &merchant, &thief, &guard };

    std::cout << "Behavior Tree - 상인, 도둑, 경비병\n";
    std::cout << "초기 물건: " << GameConfig::InitialItems << "개 / Tick 간격: "
        << GameConfig::TickIntervalMs << "ms\n";

    const auto interval = std::chrono::milliseconds(GameConfig::TickIntervalMs);
    auto nextTick = std::chrono::steady_clock::now();
    unsigned int tickNumber = 0;
    while (true)
    {
        nextTick += interval;
        std::this_thread::sleep_until(nextTick);
        if (!Tick(++tickNumber, entities, blackboard, merchant, thief))
            break;
    }

    PrintResult(merchant, thief);
    return 0;
}
