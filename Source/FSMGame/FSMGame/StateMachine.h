#pragma once

#include "State.h"

template<class Type>
class CStateMachine
{
public:
    explicit CStateMachine(Type* owner) : m_pOwner(owner)
    {
        if (!owner)
            throw std::invalid_argument("A state machine needs an owner.");
    }
    ~CStateMachine() = default;
    CStateMachine(const CStateMachine&) = delete;
    CStateMachine& operator=(const CStateMachine&) = delete;

    void Update()
    {
        if (m_pGlobalState)
            m_pGlobalState->Execute(m_pOwner);
        if (m_pCurrentState)
            m_pCurrentState->Execute(m_pOwner);
    }

    void ChangeState(CState<Type>* newState)
    {
        if (!newState || newState == m_pCurrentState)
            return;

        m_pPreviousState = m_pCurrentState;
        if (m_pCurrentState)
            m_pCurrentState->Exit(m_pOwner);
        m_pCurrentState = newState;
        m_pCurrentState->Enter(m_pOwner);
    }

    bool HandleMessage(const Telegram& telegram)
    {
        if (m_pCurrentState && m_pCurrentState->OnMessage(m_pOwner, telegram))
            return true;
        return m_pGlobalState && m_pGlobalState->OnMessage(m_pOwner, telegram);
    }

    void RevertToPreviousState() { ChangeState(m_pPreviousState); }
    void SetCurrentState(CState<Type>* state) { ChangeState(state); }
    void SetPreviousState(CState<Type>* state) { m_pPreviousState = state; }
    void SetGlobalState(CState<Type>* state)
    {
        if (state == m_pGlobalState)
            return;
        if (m_pGlobalState)
            m_pGlobalState->Exit(m_pOwner);
        m_pGlobalState = state;
        if (m_pGlobalState)
            m_pGlobalState->Enter(m_pOwner);
    }

    CState<Type>* CurrentState() const { return m_pCurrentState; }
    CState<Type>* PreviousState() const { return m_pPreviousState; }
    CState<Type>* GlobalState() const { return m_pGlobalState; }
    bool IsInState(const CState<Type>* state) const { return m_pCurrentState == state; }

private:
    Type* m_pOwner;
    // States are shared static instances; the state machine does not delete them.
    CState<Type>* m_pCurrentState = nullptr;
    CState<Type>* m_pPreviousState = nullptr;
    CState<Type>* m_pGlobalState = nullptr;
};
