#pragma once

#include <utility>
#include "traits.hpp"
#include "get.hpp"

namespace ufsm
{
namespace back
{

template<size_type Idx, typename State>
class FsmState
{
private:
    State value_{};
public:
    template<typename, typename...> friend class Fsm_impl;
    template<typename, typename> friend class Fsm;
    using type = State;

    constexpr FsmState() noexcept = default;
    constexpr FsmState(FsmState&) = default;
    constexpr FsmState(FsmState const&) = default;
    constexpr FsmState(FsmState&&) noexcept = default;
    constexpr FsmState& operator=(FsmState const&) = default;
    constexpr FsmState& operator=(FsmState&&) noexcept = default;
    ~FsmState() noexcept = default;

    template<typename U
        /* , typename = std::enable_if_t<!std::is_same_v<FsmState<...>,std::decay_t<U>>> */>
    constexpr FsmState(U&& fvalue) noexcept(std::is_nothrow_constructible_v<State,U>)
        : value_{std::forward<U>(fvalue)} { }

    constexpr State& get() & noexcept { return value_; }
    constexpr State const& get() const& noexcept { return value_; }
    constexpr State&& get() && noexcept { return std::move(value_); }
    constexpr State const&& get() const&& noexcept { return std::move(value_); }
};

template<size_type Idx, typename State>
using FsmState_t = typename FsmState<Idx,State>::type;

/* Get */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT>& Get_impl(FsmState<Index,StateT>& state) noexcept;
template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT> const& Get_impl(FsmState<Index,StateT> const& state) noexcept;
template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT>&& Get_impl(FsmState<Index,StateT>&& state) noexcept;
template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT> const&& Get_impl(FsmState<Index,StateT> const&& state) noexcept;


template<size_type I, typename U>
inline constexpr FsmState_t<I,U>& Get_impl(FsmState<I,U>& st) noexcept
{
    return st.get();
}
template<size_type I, typename U>
inline constexpr FsmState_t<I,U> const& Get_impl(FsmState<I,U> const& st) noexcept
{
    return st.get();
}
template<size_type I, typename U>
inline constexpr FsmState_t<I,U>&& Get_impl(FsmState<I,U>&& st) noexcept
{
    return std::move(st).get();
}
template<size_type I, typename U>
inline constexpr FsmState_t<I,U>&& Get_impl(FsmState<I,U> const&& st) noexcept
{
    return std::move(st).get();
}

template<size_type Idx, typename FsmT>
inline constexpr decltype(auto) Get(FsmT&& fsm) noexcept
{
    return Get_impl<Idx>(std::forward<FsmT>(fsm));
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

} // namespace back
} // namespace ufsm
