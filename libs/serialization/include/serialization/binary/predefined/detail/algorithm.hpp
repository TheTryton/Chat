#pragma once

#include <serialization/includes.hpp>
#include <tuple>

SERIALIZATION_NAMESPACE_BEGIN
namespace detail
{

template<typename... Types, typename F, size_t... I>
requires (std::is_invocable_v<F, const Types&> && ...)
void for_each_impl(const std::tuple<Types...>& t, F&& f, std::index_sequence<I...>)
{
    int dummy[] = {(f(std::get<I>(t)), 0)...};
}

template<typename... Types, typename F, size_t... I>
requires (std::is_invocable_v<F, Types&> && ...)
void for_each_impl(std::tuple<Types...>& t, F&& f, std::index_sequence<I...>)
{
int dummy[] = {(f(std::get<I>(t)), 0)...};
}

template<typename... Types, typename F>
requires (std::is_invocable_v<F, const Types&> && ...)
void for_each(const std::tuple<Types...>& t, F&& f)
{
    for_each_impl(t, std::forward<F>(f), std::make_index_sequence<sizeof...(Types)>());
}
template<typename... Types, typename F>
requires (std::is_invocable_v<F, Types&> && ...)
void for_each(std::tuple<Types...>& t, F&& f)
{
detail::for_each_impl(t, std::forward<F>(f), std::make_index_sequence<sizeof...(Types)>());
}

}
SERIALIZATION_NAMESPACE_END