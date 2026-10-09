#include "common.h"
#include "MessageManager.h"
#include "Entity.h"

void MessageManager::RegisterEntity(CBaseEntity& entity)
{
    if (!m_entities.emplace(entity.ID(), &entity).second)
        throw std::logic_error("Entity ID is already registered.");
}

void MessageManager::RemoveEntity(int id)
{
    m_entities.erase(id);
    for (auto it = m_pendingMessages.begin(); it != m_pendingMessages.end();)
    {
        if (it->second.Sender == id || it->second.Receiver == id)
            it = m_pendingMessages.erase(it);
        else
            ++it;
    }
}

CBaseEntity* MessageManager::GetEntity(int id) const
{
    const auto it = m_entities.find(id);
    return it == m_entities.end() ? nullptr : it->second;
}

bool MessageManager::DispatchMessage(int sender, int receiver, int message,
    double delaySeconds, void* extraInfo)
{
    if (!GetEntity(sender) || !GetEntity(receiver))
        return false;

    const double deliveryTime = m_currentTime + (delaySeconds > 0.0 ? delaySeconds : 0.0);
    m_pendingMessages.emplace(deliveryTime,
        Telegram(deliveryTime, sender, receiver, message, extraInfo));
    return true;
}

void MessageManager::Update(double currentTimeSeconds)
{
    m_currentTime = currentTimeSeconds;
    DispatchPendingMessages();
}

void MessageManager::DispatchPendingMessages()
{
    while (!m_pendingMessages.empty() && m_pendingMessages.begin()->first <= m_currentTime)
    {
        const Telegram telegram = m_pendingMessages.begin()->second;
        m_pendingMessages.erase(m_pendingMessages.begin());
        if (CBaseEntity* receiver = GetEntity(telegram.Receiver))
            receiver->HandleMessage(telegram);
    }
}
