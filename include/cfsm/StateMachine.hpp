#pragma once

namespace cfsm {

template <class StateType>
class StateMachine {
    template <class U, template <U, U> class, U> friend class StateChanger;

public:
    StateMachine() = delete;
    StateMachine(StateMachine const&) = delete;

    explicit StateMachine(StateType const initialState)
        : state_(initialState) { }

    virtual StateType getState() const {
        return state_;
    }

protected:
    virtual ~StateMachine() = default;

private:
    void setState(StateType const state) {
        state_ = state;
    }

    StateType state_;
};

} // namespace cfsm

