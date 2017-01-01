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
        size_t size = end - begin;

        if (size < 2)
            return;

        using Value = typename std::iterator_traits<Iter>::value_type;
        std::vector<Value> dropped;

        size_t num_dropped_in_row = 0;
        auto write = begin;
        auto read = begin;

        constexpr size_t recency = 8;

        while (read != end) {
            if (begin != write && comp(*read, *(write - 1))) {

                if (double_comparison && num_dropped_in_row == 0 && write > begin+1 && !comp(*read, *(write-2))) {
                    dropped.push_back(*(write-1));
                    *(write-1) = *read;
                    ++read;
                    continue;
                }

                if (num_dropped_in_row < recency) {
                    dropped.push_back(*read);
                    ++read;
                    ++num_dropped_in_row;
                } else {
                    size_t trunc_to_length = dropped.size() - num_dropped_in_row;
                    dropped.resize(trunc_to_length);
                    read -= num_dropped_in_row;

                    --write;
                    dropped.push_back(*write);

                    num_dropped_in_row = 0;
                }
            } else {
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

            while (begin != write && comp(last_dropped, *(write - 1))) {
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
        size_t size = end - begin;

        if (size < 2)
            return;

        using Value = typename std::iterator_traits<Iter>::value_type;
        std::vector<Value> dropped;

        size_t num_dropped_in_row = 0;
        auto write = begin;
        auto read = begin;

        constexpr size_t recency = 8;

        while (read != end) {
            if (begin != write && comp(*read, *(write - 1))) {

                if (double_comparison && num_dropped_in_row == 0 && write > begin+1 && !comp(*read, *(write-2))) {
                    dropped.push_back(std::move(*(write-1)));
                    *(write-1) = std::move(*read);
                    ++read;
                    continue;
                }

                if (num_dropped_in_row < recency) {
                    dropped.push_back(std::move(*read));
                    ++read;
                    ++num_dropped_in_row;
                } else {
                    size_t trunc_to_length = dropped.size() - num_dropped_in_row;
                    for (int i = 0; i < num_dropped_in_row; ++i) {
                        --read;
                        *read = std::move(*(dropped.end() - (i+1)));
                    }
                    dropped.resize(trunc_to_length);

                    --write;
                    dropped.push_back(std::move(*write));

                    num_dropped_in_row = 0;
                }
            } else {
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

            while (begin != write && comp(last_dropped, *(write - 1))) {
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