/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <tuple>

namespace LC
{
namespace Util
{
	template<typename...>
	struct Typelist
	{
	};

	template<typename>
	struct Head;

	template<template<typename...> class List, typename H, typename... T>
	struct Head<List<H, T...>>
	{
		using Head_t = H;
	};

	template<typename List>
	using Head_t = typename Head<List>::Head_t;

	template<template<typename...> class List, typename H, typename... T>
	constexpr List<T...> Tail (List<H, T...>)
	{
		return {};
	}

	namespace detail
	{
		template<int N, typename List>
		struct DropImpl
		{
			using Result_t = typename DropImpl<N - 1, decltype (Tail (List {}))>::Result_t;
		};

		template<typename List>
		struct DropImpl<0, List>
		{
			using Result_t = List;
		};
	}

	template<int N, template<typename...> class List, typename... Args>
	constexpr typename detail::DropImpl<N, List<Args...>>::Result_t Drop (List<Args...>)
	{
		return {};
	}

	template<template<typename...> class List, typename... Args1, typename... Args2>
	constexpr List<Args1..., Args2...> Concat (List<Args1...>, List<Args2...>)
	{
		return {};
	}

	template<template<typename...> class List>
	constexpr List<> Reverse (List<>)
	{
		return {};
	}

	template<template<typename...> class List, typename Head, typename... Tail>
	constexpr auto Reverse (List<Head, Tail...>) -> decltype (Concat (Reverse (List<Tail...> {}), List<Head> {}))
	{
		return {};
	}

	namespace detail
	{
		template<template<typename...> class List, typename Tuple, size_t... Is>
		constexpr auto InitImpl (std::integer_sequence<size_t, Is...>)
		{
			return List<std::tuple_element_t<Is, Tuple>...> {};
		}
	}

	template<template<typename...> class List, typename... Args>
	constexpr auto Init (List<Args...>)
	{
		return detail::InitImpl<List, std::tuple<Args...>> (std::make_index_sequence<sizeof... (Args) - 1> {});
	}

	template<typename Type, template<typename...> class List, typename... Args>
	constexpr bool HasType (List<Args...>)
	{
		return (std::is_same_v<Type, Args> || ...);
	}

	namespace detail
	{
		template<template<typename> class, typename, typename = void>
		struct Filter;
	}

	template<template<typename> class Pred, typename List>
	using Filter_t = typename detail::Filter<Pred, List>::Result_t;

	namespace detail
	{
		template<template<typename> class Pred, template<typename...> class List, typename Head, typename... Tail>
		struct Filter<Pred, List<Head, Tail...>, std::enable_if_t<Pred<Head>::value>>
		{
			using Result_t = decltype (Concat (List<Head> {}, Filter_t<Pred, List<Tail...>> {}));
		};

		template<template<typename> class Pred, template<typename...> class List, typename Head, typename... Tail>
		struct Filter<Pred, List<Head, Tail...>, std::enable_if_t<!Pred<Head>::value>>
		{
			using Result_t = Filter_t<Pred, List<Tail...>>;
		};

		template<template<typename> class Pred, template<typename...> class List>
		struct Filter<Pred, List<>>
		{
			using Result_t = List<>;
		};
	}

	template<typename T>
	struct AsTypelist;

	template<template<typename...> class OtherList, typename... Args>
	struct AsTypelist<OtherList<Args...>>
	{
		using Result_t = Typelist<Args...>;
	};

	template<typename T>
	using AsTypelist_t = typename AsTypelist<T>::Result_t;

	template<typename F, typename G, typename Def, typename Head, typename... Args>
	auto FirstMatching (F f, G g, Def def, Util::Typelist<Head, Args...>)
	{
		if (f (Head {}))
			return g (Head {});

		if constexpr (sizeof... (Args) > 0)
			return FirstMatching (f, g, def, Util::Typelist<Args...> {});
		else
			return def ();
	}

	namespace detail
	{
		template<template<typename> class Name, typename Def, typename... Args>
		struct Find;

		template<template<typename> class Name, typename Def, typename T, typename... Rest>
		struct Find<Name, Def, T, Rest...> : Find<Name, Def, Rest...> {};

		template<template<typename> class Name, typename Def, typename T, typename... Rest>
		struct Find<Name, Def, Name<T>, Rest...>
		{
			using type = T;
		};

		template<template<typename> class Name, typename Def>
		struct Find<Name, Def>
		{
			using type = Def;
		};
	}

	template<template<typename> class Name, typename Def, typename... Args>
	using Find = typename detail::Find<Name, Def, Args...>::type;
}
}
