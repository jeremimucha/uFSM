#pragma once

#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "logging.hpp"
#include "try_set_initial_state.hpp"
#include "entry_policy.hpp"


namespace ufsm
{
namespace back
{
namespace detail
{
template<typename State, typename = void_t<>, typename... Args>
struct HasEntryT : std::false_type { };

template<typename State, typename... Args>
struct HasEntryT<State,
                 void_t<decltype(std::declval<State>().entry(std::declval<Args>()...))>,
                 Args...>
: std::true_type { };

template<typename State, typename... Args>
constexpr inline auto HasEntry{HasEntryT<State, void, Args...>::value};

template<typename State, bool = IsFsm<State>>
struct tryEnter;

template<typename State, typename = GetEntryPolicy<std::decay_t<State>>>
struct propagateEntry {
    constexpr inline void operator()(State const&) const noexcept {/* nop */}
};
} // namespace detail

// Intentionally do not decay the types here - HasEntry should decide if FsmT has an entry()
// member callable with the given state including the qualifiers
template<typename FsmT_, typename State_,
         bool = detail::HasEntry<State_, FsmT_>>
struct fsmEntry {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&&, State&&) const noexcept { }
};

template<typename FsmT_, typename State_>
struct fsmEntry<FsmT_, State_, true> {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&& fsm, State&& state) const noexcept
    {
        logging::fsm_log_entry(fsm, state);
        // TODO: How to reliably keep both std::forwards here?
        std::forward<State>(state).entry(std::forward<FsmT>(fsm));
        detail::propagateEntry<std::decay_t<State>>{}(std::forward<State>(state));
    }
};

template<typename FsmT, typename State>
constexpr inline void fsm_entry(FsmT&& fsm, State&& state) noexcept
{
    // using fsm_t = std::decay_t<FsmT>;
    // using state_t = std::decay_t<State>;
    // Intentionally do not decay the types here - HasEntry should decide if FsmT has an entry()
    // member callable with the given state including the qualifiers
    fsmEntry<FsmT, State>{}(std::forward<FsmT>(fsm), std::forward<State>(state));
}

namespace detail
{
// TODO: optimization - narrow down Indices to only those referring to states which actually
// do implement an entry-action
template<typename Indices> struct enterCurrentState;
template<>
struct enterCurrentState<IndexSequence<>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&&) const noexcept { }
};

template<size_type I, size_type... Is>
struct enterCurrentState<IndexSequence<I, Is...>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept {
        if (I == fsm.state()) {
            fsm_entry(std::forward<FsmT>(fsm), Get<I>(std::forward<FsmT>(fsm)));
            return;
        }
        enterCurrentState<IndexSequence<Is...>>{}(std::forward<FsmT>(fsm));
    }
};

template<typename State, bool>
struct tryEnter {
    template<typename State_>
    constexpr inline void operator()(State_&&) const noexcept {/* nop */}
};

template<typename State>
struct tryEnter<State, true> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept {
        enterCurrentState<typename std::decay_t<FsmT>::Indices>{}(std::forward<FsmT>(fsm));
    }
};

template<typename State>
struct propagateEntry<State, InitialStateEntryPolicy> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept {
        detail::trySetInitialState<std::decay_t<FsmT>>{}(std::forward<FsmT>(fsm));
    }
};

template<typename State>
struct propagateEntry<State, CurrentStateEntryPolicy> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept {
        detail::tryEnter<std::decay_t<FsmT>>{}(std::forward<FsmT>(fsm));
    }
};
} // namespace detail


} // namespace back
} // namespace ufsm
