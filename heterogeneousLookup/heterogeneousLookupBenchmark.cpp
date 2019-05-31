/*

Simple benchmark to demonstrate the performance problems in heterogeneous lookup by const char* from std::string's.

Example run from VC2017 x64 Release with /std:c++17

>>>>>>>>>>>>>>>>>>>>>

Runs with lookup length 1
----------------------------------
classic map, const char* lookup length 1, time: 0.131559, sum: 2302634772218935
classic map, std::string lookup length 1, time: 0.135326, sum: 2302634772218935
std::less<> const char* lookup length 1,  time: 0.119734, sum: 2302634772218935
string_view lookup length 1,              time: 0.144572, sum: 2302634772218935

Runs with lookup length 3
----------------------------------
classic map, const char* lookup length 3, time: 0.14821, sum: 205708300943987
classic map, std::string lookup length 3, time: 0.148739, sum: 205708300943987
std::less<> const char* lookup length 3,  time: 0.168525, sum: 205708300943987
string_view lookup length 3,              time: 0.138451, sum: 205708300943987

Runs with lookup length 32
----------------------------------
classic map, const char* lookup length 32, time: 0.210349, sum: 0
classic map, std::string lookup length 32, time: 0.139863, sum: 0
std::less<> const char* lookup length 32,  time: 0.41165, sum: 0
string_view lookup length 32,              time: 0.142793, sum: 0

Runs with lookup length 1000
----------------------------------
classic map, const char* lookup length 1000, time: 0.667547, sum: 0
classic map, std::string lookup length 1000, time: 0.139149, sum: 0
std::less<> const char* lookup length 1000,  time: 6.20271, sum: 0
string_view lookup length 1000,              time: 0.155679, sum: 0

Runs with lookup length 5000
----------------------------------
classic map, const char* lookup length 5000, time: 2.66173, sum: 0
classic map, std::string lookup length 5000, time: 0.141601, sum: 0
std::less<> const char* lookup length 5000,  time: 29.0042, sum: 0
string_view lookup length 5000,              time: 0.141754, sum: 0

<<<<<<<<<<<<<<<<<<<<<

Notes:
    -The results are highly dependent on implementation: results vary massively between VC2017, GCC 7.4.0 and Clang 6.0.0.
           -The effect of heterogeneous lookup getting worse with the increased size of lookup string can, however, be seen in all of them.
    -Only with string view and pre-constructed std::string lookup strings times are reasonable: independent of lookup string length.
    -The larger the lookup string length is, the worse the effect of the classic temporary std::string gets.
    -heterogeneous lookup suffers a huge performance penalty with increasing lookup string size
        -This is caused by strlen() getting called on every operator<(std::string, const char*) for the lookup string.

*/

#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <chrono>
#include <random>
#include <array>

const char*         lookupTypeConstCharPtr(const std::string& s) { return s.c_str(); }
const std::string&  lookupTypeStdString(const std::string& s)    { return s; }
std::string_view    lookupTypeStringView(const std::string& s)   { return std::string_view(s); }

template <class Cont_T, class Func_T>
void runImpl(const size_t nLookupStringLength, Func_T toLookupType)
{
    std::mt19937 randEng;
    randEng.seed(123456);

    const size_t nMapSize = 100000;
    const size_t nIterCount = 1000000;

    std::array<std::string, 3> arrLookupStrings =
    {
        std::string(nLookupStringLength, '0'),
        std::string(nLookupStringLength, '1'),
        std::string(nLookupStringLength, '2')
    };

    Cont_T cont;
    for (size_t i = 0; i < nMapSize; ++i)
        cont.insert(std::pair<std::string, unsigned int>(std::to_string(i), randEng()));

    std::uniform_int_distribution<size_t> lookupDistr(0, arrLookupStrings.size() - 1);

    const auto endIter = cont.end();
    size_t nSum = 0;
    std::chrono::high_resolution_clock timer;
    const auto startTime = timer.now();
    for (size_t i = 0; i < nIterCount; ++i)
    {
        const auto index = lookupDistr(randEng);
        auto iter = cont.find(toLookupType(arrLookupStrings[index]));
        if (iter != endIter)
            nSum += iter->second;
    }
    const auto endTime = timer.now();
    std::cout << "length " << nLookupStringLength << ", time: " << std::chrono::duration<double>(endTime - startTime).count() << ", sum: " << nSum << "\n";
}

void doRuns(const size_t nLookupStringLength)
{
    std::cout << "Runs with lookup length " << nLookupStringLength << '\n';
    std::cout << "----------------------------------\n";
    std::cout << "classic map, const char* lookup ";
    runImpl<std::map<std::string, unsigned int>>(nLookupStringLength, lookupTypeConstCharPtr);
    std::cout << "classic map, std::string lookup ";
    runImpl<std::map<std::string, unsigned int>>(nLookupStringLength, lookupTypeStdString);
    std::cout << "std::less<> const char* lookup ";
    runImpl<std::map<std::string, unsigned int, std::less<>>>(nLookupStringLength, lookupTypeConstCharPtr);
    std::cout << "string_view lookup ";
    runImpl<std::map<std::string, unsigned int, std::less<>>>(nLookupStringLength, lookupTypeStringView);
}

int main()
{
    doRuns(1);
    std::cout << '\n';
    doRuns(3);
    std::cout << '\n';
    doRuns(32);
    std::cout << '\n';
    doRuns(1000);
    std::cout << '\n';
    doRuns(5000);
}
