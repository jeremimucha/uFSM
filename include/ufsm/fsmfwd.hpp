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

} // namespace ufsm

#endif /* FSMFWD_HEADER_GUARD_HPP_ */
