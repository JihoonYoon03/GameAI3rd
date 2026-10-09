#pragma once

#include "GameConfig.h"

// 세 인물이 재고, 체포 여부와 경비병 도움 요청을 공유하는 공간이다.
class Blackboard
{
public:
    int GetMerchantItems() const { return m_merchantItems; }
    unsigned int GetStolenItems() const { return m_stolenItems; }
    bool IsThiefCaught() const { return m_thiefCaught; }
    bool IsGuardRequested() const { return m_guardRequested; }

    bool StealOneItem();
    void CatchThief() { m_thiefCaught = true; }
    void CompleteGuardReturn() { m_guardReturnPending = true; }
    void CommitTick();

private:
    int m_merchantItems = GameConfig::InitialItems;
    unsigned int m_stolenItems = 0;
    bool m_thiefCaught = false;

    // 도난을 발견한 시점부터 경비병의 복귀 완료까지 유지되는 도움 요청이다.
    bool m_guardRequested = false;
    bool m_theftPending = false;
    bool m_guardReturnPending = false;
};
