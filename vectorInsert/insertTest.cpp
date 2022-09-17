#include <iostream>
#include <chrono>
#include <vector>
#include <type_traits>

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
    printTraits<SimplePair>();

    std::chrono::high_resolution_clock timer;
#if 1
    using Pair = std::pair<int, int>;
#else
    using Pair = SimplePair;
#endif
    const int nInsertCount = 100000;
    std::vector<Pair> cont;
    cont.reserve(nInsertCount);

    const auto tStart = timer.now();
    for (int i = 0; i < nInsertCount; ++i)
        cont.insert(cont.begin(), Pair(i, i));

    const auto nSum = [&]() { int nSum = 0; for (auto pair : cont) nSum += pair.first + pair.second; return nSum; }();
    const auto tEnd = timer.now();

    std::cout << "Time for " << typeid(Pair).name() << ": " << std::chrono::duration<double>(tEnd - tStart).count() << " (sum = " << nSum << ")\n";

    return 0;
}
