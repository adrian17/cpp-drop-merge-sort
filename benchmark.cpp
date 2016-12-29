#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include "randutils.hpp" // see https://gist.github.com/imneme/540829265469e673d045
#include <boost/sort/spreadsort/spreadsort.hpp>
#include "drop_merge_sort.hpp"

using namespace std::chrono;

randutils::default_rng rng;

template<typename T>
std::vector<T> randomize(float factor) {
    int max = 1000000;
    std::vector<T> ret(max);
    for (int i = 0; i < max; ++i) {
        if (rng.uniform(0.0, 1.0) < factor)
            ret[i] = rng.uniform(0, max-1);
        else
            ret[i] = i;
    }
    return ret;
}
template<>
std::vector<std::string> randomize<std::string>(float factor) {
    int max = 100000;
    std::vector<std::string> ret(max);
    for (int i = 0; i < max; ++i) {
        std::string s;
        if (rng.uniform(0.0, 1.0) < factor)
            s = std::to_string(rng.uniform(0, max-1));
        else
            s = std::to_string(i);
        ret[i] = std::string(100 - s.size(), '0') + s;
    }
    return ret;
}

template<typename T, typename Sort>
float measure(float factor, Sort sort) {
    size_t total = 0;
    constexpr size_t sz = 5;
    for (int i = 0; i < sz; ++i) {
        auto tab = randomize<T>(factor);
        //auto copy = tab;
        auto t1 = high_resolution_clock::now();
        sort(tab);
        auto t2 = high_resolution_clock::now();
        size_t time = duration_cast<microseconds>(t2-t1).count();
        total += time;

        // sanity check
        //std::sort(copy.begin(), copy.end());
        //if(copy != tab)
        //    throw "asdf";
    }
    return total / (1000.0 * sz);
}

template<typename T>
void benchmark() {
    for (int i = 0; i <= 100; ++i) {
        float factor = i * 0.01;
        auto dt1 = measure<T>(factor, [](auto &tab){dmsort(std::begin(tab), std::end(tab));});
        auto dt2 = measure<T>(factor, [](auto &tab){std::sort(std::begin(tab), std::end(tab));});
        auto dt3 = measure<T>(factor, [](auto &tab){boost::sort::spreadsort::spreadsort(std::begin(tab), std::end(tab));});
        std::cout << (factor) << "\t" << dt1 << "\t" << dt2 << "\t" << dt3 << "\n";
    }
    std::cout << "\n";
}

int main(){
    benchmark<int>();
    benchmark<std::string>();
}