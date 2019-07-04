#if !defined(TRY_EXIT_HPP_)
#define TRY_EXIT_HPP_

#include "traits.hpp"
#include "exit_action.hpp"

namespace ufsm
{
namespace back
{
namespace detail
{

template<typename Indices> struct exitCurrentState;
template<>
struct exitCurrentState<Index_sequence<>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&&) const noexcept {/* nop */}
};
template<size_type I, size_type... Is>
struct exitCurrentState<Index_sequence<I, Is...>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept
    {
        if (I == fsm.state()) {
            fsm_exit(std::forward<FsmT>(fsm), Get<I>(std::forward<FsmT>(fsm)));
            return;
        }
        exitCurrentState<Index_sequence<Is...>>{}(std::forward<FsmT>(fsm));
    }
};

template<typename State, bool = isFsm<std::decay_t<State>>
struct tryExit {
    constexpr inline void operator()(State const&) const noexcept {/* nop */}
};

template<typename State>
struct tryExit<State, true> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept
    {
        exitCurrentState<typename FsmT::Indices>{}(std::forward<FsmT>(fsm));
    }
};

} // namespace detail

} // namespace back

} // namespace ufsm


#endif // TRY_EXIT_HPP_
