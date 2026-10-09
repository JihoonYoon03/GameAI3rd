#pragma once

// Durations are measured in Tick calls; probabilities are percentages.
namespace GameConfig
{
    inline constexpr int TickIntervalMs = 100;
    inline constexpr int InitialItems = 10;
    inline constexpr int WanderTicks = 3;
    inline constexpr int ChaseTicks = 4;
    inline constexpr int ReturnTicks = 2;
    inline constexpr int NoticeTheftPercent = 50;
    inline constexpr int CatchThiefPercent = 20;

    static_assert(TickIntervalMs > 0 && InitialItems > 0);
    static_assert(WanderTicks > 0 && ChaseTicks > 0 && ReturnTicks > 0);
    static_assert(NoticeTheftPercent >= 0 && NoticeTheftPercent <= 100);
    static_assert(CatchThiefPercent >= 0 && CatchThiefPercent <= 100);
}
