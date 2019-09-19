#pragma once

#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "logging.hpp"
#include "try_set_initial_state.hpp"
#include "entry_policy.hpp"
#include "try_dispatch.hpp"


namespace ufsm
{
namespace back
{
namespace detail
{
template<typename State, typename = void_t<>, typename... Args>
struct HasEntryT : std::false_type { };

template<typename State, typename... Args>
using state_entry_call = decltype(std::declval<State>().entry(std::declval<Args>()...));

template<typename State, typename... Args>
struct HasEntryT<State,
                 void_t<state_entry_call<State, Args...>>,
                 Args...>
: std::true_type { };

template<typename State, typename... Args>
constexpr inline auto HasEntry{HasEntryT<State, void, Args...>::value};

struct NoEntryAction { };
struct EntryActionFsm { };
struct EntryActionFsmEvent { };

template<typename State, typename FsmT, typename = void_t<>>
struct SelectEntryActionFsmT { using type = NoEntryAction; };

template<typename State, typename FsmT>
struct SelectEntryActionFsmT<State, FsmT, void_t<state_entry_call<State, FsmT>>> {
    using type = EntryActionFsm;
};

template<typename State, typename FsmT, typename Event, typename = void_t<>>
struct SelectEntryActionFsmEventT : SelectEntryActionFsmT<State, FsmT> { };

template<typename State, typename FsmT, typename Event>
struct SelectEntryActionFsmEventT<State, FsmT, Event, void_t<state_entry_call<State, FsmT, Event>>> {
    using type = EntryActionFsmEvent;
};

template<typename State, typename FsmT, typename Event>
using SelectEntryActionSignature = typename SelectEntryActionFsmEventT<State, FsmT, Event>::type;

template<typename State, bool = IsFsm<State>>
struct tryEnter;

template<typename State, typename = GetEntryPolicy<std::decay_t<State>>>
struct propagateEntry {
    template<typename Event>
    constexpr inline void operator()(State const&, Event&&) const noexcept {/* nop */}
};
} // namespace detail

// Intentionally do not decay the types here - HasEntry should decide if FsmT has an entry()
// member callable with the given state including the qualifiers
template<typename State_, typename FsmT_, typename Event_,
         typename = detail::SelectEntryActionSignature<State_, FsmT_, Event_>>
struct fsmEntry {
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&&, Event&& event) const noexcept
    {
        detail::propagateEntry<std::decay_t<State>>{}(std::forward<State>(state), std::forward<Event>(event));
        detail::tryDispatch<State>{}(std::forward<State>(state), std::forward<Event>(event));
    }
};

template<typename State_, typename FsmT_, typename Event_>
struct fsmEntry<State_, FsmT_, Event_, detail::EntryActionFsm> {
    // template<typename State, typename FsmT, typename Event>
    // constexpr inline void operator()(FsmT&& fsm, State&& state) const noexcept
    // {
    //     logging::fsm_log_entry(fsm, detail::asBaseState(state));
    //     // TODO: How to reliably keep both std::forwards here?
    //     std::forward<State>(state).entry(std::forward<FsmT>(fsm));
    //     detail::propagateEntry<std::decay_t<State>>{}(std::forward<State>(state));
    // }
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&& fsm, Event&& event) const noexcept
    {
        // operator()(fsm, state);
        logging::fsm_log_entry(fsm, detail::asBaseState(state));
        // TODO: How to reliably keep both std::forwards here?
        std::forward<State>(state).entry(std::forward<FsmT>(fsm));
        detail::propagateEntry<std::decay_t<State>>{}(std::forward<State>(state), std::forward<Event>(event));
        detail::tryDispatch<State>{}(std::forward<State>(state), std::forward<Event>(event));
    }
};

template<typename State_, typename FsmT_, typename Event_>
struct fsmEntry<State_, FsmT_, Event_, detail::EntryActionFsmEvent> {
    // template<typename State, typename FsmT, typename Event>
    // constexpr inline void operator()(FsmT&& fsm, State&& state) const noexcept
    // {
    //     logging::fsm_log_entry(fsm, detail::asBaseState(state));
    //     // TODO: How to reliably keep both std::forwards here?
    //     std::forward<State>(state).entry(std::forward<FsmT>(fsm));
    //     detail::propagateEntry<std::decay_t<State>>{}(std::forward<State>(state));
    // }
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&& fsm, Event&& event) const noexcept
    {
        // operator()(fsm, state);
        logging::fsm_log_entry(fsm, detail::asBaseState(state));
        // TODO: How to reliably keep both std::forwards here?
        std::forward<State>(state).entry(std::forward<FsmT>(fsm), std::forward<Event>(event));
        detail::propagateEntry<std::decay_t<State>>{}(std::forward<State>(state), std::forward<Event>(event));
        detail::tryDispatch<State>{}(std::forward<State>(state), std::forward<Event>(event));
    }
};

template<typename State, typename FsmT, typename Event>
constexpr inline void fsm_entry(State&& state, FsmT&& fsm, Event&& event) noexcept
{
    // using fsm_t = std::decay_t<FsmT>;
    // using state_t = std::decay_t<State>;
    // Intentionally do not decay the types here - HasEntry should decide if FsmT has an entry()
    // member callable with the given state including the qualifiers
    fsmEntry<State, FsmT, Event>{}(
        std::forward<State>(state), std::forward<FsmT>(fsm), std::forward<Event>(event)
    );
}

namespace detail
{
// TODO: optimization - narrow down Indices to only those referring to states which actually
// do implement an entry-action
template<typename Indices> struct enterCurrentState;
template<>
struct enterCurrentState<IndexSequence<>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT const&, Event const&) const noexcept { }
};

template<size_type I, size_type... Is>
struct enterCurrentState<IndexSequence<I, Is...>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept {
        if (I == fsm.state()) {
            // fsm_entry(std::forward<FsmT>(fsm), Get<I>(std::forward<FsmT>(fsm)));
            fsmEntry<StateAt<I, FsmT>, FsmT, Event>{}(
                Get<I>(std::forward<FsmT>(fsm)),
                std::forward<FsmT>(fsm),
                std::forward<Event>(event)
            );
            // As a potential minor optimization dispatch the event directly to the state `I`
            // rather than calling .dispatch_event() on the state, which will make it necessary
            // to iterate again at runtime
            // return;

        }
        else {
            enterCurrentState<IndexSequence<Is...>>{}(
                std::forward<FsmT>(fsm), std::forward<Event>(event)
            );
        }
    }
};

template<typename State_, bool>
struct tryEnter {
    template<typename State, typename Event>
    constexpr inline void operator()(State&&, Event&&) const noexcept {/* nop */}
};

template<typename State>
struct tryEnter<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept {
        enterCurrentState<typename std::decay_t<FsmT>::Indices>{}(
            std::forward<FsmT>(fsm), std::forward<Event>(event)
        );
    }
};

template<typename State>
struct propagateEntry<State, InitialStateEntryPolicy> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept {
        detail::trySetInitialState<std::decay_t<FsmT>>{}(std::forward<FsmT>(fsm));
        // The optimization would also require an event dispatch directly to the InitialState here
    }
};

template<typename State>
struct propagateEntry<State, CurrentStateEntryPolicy> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        detail::tryEnter<std::decay_t<FsmT>>{}(
            std::forward<FsmT>(fsm), std::forward<Event>(event)
        );
    }
};
} // namespace detail


} // namespace back
} // namespace ufsm
