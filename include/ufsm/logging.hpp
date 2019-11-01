#pragma once

#include "traits.hpp"
#include <type_traits>


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
template<typename FsmT, typename Action, typename Event,
         typename = void_t<>>
struct has_log_action : std::false_type { };
#if defined(FSM_DEBUG_LOG)
template<typename FsmT, typename Action, typename Event>
struct has_log_action<FsmT, Action, Event,
    void_t<decltype(FsmT::logger().log_action(
        std::declval<FsmT const&>(), std::declval<Action const&>(), std::declval<Event const&>()))>>
    : std::true_type
{
};
#endif
template<typename FsmT, typename Action, typename Event>
constexpr inline auto has_log_action_v{has_log_action<FsmT, Action, Event>::value};

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

template <class, int N, SizeT... Ns>
auto get_type_name(const char *ptr, IndexSequence<Ns...>) noexcept {
  static const char str[] = {ptr[N + Ns]..., 0};
  return str;
}
} // namespace detail


template <class T>
const char* get_type_name() {
// 53 -> offset from the beginning of name deduced by __PRETTY__FUNCTION
// -2 -> trailing ']' and '\0' in the deduced name
#if defined(COMPILING_WITH_MSVC)
    return detail::get_type_name<T, 24>(
        __PRETTY_FUNCTION__, MakeIndexSequence<sizeof(__PRETTY_FUNCTION__) - 24 - 2>{});
#elif defined(__clang__)
    return detail::get_type_name<T, 48>(
        __PRETTY_FUNCTION__, MakeIndexSequence<sizeof(__PRETTY_FUNCTION__) - 48 - 2>{});
#elif defined(__GNUC__)
    return detail::get_type_name<T, 53>(
        __PRETTY_FUNCTION__, MakeIndexSequence<sizeof(__PRETTY_FUNCTION__) - 53 - 2>{});
#endif
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
    noexcept(noexcept(FsmT::logger().log_event(fsm, state, event)))
    {
        FsmT::logger().log_event(fsm, state, event);
    }
};

} // detail

template<typename FsmT, typename State, typename Event>
constexpr inline void fsm_log_event(FsmT const& fsm, State const& state, Event const& event)
noexcept(noexcept(detail::fsm_log_event_impl<FsmT, State, Event>{}(fsm, state, event)))
{
    detail::fsm_log_event_impl<FsmT, State, Event>{}(fsm, state, event);
}
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
    noexcept(noexcept(FsmT::logger().log_guard(fsm, guard, result)))
    {
        FsmT::logger().log_guard(fsm, guard, result);
    }
};

} // detail

template<typename FsmT, typename Guard>
inline void fsm_log_guard(FsmT const& fsm, Guard const& guard, bool result)
noexcept(noexcept(detail::fsm_log_guard_impl<FsmT, Guard>{}(fsm, guard, result)))
{
    detail::fsm_log_guard_impl<FsmT, Guard>{}(fsm, guard, result);
}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
namespace detail
{

template<typename FsmT, typename Action, typename Event, bool = has_log_action_v<FsmT, Action, Event>>
struct fsm_log_action_impl {
    constexpr inline void operator()(FsmT const&, Action const&, Event const&) const noexcept
    {
        /* nop */
    }
};

template<typename FsmT, typename Action, typename Event>
struct fsm_log_action_impl<FsmT, Action, Event, true> {
    inline void operator()(FsmT const& fsm, Action const& action, Event const& event) const
    noexcept(noexcept(FsmT::logger().log_action(fsm, action, event)))
    {
        FsmT::logger().log_action(fsm, action, event);
    }
};

} // detail

template<typename FsmT, typename Action, typename Event>
inline void fsm_log_action(FsmT const& fsm, Action const& action, Event const& event)
noexcept(noexcept(detail::fsm_log_action_impl<FsmT, Action, Event>{}(fsm, action, event)))
{
    detail::fsm_log_action_impl<FsmT, Action, Event>{}(fsm, action, event);
}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
namespace detail
{

template<typename FsmT, typename SrcState, typename DstState,
         bool = has_log_state_change_v<FsmT, SrcState, DstState>>
struct fsm_log_state_change_impl {
    constexpr inline void operator()(FsmT const&, SrcState const&, DstState const&) const noexcept
    {
        /* nop */
    }
};

template<typename FsmT, typename SrcState, typename DstState>
struct fsm_log_state_change_impl<FsmT, SrcState, DstState, true> {
    inline void operator()(FsmT const& fsm, SrcState const& src_state, DstState const& dst_state) const
    noexcept(noexcept(FsmT::logger().log_state_change(fsm, src_state, dst_state)))
    {
        FsmT::logger().log_state_change(fsm, src_state, dst_state);
    }
};

}  // detail

template<typename FsmT, typename SrcState, typename DstState>
constexpr inline void fsm_log_state_change(
    FsmT const& fsm, SrcState const& src_state, DstState const& dst_state)
    noexcept(noexcept(detail::fsm_log_state_change_impl<FsmT, SrcState, DstState>{}(fsm, src_state, dst_state)))
{
    detail::fsm_log_state_change_impl<FsmT, SrcState, DstState>{}(fsm, src_state, dst_state);
}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
namespace detail
{

template<typename FsmT, typename State, bool = has_log_exit_v<FsmT, State>>
struct fsm_log_exit_impl {
    constexpr inline void operator()(FsmT const&, State const&) const noexcept { /* nop */ }
};

template<typename FsmT, typename State>
struct fsm_log_exit_impl<FsmT, State, true> {
    inline void operator()(FsmT const& fsm, State const& state) const
    noexcept(noexcept(FsmT::logger().log_exit(fsm, state)))
    {
        FsmT::logger().log_exit(fsm, state);
    }
};

} // detail

template<typename FsmT, typename State>
constexpr inline void fsm_log_exit(FsmT const& fsm, State const& state)
noexcept(noexcept(detail::fsm_log_exit_impl<FsmT, State>{}(fsm, state)))
{
    detail::fsm_log_exit_impl<FsmT, State>{}(fsm, state);
}


// template<typename FsmT, typename State>
// std::enable_if_t<detail::has_log_exit_v<FsmT, State>>
// fsm_log_exit(FsmT const& fsm, State const& state) noexcept
// {
//     FsmT::logger().log_exit(fsm, state);
// }

// template<typename FsmT, typename State>
// std::enable_if_t<!detail::has_log_exit_v<FsmT, State>>
// fsm_log_exit(FsmT const&, State const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
namespace detail
{

template<typename FsmT, typename State, bool = has_log_entry_v<FsmT, State>>
struct fsm_log_entry_impl {
    constexpr inline void operator()(FsmT const&, State const&) const noexcept { /* nop */ }
};

template<typename FsmT, typename State>
struct fsm_log_entry_impl<FsmT, State, true> {
    inline void operator()(FsmT const& fsm, State const& state) const
    noexcept(noexcept(FsmT::logger().log_entry(fsm, state)))
    {
        FsmT::logger().log_entry(fsm, state);
    }
};

} // detail

template<typename FsmT, typename State>
constexpr inline void fsm_log_entry(FsmT const& fsm, State const& state)
noexcept(noexcept(detail::fsm_log_entry_impl<FsmT, State>{}(fsm, state)))
{
    detail::fsm_log_entry_impl<FsmT, State>{}(fsm, state);
}
/* --------------------------------------------------------------------------------------------- */


} // namespace logging
} // namespace ufsm
