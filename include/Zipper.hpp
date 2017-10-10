#ifndef SIMPLE_ZIPPER_HPP
#define SIMPLE_ZIPPER_HPP

#include <iterator>
#include <tuple>

namespace Simple {

template <typename Ts...>
class Zipper {
    static_assert(sizeof...(Ts) > 0, "Zipper take one ore more types.");
public:
    class iterator : std::iterator<std::forward_iterator_tag,
        std::tuple<Ts::value_type>> {
    protected:
        std::tuple<Ts::iterator...> current;
    public:
        explicit iterator(typename Ts::iterator... s) :
            current(s...) {};

        iterator(const iterator& rhs) : current(rhs.current) {};

        iterator& operator++() {
            //increment(current);
            return *this;
        }

        iterator operator++(int) {
            auto a = *this;
            //increment(current);
            return a;
        }

        bool operator!=(const iterator& rhs) {
            return false;// not_equal_tuples(current, rhs.current);
        }

        typename iterator::reference operator*() {
            return dereference_tuple(current);
        }

    };

    using IteratorType = Zipper<Ts...>::iterator;
    using ConstIteratorType = Zipper<Ts...>::const_iterator;

    explicit Zipper(Ts&... c)
        : begin_{ c.begin()... }
        , end_{ c.end()... }
    {}

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
}

#endif
