#include <iostream>
#include <map>
#include <unordered_map>
#include <random>
#include <chrono>

#include <boost/container/flat_map.hpp>

#include <dfg/os/memoryInfo.hpp>
#include <dfg/str/byteCountFormatter.hpp>
#include <dfg/build/buildTimeDetails.hpp>
#include <dfg/time/timerCpu.hpp>
#include <dfg/time.hpp>
#include <dfg/cont/MapVector.hpp>

template <class K, class V> using MapVectorSoA = dfg::cont::MapVectorSoA<K, V>;
template <class K, class V> using MapVectorAoS = dfg::cont::MapVectorAoS<K, V>;

std::mt19937 randEng(static_cast<unsigned int>(1234));

template <class T> std::string prettyTypeName()								   { return typeid(T).name(); }
template <> std::string prettyTypeName<std::map<int, int>>()                   { return "std::map<int,int>"; }
template <> std::string prettyTypeName<std::unordered_map<int, int>>()         { return "std::unordered_map<int, int>"; }
template <> std::string prettyTypeName<boost::container::flat_map<int, int>>() { return "boost::flat_map<int, int>"; }
template <> std::string prettyTypeName<MapVectorSoA<int, int>>()               { return "MapVectorSoA<int, int>"; }
template <> std::string prettyTypeName<MapVectorAoS<int, int>>()               { return "MapVectorAoS<int, int>"; }

template <class Map_T, class Inserter_T>
void testMap(Inserter_T inserter)
{
    using Timer = dfg::time::TimerCpu;
    const char cDelim = ';';
    std::cout << dfg::time::localDate_yyyy_mm_dd_hh_mm_ss_C() << cDelim;
    //std::cout << std::chrono::utc_clock().now() << cDelim; // Not available in GCC 11.3.0
    std::cout << prettyTypeName<Map_T>() << cDelim;
    Timer timerTotal;
    {
        Timer timerDestroy;
        {
            Map_T m;
            Timer timerInsert;
            const int nInsertCount = 10000000; // 1e7
            // If map-type has reserve, using it.
            if constexpr (requires { m.reserve(nInsertCount); })
                m.reserve(nInsertCount);
            for (int i = 0; i < nInsertCount; ++i)
                inserter(m, i, i);
            std::cout << timerInsert.elapsedWallSeconds() << cDelim;
            // Accessing random element in map to prevent optimizer from thinking nothing uses the data.
            {
                const auto nTest = std::uniform_int_distribution<>(0, nInsertCount - 1)(randEng);
                std::cout << m[nTest] << cDelim;
            }
            timerDestroy = Timer();
        }
        std::cout << timerDestroy.elapsedWallSeconds() << cDelim;
    }
    std::cout << timerTotal.elapsedWallSeconds() << cDelim;
    const auto peakMemUsage = ::DFG_MODULE_NS(os)::getMemoryUsage_process().workingSetPeakSize();
    if (peakMemUsage.has_value())
        std::cout << ::DFG_MODULE_NS(str)::ByteCountFormatter_metric(*peakMemUsage);
    std::cout << cDelim << dfg::getBuildTimeDetailStr<dfg::BuildTimeDetail_compilerAndShortVersion>();
    std::cout << cDelim << dfg::getBuildTimeDetailStr<dfg::BuildTimeDetail_cppStandardVersion>();
    std::cout << cDelim << dfg::getBuildTimeDetailStr<dfg::BuildTimeDetail_buildDebugReleaseType>() << '\n';
}

int main()
{
    std::cout << "Run time;Map type;Insert duration;Random element;Delete duration;Total duration;Peak memory usage;Compiler;C++ standard version;Build type\n";
    testMap<std::map<int, int>>([](auto& m, auto a, auto b) { m.insert(std::pair(a, b)); });
    //testMap<std::unordered_map<int, int>>([](auto& m, auto a, auto b) { m.insert(std::pair(a, b)); });
    //testMap<boost::container::flat_map<int, int>>([](auto& m, auto a, auto b) { m.insert(std::pair(a, b)); });
    //testMap<MapVectorSoA<int, int>>([](auto& m, auto a, auto b) { m.insert(a, b); });
    //testMap<MapVectorAoS<int, int>>([](auto& m, auto a, auto b) { m.insert(a, b); });
}
