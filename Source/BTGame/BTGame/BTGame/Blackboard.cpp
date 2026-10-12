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
    // 상인이 판단한 도움 요청을 다음 Tick에 읽을 수 있도록 반영한다.
    if (m_guardRequestPending)
        m_guardRequested = true;

    // 복귀 완료가 공유되면 다음 Tick부터 판매, 배회, 순찰을 재개한다.
    if (m_guardReturnPending)
        m_guardRequested = false;

    m_theftPending = false;
    m_guardRequestPending = false;
    m_guardReturnPending = false;
}
