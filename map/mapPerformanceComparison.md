# Performance benchmarking for map-implementations

## General

This document present some performance aspects related to various map implementations, namely
* std::map
* std::unordered_map
* boost::flat_map
* dfglib::MapVectorAoS
* dfglib::MapVectorSoA

Instead of looking at performance as function of element count, the focus is to give concrete examples at some arbitrarily chosen element count and use case. All code was compiled with 32-bit MSVC2015 update 3 and boost 1.61.0.

Note that while these benchmarks compare maps, results of tests that only use key's are probably more or less directly translatable to corresponding set-implementations.

## 1. Example: Insert performance with \<int, int\> maps

This benchmarked insert time of 50000 random integers which resulted to map of size 49936 (i.e. 64 ints were duplicates). The detailed  test code, that uses revision [06f3ceb678b9e39f0462d40378a11fd18145dee3](https://github.com/tc3t/dfglib/tree/06f3ceb678b9e39f0462d40378a11fd18145dee3) of dfglib, can be found from [here](dfgTestContMapVectorPerformance.cpp), but essentially the code measured the time taken by the following loop:
```C++
for (int i = 0; i < 50000; ++i)
{
    auto key = random_integer_in_range(-10000000, 10000000);
    cont.insert(std::pair<int, int>(key, key));
}
```

Below is a chart showing the times for different maps and a table with relative times (the raw result table can be found from [here](benchmarkMapVectorInsertPerformance_MSVC_2015_u3_32_release.csv).)

![alt text](charts/insert.png)

| Container     | Relative time (median time) | 
| ------------- | ------------- |
| Interleaved std::vector (50000 pairs) | 0.22 |
| Interleaved boost::vector (50000 pairs) | 0.29 |
| MapVectorAoS push-sort-unique, reserved | 1.00 |
| MapVectorAoS push-sort-unique, not reserved | 1.01 |
| std::unordered_map | 1.65 |
| std::map | 2.48 |
| MapVectorAoS, sorted, not reserved | 32.85 |
| MapVectorAoS, sorted, reserved | 32.96 |
| MapVectorSoA, sorted, not reserved | 33.06 |
| MapVectorSoA, sorted, reserved | 33.09 |
| MapVectorSoA, not sorted, reserved | 72.90 |
| MapVectorSoA, not sorted, not reserved | 73.19 |
| boost::flat_map | 78.69 |
| MapVectorAoS, not sorted, not reserved | 80.38 |
| MapVectorAoS, not sorted, reserved | 80.49 |

The graph and the table shows that the differences are big as expected: almost two orders of magnitude. Notes:

* Both sorted MapVector's are much faster than boost::flat_map. This is essentially a concequence of differences in vector insert performance, which has been analysed in a [separate document](https://github.com/tc3t/benchmarks/blob/master/vectorInsert/vectorInsertBenchmarking.md).

* The fastest time is achieved with vector-based map, which allows the use of push-sort-unique -technique, i.e. push everything to map, sort and remove duplicates.

* For vector-based maps, reserve() has practically no effect on the results.

## 2. Example: Find performance with \<int, int\> maps

This benchmarked find times from the maps constructed in Example 1 using 250000 random keys with find percentage of about 0.25 %. The timed code was essentially the following (for the actual test code see the links in Example 1):

```C++
for (int i = 0; i < 250000; ++i)
{
    auto key = random_integer_in_range(randEng, -10000000, 10000000);
    nFound += (cont.find(key) != cont.end());
}
```

Result are below in graphical form and as table (non-sorted maps are not included in the chart). The raw result table can be found from [here](benchmarkMapVectorFindPerformance_MSVC_2015_u3_32_release.csv)):

![alt text](charts/find.png)

| Container     | Relative time (median time) | 
| ------------- | ------------- |
| std::unordered_map | 1.00 |
| boost::flat_map | 2.39 |
| MapVectorSoA, sorted | 2.48 (*) |
| MapVectorAoS, sorted | 2.51 (*) |
| std::map | 3.75 |
| MapVectorSoA, not sorted | 296.18 (*) |
| MapVectorAoS, not sorted | 331.66 (*) |

\(*) Average of two 5 iteration runs.

Notes:

* std::unordered_map is clearly the fastest as expected.
* boost::flap_map is slightly faster than MapVector's suggesting a quality of implementation issue in MapVector.
* Vector-based maps are faster than std::map as expected.
* For non-sorted case that does a linear search, the better performance of MapVectorSoA is reckoned to be the result of better locality when searching keys (SoA has separate arrays for keys and values instead of storing array of pairs).


## Miscellaneous

* Details on test machines (listed in raw results file in column 'Test machine')
    * 1: 
        * OS: TODO
        * CPU: TODO

