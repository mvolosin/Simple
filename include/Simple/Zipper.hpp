#ifndef SIMPLE_ZIPPER_HPP
#define SIMPLE_ZIPPER_HPP

#include <iterator>
#include <tuple>

namespace Simple {
namespace details {
template <std::size_t TRem>
class TupleForEachHelper {
public:
    template <typename TTuple, typename TFunc>
    static void exec(TTuple&& tuple, TFunc&& func)
    {
        using Tuple = typename std::decay<TTuple>::type;
        constexpr std::size_t TupleSize = std::tuple_size<Tuple>::value;
        static_assert(TRem <= TupleSize, "Incorrect parameters");

        // Invoke function with current element
        static const std::size_t Idx = TupleSize - TRem;
        func(std::get<Idx>(std::forward<TTuple>(tuple)));

        // Compile time recursion - invoke function with the remaining elements
        TupleForEachHelper<TRem - 1>::exec(std::forward<TTuple>(tuple), std::forward<TFunc>(func));
    }
};

template <>
class TupleForEachHelper<0> {
public:
    // Stop compile time recursion
    template <typename TTuple, typename TFunc>
    static void exec(TTuple&& tuple, TFunc&& func)
    {
        static_cast<void>(tuple);
        static_cast<void>(func);
    }
};

template <std::size_t TRem>
class AreSameHelper {
public:
    template <typename TTuple>
    static bool exec(TTuple&& tuple1, TTuple&& tuple2)
    {
        using Tuple = typename std::decay<TTuple>::type;
        constexpr std::size_t TupleSize = std::tuple_size<Tuple>::value;
        static_assert(TRem <= TupleSize, "Incorrect parameters");

        // Invoke function with current element
        static const std::size_t Idx = TupleSize - TRem;

        auto& e1 = std::get<Idx>(std::forward<TTuple>(tuple1));
        auto& e2 = std::get<Idx>(std::forward<TTuple>(tuple2));

        if (e1 != e2) {
            return false;
        }

        // Compile time recursion - invoke function with the remaining elements
        AreSameHelper<TRem - 1>::exec(std::forward<TTuple>(tuple1), std::forward<TTuple>(tuple2));
    }
};

template <>
class AreSameHelper<0> {
public:
    // Stop compile time recursion
    template <typename TTuple>
    static bool exec(TTuple&& tuple1, TTuple&& tuple2)
    {
        return true;
    }
};
} // namespace details

template <typename TTuple, typename TFunc>
void tupleForEach(TTuple&& tuple, TFunc&& func)
{
    using Tuple = typename std::decay<TTuple>::type;
    constexpr std::size_t TupleSize = std::tuple_size<Tuple>::value;

    details::TupleForEachHelper<TupleSize>::exec(std::forward<TTuple>(tuple), std::forward<TFunc>(func));
}

template <typename TTuple>
bool areSame(TTuple&& tuple1, TTuple&& tuple2)
{
    using Tuple = typename std::decay<TTuple>::type;
    constexpr std::size_t TupleSize = std::tuple_size<Tuple>::value;

    return details::AreSameHelper<TupleSize>::exec(std::forward<TTuple>(tuple1), std::forward<TTuple>(tuple2));
}

struct Increment {
public:
    template <class T>
    void operator()(T& other)
    {
        other++;
    }
};

template <size_t... n>
struct ct_integers_list {
    template <size_t m>
    struct push_back {
        typedef ct_integers_list<n..., m> type;
    };
};

template <size_t max>
struct ct_iota_1 {
    typedef typename ct_iota_1<max - 1>::type::template push_back<max>::type type;
};

template <>
struct ct_iota_1<0> {
    typedef ct_integers_list<> type;
};

/****************************
// dereference a subset of elements of a tuple (dereferencing the iterators)
****************************/
template <size_t... indices, typename Tuple>
auto dereference_subset(const Tuple& tpl, ct_integers_list<indices...>)
    -> decltype(std::tie(*std::get<indices - 1>(tpl)...))
{
    return std::tie(*std::get<indices - 1>(tpl)...);
}

/****************************
// dereference every element of a tuple (applying operator* to each element, and returning the tuple)
****************************/
template <typename... Ts>
inline auto dereference_tuple(std::tuple<Ts...>& t1)
    -> decltype(dereference_subset(std::tuple<Ts...>(), typename ct_iota_1<sizeof...(Ts)>::type()))
{
    return dereference_subset(t1, typename ct_iota_1<sizeof...(Ts)>::type());
}

template <class... Ts>
class Zipper {
    static_assert(sizeof...(Ts) > 0, "Zipper take one ore more types.");

public:
    class iterator {
    public:
        using type = std::tuple<typename Ts::iterator...>;
        using dereferenced_type = std::tuple<typename Ts::value_type&...>;
        using reference = type&;
        using pointer = type*;

        explicit iterator(typename Ts::iterator... s)
            : current(s...){};

        iterator(const iterator& rhs)
            : current(rhs.current){};

        iterator& operator++()
        {
            increment();
            return *this;
        }

        iterator operator++(int)
        {
            auto a = *this;
            increment();
            return a;
        }

        bool operator!=(const iterator& other)
        {
            return !areSame(current, other.current);
        }

        auto operator*()
        {
            return std::tie(current);
        }

    protected:
        std::tuple<typename Ts::iterator...> current;

    private:
        void increment()
        {
            tupleForEach(current, Increment());
        }
    };

    using IteratorType = Zipper<Ts...>::iterator;
    using ConstIteratorType = const IteratorType;

    explicit Zipper(Ts&... c)
        : begin_{c.begin()...}
        , end_{c.end()...}
    {
    }

    IteratorType begin() const
    {
        return begin_;
    }

    IteratorType end() const
    {
        return end_;
    }

    ConstIteratorType cbegin() const
    {
        return begin_;
    }

    ConstIteratorType cend() const
    {
        return end_;
    }

private:
    IteratorType begin_;
    IteratorType end_;
};
} // namespace Simple

#endif
