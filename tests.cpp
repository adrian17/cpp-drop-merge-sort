#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include "drop_merge_sort.hpp"

/*
    Measure number of special functions used
    (Was used to make sure that the move-only version was really move-only)
*/

int constr;
int destr;
int copy;
int ass;
int move;
int moveass;

struct S{
    int x;
    S() : x(-1)                          { printf("huh"); }
    S(int x) : x(x)                      { constr++; }
    ~S()                                 { destr++; }
    S(const S& rhs) : x(rhs.x)           { copy++; }
    S& operator=(const S& rhs)           { ass++; x = rhs.x; return *this; }
    S(S&& rhs) noexcept : x(rhs.x)       { move++; }
    S& operator=(S&& rhs) noexcept       { moveass++; x = rhs.x; return *this; }
};
bool operator<(const S& lhs, const S& rhs){return lhs.x < rhs.x;}

void test_counts(){
    std::vector<S> tab;
    for(int i = 0; i < 50000000; ++i)
        tab.push_back(S(i));
    tab[6] = S(90);
    tab[75] = S(5);
    tab[675] = S(60);
    tab[975] = S(5);
    tab[100000-10] = S(9812);

    constr = 0;
    copy = 0;
    ass = 0;
    destr = 0;
    move = 0;
    moveass = 0;

    dmsort(std::begin(tab), std::end(tab));
    //std::sort(std::begin(tab), std::end(tab));

    std::cout << "\n";
    std::cout << std::is_sorted(std::begin(tab), std::end(tab)) << "\n";
    #define PRINT(a) std::cout << #a << ' ' << a << '\n';
    PRINT(constr);
    PRINT(copy);
    PRINT(ass);
    PRINT(destr);
    PRINT(move);
    PRINT(moveass);
}

/*
    Check that the move-only version actually works with move-only types
*/

void test_noncopyable(){
    std::vector<std::unique_ptr<int>> tab;
    for(int i = 0; i < 10; ++i)
        tab.push_back(std::make_unique<int>(-i));
    auto cmp = [](const auto &a, const auto &b){return *a < *b;};
    //std::sort(tab.begin(), tab.end(), cmp);
    dmsort(tab.begin(), tab.end(), cmp);
    for(auto &e : tab)
        std::cout << *e << " ";
    std::cout << "\n";
    std::cout << std::is_sorted(std::begin(tab), std::end(tab), cmp) << "\n";
}

struct NonDefaultConstructible {
    NonDefaultConstructible() = delete;
    NonDefaultConstructible(const NonDefaultConstructible&) = delete;
    NonDefaultConstructible& operator=(const NonDefaultConstructible&) = delete;
    NonDefaultConstructible(int value) : value(value) {}
    NonDefaultConstructible(NonDefaultConstructible&& other) : value(other.value) {}
    NonDefaultConstructible& operator=(NonDefaultConstructible&& other) {
        value = other.value;
        return *this;
    }

    int value;
};

bool operator<(const NonDefaultConstructible& lhs, const NonDefaultConstructible& rhs) {
    return lhs.value < rhs.value;
}

void test_non_default_constructible(){
    std::vector<NonDefaultConstructible> tab;
    for(int i = 0; i < 10; ++i)
        tab.push_back(NonDefaultConstructible(-i));
    dmsort(tab.begin(), tab.end());
    for(auto &e : tab)
        std::cout << e.value << " ";
    std::cout << "\n";
    std::cout << std::is_sorted(std::begin(tab), std::end(tab)) << "\n";
}

int main(){
    test_counts();
    test_noncopyable();
    test_non_default_constructible();
}
