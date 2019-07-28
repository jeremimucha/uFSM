#ifndef ENTRY_POLICY_HEADER_GUARD_HPP_
#define ENTRY_POLICY_HEADER_GUARD_HPP_

#include "traits.hpp"

namespace ufsm
{

namespace detail
{

template<typename FsmT, typename = void_t<>>
struct HasEntryPolicyT : std::false_type { };
template<typename FsmT>
struct HasEntryPolicyT<FsmT, void_t<typename FsmT::EntryPolicy>> : std::true_type { };
template<typename FsmT>
constexpr inline auto HasEntryPolicy{HasEntryPolicyT<FsmT>::value};

} // namespace detail

struct InitialStateEntryPolicy { };
struct CurrentStateEntryPolicy { };

template<typename FsmT, bool = detail::HasEntryPolicy<std::decay_t<FsmT>>>
struct GetEntryPolicyT {
    using type = InitialStateEntryPolicy;
};
template<typename FsmT>
struct GetEntryPolicyT<FsmT, true> {
    using type = typename FsmT::EntryPolicy;
};
template<typename FsmT>
using GetEntryPolicy = typename GetEntryPolicyT<FsmT>::type;

} // namespace ufsm

#endif // ENTRY_POLICY_HEADER_GUARD_HPP_
