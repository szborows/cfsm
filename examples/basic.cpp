#include <cfsm/StateChanger.hpp>
#include <cfsm/StateMachine.hpp>
#include <cfsm/Transition.hpp>

#include <array>
#include <memory>
#include <type_traits>

using namespace cfsm;

/// Possible states, with terminating one.
enum class MyState {
    First,
    Second,
    Invalid
};

/// State machine class.
class MyStateMachine
    : public StateMachine<MyState>
{
public:
    MyStateMachine(MyState const initialState)
        : StateMachine<MyState>(initialState)
    {}
};

template <MyState From, MyState To>
class MyTransition : private Transition<MyState, From, To> { };

/// State changer, only this class is able to change state of a state machine.
template <MyState From>
class MyStateChanger : private StateChanger<MyState, MyTransition, From> {
public:
    template <MyState To>
    static void changeState(MyStateMachine & stateMachine) {
        StateChanger<MyState, MyTransition, From>::template changeState<To>(stateMachine);
    }
};

/// Possible transitions.
template <> class MyTransition<MyState::First, MyState::Second>{};
template <> class MyTransition<MyState::Second, MyState::First>{};

/// Example event.
struct MyEvent { };

/// Base class for all handlers.
struct IHandler {
    virtual void handleEvent(MyEvent const&, MyStateMachine &) const = 0;
};

/// Two event handlers, each for particular state machine state.
struct FirstStateHandler : public MyStateChanger<MyState::First>, IHandler {
    void handleEvent(MyEvent const&, MyStateMachine & stateMachine) const {
        // Stuff goes here...
        changeState<MyState::Second>(stateMachine);
    }
};

struct SecondStateHandler : public MyStateChanger<MyState::Second>, IHandler {
    void handleEvent(MyEvent const&, MyStateMachine & stateMachine) const {
        // and also here...
        changeState<MyState::First>(stateMachine);
    }
};

int main(void) {
    using namespace std;
    using StateEnumType = underlying_type<MyState>::type;

    array<unique_ptr<IHandler>, static_cast<StateEnumType>(MyState::Invalid)> handlers;
    handlers[static_cast<StateEnumType>(MyState::First)].reset(new FirstStateHandler);
    handlers[static_cast<StateEnumType>(MyState::Second)].reset(new SecondStateHandler);

    auto getHandler = [&handlers](MyState const state) -> IHandler const& {
        return *handlers[static_cast<StateEnumType>(state)].get();
    };

    auto receiveEvent = []() {
        return MyEvent{}; // Proper implementation should go here...
    };

    MyStateMachine myStateMachine(MyState::First);
    while (true) {
        getHandler(myStateMachine.getState()).handleEvent(receiveEvent(), myStateMachine);
    }
}

