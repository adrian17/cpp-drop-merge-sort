#ifndef DROP_MERGE_SORT_H
#define DROP_MERGE_SORT_H

#include <vector>
#include <algorithm>
#include <type_traits>
#include <iterator>

/*
    A C++ reimplementation of a drop-merge sort, originally made by Emil Ernerfeldt:
    https://github.com/emilk/drop-merge-sort

    There are two versions of this function.

    The first one is identical to the reference Rust implementation.
    It doesn't support uncopyable types.
    Here, it's used only for trivially copyable types.
    (just like the Rust version, which required the Copy trait).

    The second version is copy-free, so it supports noncopyable types like std::unique_ptr.
    It's also more efficient for types with expensive copying, like std::string.
    However, being copy-free involves some extra bookkeeping, so for trivial types
    it's a bit less efficient than the original function - which is why I use both versions.
*/

namespace detail {

    constexpr static bool double_comparison = true;

    // with-trivial-copies version
    template<typename Iter, typename Comp>
    void dmsort(Iter begin, Iter end, Comp comp, std::true_type) {
        if (begin == end || std::next(begin) == end)
            return;

        using Value = typename std::iterator_traits<Iter>::value_type;
        std::vector<Value> dropped;

        size_t num_dropped_in_row = 0;
        auto write = begin;
        auto read = begin;

        constexpr size_t recency = 8;

        while (read != end) {
            if (begin != write && comp(*read, *std::prev(write))) {

                if (double_comparison && num_dropped_in_row == 0 && write != std::next(begin)
                    && !comp(*read, *std::prev(write, 2))) {
                    dropped.push_back(*std::prev(write));
                    *std::prev(write) = *read;
                    ++read;
                    continue;
                }

                if (num_dropped_in_row < recency) {
                    dropped.push_back(*read);
                    ++read;
                    ++num_dropped_in_row;
                } else {
                    for (int i = 0; i < num_dropped_in_row; ++i) {
                        dropped.pop_back();
                    }
                    std::advance(read, -num_dropped_in_row);

                    --write;
                    dropped.push_back(*write);

                    num_dropped_in_row = 0;
                }
            } else {
                // Here we don't need to guard against self-move because such an
                // operation can't destroy the value for trivially copyable types
                *write = std::move(*read);
                ++read;
                ++write;
                num_dropped_in_row = 0;
            }
        }

        std::sort(dropped.begin(), dropped.end(), comp);

        auto back = end;

        while (!dropped.empty()) {
            auto & last_dropped = dropped.back();

            while (begin != write && comp(last_dropped, *std::prev(write))) {
                --back;
                --write;
                *back = std::move(*write);
            }
            --back;
            *back = std::move(last_dropped);
            dropped.pop_back();
        }
    }

    // move-only version
    template<typename Iter, typename Comp>
    void dmsort(Iter begin, Iter end, Comp comp, std::false_type) {
        if (begin == end || std::next(begin) == end)
            return;

        using Value = typename std::iterator_traits<Iter>::value_type;
        std::vector<Value> dropped;

        size_t num_dropped_in_row = 0;
        auto write = begin;
        auto read = begin;

        constexpr size_t recency = 8;

        while (read != end) {
            if (begin != write && comp(*read, *std::prev(write))) {

                if (double_comparison && num_dropped_in_row == 0 && write != std::next(begin)
                    && !comp(*read, *std::prev(write, 2))) {
                    dropped.push_back(std::move(*std::prev(write)));
                    *std::prev(write) = std::move(*read);
                    ++read;
                    continue;
                }

                if (num_dropped_in_row < recency) {
                    dropped.push_back(std::move(*read));
                    ++read;
                    ++num_dropped_in_row;
                } else {
                    for (int i = 0; i < num_dropped_in_row; ++i) {
                        --read;
                        *read = std::move(*(dropped.end() - 1));
                        dropped.pop_back();
                    }

                    --write;
                    dropped.push_back(std::move(*write));

                    num_dropped_in_row = 0;
                }
            } else {
                if (read != write) {
                    *write = std::move(*read);
                }
                ++read;
                ++write;
                num_dropped_in_row = 0;
            }
        }

        std::sort(dropped.begin(), dropped.end(), comp);

        auto back = end;

        while (!dropped.empty()) {
            auto & last_dropped = dropped.back();

            while (begin != write && comp(last_dropped, *std::prev(write))) {
                --back;
                --write;
                *back = std::move(*write);
            }
            --back;
            *back = std::move(last_dropped);
            dropped.pop_back();
        }
    }
}
template<typename Iter, typename Comp>
void dmsort(Iter begin, Iter end, Comp comp) {
    using Value = typename std::iterator_traits<Iter>::value_type;
    detail::dmsort(begin, end, comp, std::is_trivially_copyable<Value>());
}
template<typename Iter>
void dmsort(Iter begin, Iter end) {
    dmsort(begin, end, std::less<>());
}

#endif // DROP_MERGE_SORT_H
