#ifndef FSMFWD_HEADER_GUARD_HPP_
#define FSMFWD_HEADER_GUARD_HPP_

namespace ufsm
{
namespace detail
{
template<typename SM>
struct get_fsm_state_list;
} // namespace detail

template<typename Impl, typename Statelist = typename detail::get_fsm_state_list<Impl>::type>
class Fsm;

template<typename State>
struct IsFsmT : std::false_type { };
template<typename Impl, typename States>
struct IsFsmT<::ufsm::Fsm<Impl, States>> : std::true_type { };
template<typename State>
constexpr inline auto IsFsm{IsFsmT<State>::value};

namespace back
{
namespace detail
{

template<typename FsmT> struct BaseFsmStateTImpl;
template<typename Impl, typename States>
struct BaseFsmStateTImpl<::ufsm::Fsm<Impl, States>> { using type = Impl; };
template<typename Impl, typename States>
struct BaseFsmStateTImpl<::ufsm::Fsm<Impl, States> const> { using type = Impl const; };
template<typename Impl, typename States>
struct BaseFsmStateTImpl<::ufsm::Fsm<Impl, States> const&> { using type = Impl const&; };
template<typename Impl, typename States>
struct BaseFsmStateTImpl<::ufsm::Fsm<Impl, States> const&&> { using type = Impl const&&; };
template<typename Impl, typename States>
struct BaseFsmStateTImpl<::ufsm::Fsm<Impl, States>&> { using type = Impl&; };
template<typename Impl, typename States>
struct BaseFsmStateTImpl<::ufsm::Fsm<Impl, States>&&> { using type = Impl&&; };

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct BaseFsmStateT { using type = State; };
template<typename State>
struct BaseFsmStateT<State, true> : BaseFsmStateTImpl<State> { };

template<typename State>
using BaseFsmState = typename BaseFsmStateT<State>::type;

template<typename FsmT>
constexpr decltype(auto) asBaseState(FsmT&& fsm) noexcept
{
    return static_cast<BaseFsmState<FsmT>>(std::forward<FsmT>(fsm));
}

} // namespace detail
} // namespace back

} // namespace ufsm

#endif /* FSMFWD_HEADER_GUARD_HPP_ */
