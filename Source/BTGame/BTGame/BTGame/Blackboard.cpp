#include "common.h"
#include "Blackboard.h"

bool Blackboard::StealOneItem()
{
    if (m_thiefCaught || m_merchantItems <= 0)
        return false;

    --m_merchantItems;
    ++m_stolenItems;
    m_theftPending = true;
    return true;
}

void Blackboard::CommitTick()
{
    // 기존의 Tick 끝 메시지 처리와 같은 시점에 도난 인지 여부를 판정한다.
    // 추격 중 체포 판정이 먼저, 새 도난의 발견 판정이 나중인 순서도 유지한다.
    if (m_theftPending && m_merchantItems > 0 && !m_guardRequested)
    {
        if (std::rand() % 100 < GameConfig::NoticeTheftPercent)
            m_guardRequested = true;
    }

    // 복귀 완료가 공유되면 다음 Tick부터 판매, 배회, 순찰을 재개한다.
    if (m_guardReturnPending)
        m_guardRequested = false;

    m_theftPending = false;
    m_guardReturnPending = false;
}
