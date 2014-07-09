#pragma once

#include "StateMachine.hpp"

namespace cfsm {

template <class StateType>
class StateMachine;

template <class StateType, template <StateType, StateType> class TransitionType, StateType From>
class StateChanger {
public:
    template <StateType To>
    static void changeState(StateMachine<StateType> & stateMachine) {
        transitionValidityGuard<To>();
        stateMachine.setState(To);
    }

private:
    template <StateType To>
    static void transitionValidityGuard(TransitionType<From, To> const& = {}) { }
};

} // namespace cfsm

