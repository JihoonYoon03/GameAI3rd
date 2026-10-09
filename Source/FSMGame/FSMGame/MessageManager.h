#pragma once

#include "Telegram.h"

class CBaseEntity;

class MessageManager
{
public:
    void RegisterEntity(CBaseEntity& entity);
    void RemoveEntity(int id);
    CBaseEntity* GetEntity(int id) const;

    // Even zero-delay messages are queued, so Enter/Exit cannot be re-entered.
    bool DispatchMessage(int sender, int receiver, int message,
        double delaySeconds = 0.0, void* extraInfo = nullptr);
    void Update(double currentTimeSeconds);
    void DispatchPendingMessages();

private:
    std::map<int, CBaseEntity*> m_entities;
    // A multimap preserves messages with equal delivery times in send order.
    std::multimap<double, Telegram> m_pendingMessages;
    double m_currentTime = 0.0;
};
