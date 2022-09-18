#include <iostream>
#include <chrono>
#include <vector>
#include <type_traits>
#include <tuple>

struct SimplePair
{
    SimplePair() = default;
    SimplePair(int a, int b) : first(a), second(b) {}
    int first = 0;
    int second = 0;
};

template <class T>
void printTraits()
{
    std::cout << "Traits for " << typeid(T).name() << '\n';
    std::cout << "is_trivially_copyable              = " << std::is_trivially_copyable_v<T> << '\n';
    std::cout << "is_trivially_default_constructible = " << std::is_trivially_default_constructible_v<T> << '\n';
    std::cout << "is_trivially_destructible          = " << std::is_trivially_destructible_v<T> << '\n';
    std::cout << "is_nothrow_constructible           = " << std::is_nothrow_constructible_v<T> << '\n';
    std::cout << "is_nothrow_move_assignable         = " << std::is_nothrow_move_assignable_v<T> << '\n';
    std::cout << "is_trivially_assignable<T,T>       = " << std::is_trivially_assignable_v<T, T> << '\n';
    std::cout << "is_trivially_move_assignable       = " << std::is_trivially_move_assignable_v<T> << '\n';
    std::cout << "is_trivially_copy_assignable       = " << std::is_trivially_copy_assignable_v<T> << '\n';
    std::cout << '\n';
}

int main()
{
    printTraits<std::pair<int,int>>();
    printTraits<std::tuple<int,int>>();
    printTraits<SimplePair>();

    using Pair = std::pair<int, int>;
    //using Pair = std::tuple<int, int>;
    //using Pair = SimplePair;

    const int nInsertCount = 100000;
    std::vector<Pair> cont;
    cont.reserve(nInsertCount);

    std::chrono::high_resolution_clock timer;
    const auto tStart = timer.now();
    for (int i = 0; i < nInsertCount; ++i)
        cont.insert(cont.begin(), Pair(i, i));

    const auto nSum = [&]() { int nSum = 0; for (auto [a, b] : cont) nSum += a + b; return nSum; }();
    const auto tEnd = timer.now();

    std::cout << "Time for " << typeid(Pair).name() << ": " << std::chrono::duration<double>(tEnd - tStart).count() << " (sum = " << nSum << ")\n";

    return 0;
}
