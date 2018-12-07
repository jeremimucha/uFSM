#pragma once

#include "traits.hpp"
#include <type_traits>
// TODO: Decide if it's worth passing events down the call chain just to have additional logging
// information. As is, events are not passed down to `transition` from `dispatch event`.
// If it turns out useful for guards and actions to take an `event` parameter it might be worth it.
// Alternatively a dummy type<Event>{} could be passed to the logging functions, just to add the
// type information without providing access to the actual instance.


/* log event */
/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename State, typename Event>
std::enable_if_t<has_log_event_v<FsmT, State, Event>>
fsm_log_event(FsmT const& fsm, State const& state, Event const& event) noexcept
{
    FsmT::logger().log_event(fsm, state, event);
}

template<typename FsmT, typename State, typename Event>
std::enable_if_t<!has_log_event_v<FsmT, State, Event>>
fsm_log_event(FsmT const&, State const&, Event const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */

/* log event */
/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename Guard/* , typename Event */>
std::enable_if_t<has_log_guard_v<FsmT, Guard/* , Event */>>
fsm_log_guard(FsmT const& fsm, Guard const& guard, /* Event const& event, */ bool result) noexcept
{
    FsmT::logger().log_guard(fsm, guard, /* event, */ result);
}

template<typename FsmT, typename Guard/* , typename Event */>
std::enable_if_t<!has_log_guard_v<FsmT, Guard/* , Event */>>
fsm_log_guard(FsmT const&, Guard const&, /* Event const&, */ bool) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename Action/* , typename Event */>
std::enable_if_t<has_log_action_v<FsmT, Action/* , Event */>>
fsm_log_action(FsmT const& fsm, Action const& action/*,  Event const& event */) noexcept
{
    FsmT::logger().log_action(fsm, action/* , event */);
}

template<typename FsmT, typename Action/* , typename Event */>
std::enable_if_t<!has_log_action_v<FsmT, Action/* , Event */>>
fsm_log_action(FsmT const&, Action const&/* , Event const& */) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename SrcState, typename DstState>
std::enable_if_t<has_log_state_change_v<FsmT, SrcState, DstState>>
fsm_log_state_change(FsmT const& fsm, SrcState const& src_state, DstState const& dst_state) noexcept
{
    FsmT::logger().log_state_change(fsm, src_state, dst_state);
}

template<typename FsmT, typename SrcState, typename DstState>
std::enable_if_t<!has_log_state_change_v<FsmT, SrcState, DstState>>
fsm_log_state_change(FsmT const&, SrcState const&, DstState const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename State>
std::enable_if_t<has_log_exit_v<FsmT, State>>
fsm_log_exit(FsmT const& fsm, State const& state) noexcept
{
    FsmT::logger().log_exit(fsm, state);
}

template<typename FsmT, typename State>
std::enable_if_t<!has_log_exit_v<FsmT, State>>
fsm_log_exit(FsmT const&, State const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
template<typename FsmT, typename State>
std::enable_if_t<has_log_entry_v<FsmT, State>>
fsm_log_entry(FsmT const& fsm, State const& state) noexcept
{
    FsmT::logger().log_entry(fsm, state);
}

template<typename FsmT, typename State>
std::enable_if_t<!has_log_entry_v<FsmT, State>>
fsm_log_entry(FsmT const&, State const&) noexcept {/* nop */}
/* --------------------------------------------------------------------------------------------- */
