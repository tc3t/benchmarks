# Performance benchmarking for map-implementations

## General

This document present some performance aspects related to various map implementations, namely
* std::map
* std::unordered_map
* boost::flat_map
* dfglib::MapVectorAoS
* dfglib::MapVectorSoA
* Draft implementations of flat map proposals (only in example 3)

Instead of looking at performance as function of element count, the focus is to give concrete examples at some arbitrarily chosen element count and use case. All code was compiled with 32-bit MSVC2015 update 3 and boost 1.61.0.

Note that while these benchmarks compare maps, results of tests that only use key's are probably more or less directly translatable to corresponding set-implementations.

### Disclaimer

Performance benchmarking is a tricky field and while the results provided are made with the intention of being correct enough to aid evaluating the performance in some special use cases, the reader should be aware that the author can not guarantee the accuracy of the results or even the methological correctness. If you intend to make decision based on the results presented here, it's strongly recommended to personally verify the used test code. If flaws are detected, reports and corrections would be highly appreciated.

---

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

---

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

---

 ## 3. Example: Finding map keys of type std::string using const char* as search parameter

Until C++14, std::map find() took key_type as parameter and thus passing const char* to find() of a map with std::string keys forced creation of std::string, which could also mean a memory allocation. This example examines the effect in some cases using pre C++14 behaviour and dfglib revision [5cebbad64e0066f8c733cc8018ecdc22a53c9d17](https://github.com/tc3t/dfglib/tree/5cebbad64e0066f8c733cc8018ecdc22a53c9d17).

 ### 3.1 Test overview

Using various map-implementations, the following code was run.

 * 1. Create N path-like std::string keys to map all of which are of format "c:/an/example/path/file{}.txt", where '{}' is replaced by a running index in range [0, N).
 * 2. For M times, randomly pick a const char* from a static array and pass it to map's find(). The strings are:
    * "c:/an/example/path/file5.txt",
    * "c:/an/example/path/file55",
    * "d:/temp",
    * "C:/temp",
    * "c:/an/example/path/file1.txt",
    * "c:/an/example/path/file.txt"

Time was measured for step 2 with the following implementations with key_type = std::string, mapped_type = int:

* std::map
* std::unordered_map
* boost::container::flat_map
* P0460R0 flat_map draft ([proposal](http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2016/p0460r0.pdf), [code](https://github.com/WG21-SG14/flat_containers/tree/master/source) and a [single line edit](P0460R0_flat_map.diff) to make it compile)
* Pubby flat_map draft ([proposal](http://pubby.github.io/proposal.html), [code](https://github.com/pubby/flat))
* MapVector's (AoS and SoA with both sorted and unsorted)

With parameters N = 3, M = 33333333 (i.e. tiny map and huge number of find()-calls), the results looked like the following (using median time of 5 runs):

| Container     | Relative time (median time) | 
| ------------- | ------------- |
| MapVectorSoA, not sorted | 1.000 |
| MapVectorAoS, not sorted | 1.003 |
| MapVectorSoA, sorted | 1.23 |
| MapVectorAoS, sorted | 1.27 |
| P0460R0 flat map | 1.40 |
| std::unordered_map | 2.04 |
| std::map | 2.04 |
| boost::flat_map | 2.21 |
| pubby flat map | 3.81 |

With tiny map, the search time is small and the memory allocation forced by pre C++14 std and boost implementations is relatively expensive making MapVector and P0460R0 flat map clearly the fastest. The reason for MapVector performing significantly better than P0460R0 flat map is probably due to the search type optimization in MapVector: while find() takes const char\*, the actual search is done using a string view which avoids redundant strlen() calls (at least in VC2015 std::string() < "abc" will call strlen() on "abc" on each comparison).

With parameters N = 1000, M = 10000000 (i.e. much bigger map causing searching to be more expensive), the results looked like the following:

| Container     | Relative time (median time) | 
| ------------- | ------------- |
| MapVectorSoA, sorted | 1.00 |
| MapVectorAoS, sorted | 1.03 |
| std::map | 1.31 |
| std::unordered_map | 1.33 |
| boost::flat_map | 1.43 |
| P0460R0 flat map | 1.81 |
| MapVectorSoA, not sorted | 5.13 |
| pubby flat map | 6.14 |
| MapVectorAoS, not sorted | 6.91 |

Raw result table can be found from [here](benchmarkStringMapWithCharPtrLookup_MSVC_2015_u3_32_release.csv).

Notes:

* While unordered_map has [constant on average complexity](http://en.cppreference.com/w/cpp/container/unordered_map/find), it's clearly slower than the best flat map implementations and is even slightly slower than std::map.
* Bad performance of P0460R0 flat map might be due to the std::string, const char* comparison issue discussed earlier.
* Pubby flat map is much slower than other flat maps.
* There's a rather significant difference between unsorted versions of MapVectorSoA and MapVectorAoS, this might be due to the better key locality in SoA-map.

 ### 3.2 Observations and notes

* find() is an example of an interface that can be inefficient by design by forcing creation of redundant object...
* ...but it can actually also be slower to use template type for find() instead of key_type as demonstrated by the performance characteristics of operator< (i.e. comparing two std::string's can be much faster than comparing std::string and const char* if the implementation recalculates length of const char* on each comparison). This is quite interesting: the C++14 template find() can actually also reduce performance depending on the map type and data.
* There are big differences between performances of flat map implementations reminding that the data structure is just one part of the puzzle.
* These results are for a specific type of data and run on a single compiler. Also the seemingly better results of MapVector compared to other flat_maps are the result of a 'std::string \& const char\*' -specific optimization: the effect depends on data.

## Miscellaneous

* Details on test machines (listed in raw results file in column 'Test machine')
    * 1: 
        * OS: TODO
        * CPU: TODO

