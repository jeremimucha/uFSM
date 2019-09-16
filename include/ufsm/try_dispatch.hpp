#if !defined(UFSM_TRY_DISPATCH_HPP_)
#define UFSM_TRY_DISPATCH_HPP_

#include "traits.hpp"

namespace ufsm
{
namespace back
{
namespace detail
{

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct tryDispatch {
    template<typename Event>
    constexpr inline void operator()(State const&, Event) const noexcept { /* nop */ }
};

template<typename State>
struct tryDispatch<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        std::forward<FsmT>(fsm).dispatch_event(std::forward<Event>(event));
    }
};

} // namespace detail

} // namespace back

} // namespace ufsm


#endif // UFSM_TRY_DISPATCH_HPP_
