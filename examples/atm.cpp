// WARNING:
// This is just an example how cfsm can be utilized. It is neither complete nor correct.
// It may contain errors, at it was written without unit tests and during late hours.

#include <cfsm/StateChanger.hpp>
#include <cfsm/StateMachine.hpp>
#include <cfsm/Transition.hpp>

#include <array>
#include <cmath>
#include <memory>
#include <type_traits>
#include <vector>

using namespace cfsm;
using namespace std;

enum class State {
    Welcome,
    SelectLanguage,
    CardError,
    EnterPin,
    PinError,
    SelectAmount,
    AmountError,
    PrintConfirmation,
    Invalid
};

struct AtmStateMachine : public StateMachine<State> {
    AtmStateMachine() : StateMachine<State>(State::Welcome) { }

    vector<unsigned char> pin;
    unsigned getPin() const {
        unsigned result = 0;
        for (int i = pin.size(), j = 0; i > 0; --i, ++j) {
            result += pin[i] * pow(10, j);
        }
        return result;
    }
};

template <State From, State To> struct AtmTransition : private Transition<State, From, To> { };

template <State From>
struct AtmStateChanger : private StateChanger<State, AtmTransition, From> {
    template <State To>
    static void changeState(AtmStateMachine & atm) {
        StateChanger<State, AtmTransition, From>::template changeState<To>(atm);
    }
};

template <> struct AtmTransition<State::Welcome, State::SelectLanguage> { };
template <> struct AtmTransition<State::Welcome, State::CardError> { };
template <> struct AtmTransition<State::CardError, State::Welcome> { };
template <> struct AtmTransition<State::SelectLanguage, State::EnterPin> { };
template <> struct AtmTransition<State::EnterPin, State::PinError> { };
template <> struct AtmTransition<State::EnterPin, State::Welcome> { };
template <> struct AtmTransition<State::PinError, State::Welcome> { };
template <> struct AtmTransition<State::EnterPin, State::SelectAmount> { };
template <> struct AtmTransition<State::SelectAmount, State::AmountError> { };
template <> struct AtmTransition<State::SelectAmount, State::PrintConfirmation> { };
template <> struct AtmTransition<State::SelectAmount, State::Welcome> { };
template <> struct AtmTransition<State::AmountError, State::Welcome> { };
template <> struct AtmTransition<State::PrintConfirmation, State::Welcome> { };

enum class Key {
    N1, N2, N3, N4, N5, N6, N7, N8, N9, N0,
    A, B, C, D, E, F, G, H,
    Confirm, Backspace, Abort,
    Invalid
};

struct Time { };

enum class Language { English, German, Polish };

enum class Amount { B20, B50, B100, B200, Invalid };

struct KeypadEvent {
    Time time;
    Key code;
};

struct CardEvent {
    Time time;
    unsigned char const* const blockData;
};

struct Display {
    void showWelcomeScreen() { }
    void showLanguageSelectionScreen() { }
    void showCardErrorScreen() { }
    void showPinScreen() { }
    void showAmountScreen() { }
    void showPinErrorScreen() { }
    void changeLanguage(Language const) { }
};

struct CardReader {
    void returnCard() { }
    bool verifyPin(unsigned) { return true; }
};

struct IHandler {
    IHandler(Display & display, CardReader & cardReader)
        : display_(display)
        , cardReader_(cardReader){ }

    virtual void onCardEvent(CardEvent const&, AtmStateMachine &) const { };
    virtual void onKeypadEvent(KeypadEvent const&, AtmStateMachine &) const { };

protected:
    Display & display_;
    CardReader & cardReader_;
};

struct WelcomeStateHandler : AtmStateChanger<State::Welcome>, IHandler {
    WelcomeStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onCardEvent(CardEvent const& event, AtmStateMachine & atm) const {
        if (cardDataValid(event.blockData)) {
            display_.showLanguageSelectionScreen();
            changeState<State::SelectLanguage>(atm);
        }
        else {
            display_.showCardErrorScreen();
            changeState<State::CardError>(atm);
        }
    }

private:
    bool cardDataValid(unsigned char const* const) const {
        return true; // Real implementation goes here.
    }
};

struct SelectLanguageStateHandler : AtmStateChanger<State::SelectLanguage>, IHandler {
    SelectLanguageStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onKeypadEvent(KeypadEvent const& event, AtmStateMachine & atm) const override {
        switch (event.code) {
        case Key::E:
            display_.changeLanguage(Language::English);
            display_.showPinScreen();
            changeState<State::EnterPin>(atm);
            break;

        case Key::F:
            display_.changeLanguage(Language::German);
            display_.showPinScreen();
            changeState<State::EnterPin>(atm);
            break;

        case Key::G:
            display_.changeLanguage(Language::Polish);
            display_.showPinScreen();
            changeState<State::EnterPin>(atm);
            break;

        default:
            break;
        }
    }
};

struct CardErrorStateHandler : AtmStateChanger<State::CardError>, IHandler {
    CardErrorStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onKeypadEvent(KeypadEvent const&, AtmStateMachine & atm) const override {
        cardReader_.returnCard();
        changeState<State::Welcome>(atm);
    }
};

struct EnterPinStateHandler : AtmStateChanger<State::EnterPin>, IHandler {
    EnterPinStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onKeypadEvent(KeypadEvent const& event, AtmStateMachine & atm) const override {
        switch (event.code) {
        case Key::N0: case Key::N1: case Key::N2: case Key::N3: case Key::N4:
        case Key::N5: case Key::N7: case Key::N8: case Key::N9:
            atm.pin.push_back(static_cast<unsigned char>(event.code) - static_cast<unsigned char>(Key::N0));
            break;

        case Key::Backspace:
            if (!atm.pin.empty()) {
                atm.pin.pop_back();
            }
            break;

        case Key::Abort:
            display_.showWelcomeScreen();
            cardReader_.returnCard();
            changeState<State::Welcome>(atm);
            break;

        case Key::Confirm:
            if (cardReader_.verifyPin(atm.getPin())) {
                display_.showAmountScreen();
                changeState<State::SelectAmount>(atm);
            }
            else {
                display_.showPinErrorScreen();
                changeState<State::PinError>(atm);
            }
            atm.pin.clear();
            break;

        default:
            break;
        }
    }
};

struct PinErrorStateHandler : AtmStateChanger<State::PinError>, IHandler {
    PinErrorStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onKeypadEvent(KeypadEvent const&, AtmStateMachine & atm) const override {
        cardReader_.returnCard();
        changeState<State::Welcome>(atm);
    }
};

struct SelectAmountStateHandler : AtmStateChanger<State::SelectAmount>, IHandler {
    SelectAmountStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onKeypadEvent(KeypadEvent const& event, AtmStateMachine & atm) const override {
        Amount amount = Amount::Invalid;
        switch (event.code) {
        case Key::A:
            amount = Amount::B20;
            break;

        case Key::B:
            amount = Amount::B50;
            break;

        case Key::C:
            amount = Amount::B100;
            break;

        case Key::D:
            amount = Amount::B200;
            break;

        case Key::Abort:
            display_.showWelcomeScreen();
            cardReader_.returnCard();
            changeState<State::Welcome>(atm);
            break;

        default:
            break;
        }

        if (Amount::Invalid != amount) {
            // Withdraw money...
            changeState<State::PrintConfirmation>(atm);
        }
    }
};

struct AmountErrorStateHandler : AtmStateChanger<State::AmountError>, IHandler {
    AmountErrorStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onKeypadEvent(KeypadEvent const&, AtmStateMachine & atm) const override {
        cardReader_.returnCard();
        changeState<State::Welcome>(atm);
    }
};

struct PrintConfirmationStateHandler : AtmStateChanger<State::PrintConfirmation>, IHandler {
    PrintConfirmationStateHandler(Display & display, CardReader & cardReader)
        : IHandler(display, cardReader) { }

    void onKeypadEvent(KeypadEvent const&, AtmStateMachine & atm) const override {
        // Print confirmation...

        cardReader_.returnCard();
        changeState<State::Welcome>(atm);
    }
};

int main(void) {
    using StateEnumType = underlying_type<State>::type;

    array<unique_ptr<IHandler>, static_cast<StateEnumType>(State::Invalid)> handlers;
    auto registerHandler = [&handlers](State const state, IHandler * const handler) {
        handlers[static_cast<StateEnumType>(state)].reset(handler);
    };

    Display display;
    CardReader cardReader;

    registerHandler(State::Welcome, new WelcomeStateHandler{display, cardReader});
    registerHandler(State::SelectLanguage, new SelectLanguageStateHandler{display, cardReader});
    registerHandler(State::CardError, new CardErrorStateHandler{display, cardReader});
    registerHandler(State::EnterPin, new EnterPinStateHandler{display, cardReader});
    registerHandler(State::PinError, new PinErrorStateHandler{display, cardReader});
    registerHandler(State::SelectAmount, new SelectAmountStateHandler{display, cardReader});
    registerHandler(State::AmountError, new AmountErrorStateHandler{display, cardReader});
    registerHandler(State::PrintConfirmation, new PrintConfirmationStateHandler{display, cardReader});
}

