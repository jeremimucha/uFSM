#pragma once

#include "traits.hpp"
#include <type_traits>
// TODO: Decide if it's worth passing events down the call chain just to have additional logging
// information. As is, events are not passed down to `transition` from `dispatch event`.
// If it turns out useful for guards and actions to take an `event` parameter it might be worth it.
// Alternatively a dummy type<Event>{} could be passed to the logging functions, just to add the
// type information without providing access to the actual instance.


namespace ufsm
{
namespace logging
{

namespace detail
{

template<typename T, typename = void_t<>>
struct has_logger : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename T>
struct has_logger<T, void_t<decltype(T::logger())>> : std::true_type { };
#endif
template<typename T>
constexpr inline auto has_logger_v{has_logger<T>::value};

// log event
template<typename FsmT, typename State, typename Event,
         typename = void_t<>>
struct has_log_event : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename FsmT, typename State, typename Event>
struct has_log_event<FsmT, State, Event,
    void_t<decltype(FsmT::logger().log_event(
        std::declval<FsmT const&>(), std::declval<State const&>(), std::declval<Event const&>()))>>
    : std::true_type
{
};
#endif
template<typename FsmT, typename State, typename Event>
constexpr inline auto has_log_event_v{has_log_event<FsmT, State, Event>::value};

// log guard - FsmT::logger().log_guard(fsm, guard, event, guard_result);
template<typename FsmT, typename Guard, /* typename Event, */
         typename = void_t<>>
struct has_log_guard : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename FsmT, typename Guard/* , typename Event */>
struct has_log_guard<FsmT, Guard, /* Event, */
    void_t<decltype(FsmT::logger().log_guard(
        std::declval<FsmT const&>(), std::declval<Guard const&>(),
        /* std::declval<Event const&>(), */ false))>>
    : std::true_type
{
};
#endif
template<typename FsmT, typename Guard/* , typename Event */>
constexpr inline auto has_log_guard_v{has_log_guard<FsmT, Guard/* , Event */>::value};

// log action
template<typename FsmT, typename Action, /* typename Event, */
         typename = void_t<>>
struct has_log_action : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename FsmT, typename Action/* , typename Event */>
struct has_log_action<FsmT, Action, /* Event, */
    void_t<decltype(FsmT::logger().log_action(
        std::declval<FsmT const&>(), std::declval<Action const&>()/* , std::declval<Event const&>() */))>>
    : std::true_type
{
};
#endif
template<typename FsmT, typename Action/* , typename Event */>
constexpr inline auto has_log_action_v{has_log_action<FsmT, Action/* , Event */>::value};

// log state change
template<typename FsmT, typename SrcState, typename DstState,
         typename = void_t<>>
struct has_log_state_change : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename FsmT, typename SrcState, typename DstState>
struct has_log_state_change<FsmT, SrcState, DstState,
    void_t<decltype(FsmT::logger().log_state_change(
        std::declval<FsmT const&>(), std::declval<SrcState const&>(),
        std::declval<DstState const&>()))>>
    : std::true_type
{
};
#endif
template<typename FsmT, typename SrcState, typename DstState>
constexpr inline auto has_log_state_change_v{has_log_state_change<FsmT, SrcState, DstState>::value};

template<typename FsmT, typename State, typename = void_t<>>
struct has_log_exit : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename FsmT, typename State>
struct has_log_exit<FsmT, State,
    void_t<decltype(FsmT::logger().log_exit(
        std::declval<FsmT const&>(),std::declval<State const&>()))>>
    : std::true_type
{
};
#endif
template<typename FsmT, typename State>
constexpr inline auto has_log_exit_v{has_log_exit<FsmT,State>::value};

template<typename FsmT, typename State, typename = void_t<>>
struct has_log_entry : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename FsmT, typename State>
struct has_log_entry<FsmT, State,
    void_t<decltype(FsmT::logger().log_entry(
        std::declval<FsmT const&>(),std::declval<State const&>()))>>
    : std::true_type
{
};
#endif
template<typename FsmT, typename State>
constexpr inline auto has_log_entry_v{has_log_entry<FsmT,State>::value};


template <class, int N, size_type... Ns>
auto get_type_name(const char *ptr, Index_sequence<Ns...>) noexcept {
  static const char str[] = {ptr[N + Ns]..., 0};
  return str;
}
} // namespace detail


template <class T>
const char *get_type_name() {
// 53 -> offset from the beginning of name deduced by __PRETTY__FUNCTION
// -2 -> trailing ']' and '\0' in the deduced name
return detail::get_type_name<T, 53>(
    __PRETTY_FUNCTION__, Make_index_sequence<sizeof(__PRETTY_FUNCTION__) - 53 - 2>{});
}

/* log event */
/* --------------------------------------------------------------------------------------------- */
namespace detail
{

template<typename FsmT, typename State, typename Event, bool = has_log_event_v<FsmT, State, Event>>
struct fsm_log_event_impl {
    constexpr inline void operator()(FsmT const&, State const&, Event const&) const noexcept
    { /* nop */ }
};

template<typename FsmT, typename State, typename Event>
struct fsm_log_event_impl<FsmT, State, Event, true> {
    inline void operator()(FsmT const& fsm, State const& state, Event const& event) const
    {
        FsmT::logger().log_event(fsm, state, event);
    }
};

} // detail

template<typename FsmT, typename State, typename Event>
constexpr inline void fsm_log_event(FsmT const& fsm, State const& state, Event const& event)
{
    detail::fsm_log_event_impl<FsmT, State, Event>{}(fsm, state, event);
}

// template<typename FsmT, typename State, typename Event>
// std::enable_if_t<detail::has_log_event_v<FsmT, State, Event>>
// fsm_log_event(FsmT const& fsm, State const& state, Event const& event) noexcept
// {
//     FsmT::logger().log_event(fsm, state, event);
// }

// template<typename FsmT, typename State, typename Event>
// std::enable_if_t<!detail::has_log_event_v<FsmT, State, Event>>
// fsm_log_event(FsmT const&, State const&, Event const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */

/* log event */
/* --------------------------------------------------------------------------------------------- */

namespace detail
{

template<typename FsmT, typename Guard, bool = has_log_guard_v<FsmT, Guard>>
struct fsm_log_guard_impl {
    constexpr inline void operator()(FsmT const&, Guard const&, bool) const noexcept { /* nop */ }
};

template<typename FsmT, typename Guard>
struct fsm_log_guard_impl<FsmT, Guard, true> {
    inline void operator()(FsmT const& fsm, Guard const& guard, bool result) const
    {
        FsmT::logger().log_guard(fsm, guard, result);
    }
};

} // detail

template<typename FsmT, typename Guard>
constexpr inline void fsm_log_guard(FsmT const& fsm, Guard const& guard, bool result)
{
    detail::fsm_log_guard_impl<FsmT, Guard>{}(fsm, guard, result);
}


// template<typename FsmT, typename Guard/* , typename Event */>
// std::enable_if_t<detail::has_log_guard_v<FsmT, Guard/* , Event */>>
// fsm_log_guard(FsmT const& fsm, Guard const& guard, /* Event const& event, */ bool result) noexcept
// {
//     FsmT::logger().log_guard(fsm, guard, /* event, */ result);
// }

// template<typename FsmT, typename Guard/* , typename Event */>
// std::enable_if_t<!detail::has_log_guard_v<FsmT, Guard/* , Event */>>
// fsm_log_guard(FsmT const&, Guard const&, /* Event const&, */ bool) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
namespace detail
{

template<typename FsmT, typename Action, bool = has_log_action_v<FsmT, Action>>
struct fsm_log_action_impl {
    constexpr inline void operator()(FsmT const&, Action const&) const noexcept { /* nop */ }
};

template<typename FsmT, typename Action>
struct fsm_log_action_impl<FsmT, Action, true> {
    inline void operator()(FsmT const& fsm, Action const& action) const
    {
        FsmT::logger().log_action(fsm, action);
    }
};

} // detail

template<typename FsmT, typename Action>
constexpr inline void fsm_log_action(FsmT const& fsm, Action const& action)
{
    detail::fsm_log_action_impl<FsmT, Action>{}(fsm, action);
}


// template<typename FsmT, typename Action/* , typename Event */>
// std::enable_if_t<detail::has_log_action_v<FsmT, Action/* , Event */>>
// fsm_log_action(FsmT const& fsm, Action const& action/*,  Event const& event */) noexcept
// {
//     FsmT::logger().log_action(fsm, action/* , event */);
// }

// template<typename FsmT, typename Action/* , typename Event */>
// std::enable_if_t<!detail::has_log_action_v<FsmT, Action/* , Event */>>
// fsm_log_action(FsmT const&, Action const&/* , Event const& */) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename SrcState, typename DstState>
std::enable_if_t<detail::has_log_state_change_v<FsmT, SrcState, DstState>>
fsm_log_state_change(FsmT const& fsm, SrcState const& src_state, DstState const& dst_state) noexcept
{
    FsmT::logger().log_state_change(fsm, src_state, dst_state);
}

template<typename FsmT, typename SrcState, typename DstState>
std::enable_if_t<!detail::has_log_state_change_v<FsmT, SrcState, DstState>>
fsm_log_state_change(FsmT const&, SrcState const&, DstState const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename State>
std::enable_if_t<detail::has_log_exit_v<FsmT, State>>
fsm_log_exit(FsmT const& fsm, State const& state) noexcept
{
    FsmT::logger().log_exit(fsm, state);
}

template<typename FsmT, typename State>
std::enable_if_t<!detail::has_log_exit_v<FsmT, State>>
fsm_log_exit(FsmT const&, State const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename State>
std::enable_if_t<detail::has_log_entry_v<FsmT, State>>
fsm_log_entry(FsmT const& fsm, State const& state) noexcept
{
    FsmT::logger().log_entry(fsm, state);
}

template<typename FsmT, typename State>
std::enable_if_t<!detail::has_log_entry_v<FsmT, State>>
fsm_log_entry(FsmT const&, State const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


} // namespace logging
} // namespace ufsm
