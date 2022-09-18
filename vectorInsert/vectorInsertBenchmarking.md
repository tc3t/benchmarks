# Benchmarking vector insert

## General

While introducing [vector-based maps](https://github.com/tc3t/dfglib/tree/master/dfg/cont/MapVector.hpp) to dfglib and checking their performance, one of the aspect that quickly turned to be significant was performance of insert() to random positions of underlying vector implementation. This document presents some results from testing insert() performance with std::vector, boost::vector and dfglib::Vector.

### Disclaimer

Performance benchmarking is a tricky field and while the results provided are made with the intention of being correct enough to aid evaluating the performance in some special use cases, the reader should be aware that the author can not guarantee the accuracy of the results or even the methological correctness. If you intend to make decision based on the results presented here, it's strongly recommended to personally verify the used test code. If flaws are detected, reports and corrections would be highly appreciated.

### Short summary

The benchmark does 50000 inserts to random positions starting from empty vector using:
* Containers
    * std::vector
    * boost::vector
    * dfglib:::Vector
* Element types
    * int
    * double
    * std::pair<int, int>
    * dfglib::TrivialPair<int, int>
* Compilers
    * VC2010 (32bit)
    * VC2012 (update 4) (32 & 64 bit)
    * VC2013 (update 5) (32 & 64 bit)
    * VC2015 (update 3) (32 & 64 bit)
    * MinGW 4.8.0
    * [Update 2022-09-17](#update-2022-09-17) has notes about more recent MSVC compilers


#### Key Observations
* There are massive differences between std::vector<T>::insert() performance between tested compilers, for example with std::vector\<int\>, runtime ratio between VC2013 and MinGW was about 4.5 (i.e. MinGW over 4 times faster than VC2013)
    * insert() in tested VC versions, which is based on std::rotate, seems slow. Note though that it seems that the implementation has changed to VC2017 RC making it as fast as boost::vector.
* In tested VC-compilers, insert-performance for trivial types can be highly improved by using a simple memmove() implementation: insert with dfg::Vector\<int\> (that uses memmove-insert) was about 5x faster than std::vector\<int\> on VC2015.
* Using TrivialPair\<int,int\> instead of std::pair\<int,int\> could be used to improve runtime performance by factor of 2 or more in most cases in MSVC versions. The relevant difference is that the two have different value for std::is_trivially_copyable -type trait, which in turn may affect whether insert() is implementated with std::memmove() or not.
* Update 2022-09-17: major performance difference remain between std::pair\<int,int\> and "trivial pair", for details see [Update 2022-09-17](#update-2022-09-17)


## Benchmark implementation

The detailed  test code, that uses revision [0b4ca6761e3757516d81b724385cd0cc6d2a9988](https://github.com/tc3t/dfglib/tree/0b4ca6761e3757516d81b724385cd0cc6d2a9988) of dfglib, can be found from [here](dfgTestContMapVectorPerformance.cpp), but essentially the code measured the time taken to run the following code:
```C++
for (int i = 0; i < 50000; ++i)
    cont.insert(cont.begin() + randomIndex, randomElement);
```

The random indexes were read from a file (i.e. were the same for all implementations, the list was initially generated with a random generator). The randomElement was the same as the random index for int and double and [randomIndex, randomIndex] for pairs. This was run 5 times and when a single value is referred to, it's median time of these runs. Values of the contructed containers were printed to file and manually verified that they were identical.

## Results

The following figures show run times in various tests cases (the lower the faster). Each test was run 5 times so there are 5 points for each implementation giving some indication of the variance. The raw result table can be found from [here](benchmarkVectorInsert.csv).

<br>

![alt text][std_vector_int]

MinGW is clearly the fastest, a notable improvement from VC2013 -> 2015.

<br>
<br>


![alt text][std_vector_double]

MinGW and VC2015 approximate on par, earlier VC versions clearly slower.

<br>
<br>

![alt text][std_vector_stdpair_int_int]

Again, MinGW clearly the fastest and VC2015 not as slow as previous versions.

<br>
<br>

![alt text][std_vector_TrivialPair_int_int]

Similar to previous, but here it can be seen that 64-bit MSVC versions are faster than 32-bit versions, but still slower or not faster than MinGW.

<br>
<br>

![alt text][container_comparison_intdouble_MinGW_4.8.0]

With MinGW, no differences between container types when using int or double as element type.

<br>
<br>

![alt text][container_comparison_intdouble_MSVC_2015]

std::vector is clearly slower on MSVC 2015 (32-bit) with int and double compared to boost::vector and dfglib::Vector. 

<br>
<br>

![alt text][container_comparison_pair_MinGW_4.8.0]

Like in the previous MinGW one, no differences between container types when element type is std::pair\<int,int\> or dfglib::TrivialPair\<int,int\>.

<br>
<br>

![alt text][container_comparison_pair_MSVC_2015]

These results are quite interesting given the big differences between times but little differences in the element types. Also another thing worth pointing out that while MinGW was often clearly the fastest and never slower than MSVC 2015 in the std::vector comparisons, with TrivialPair and boost::vector or dfglib::Vector MSVC 2015 (32-bit) seems faster than MinGW by factor of over 2.

[std_vector_int]: std__vector_int_.png
[std_vector_double]: std__vector_double_.png
[std_vector_stdpair_int_int]: std__vector_std__pair_int__int__.png
[std_vector_TrivialPair_int_int]: std__vector_TrivialPair_int__int__.png
[container_comparison_intdouble_MinGW_4.8.0]: container_comparison_intdouble_MinGW_4.8.0.png
[container_comparison_intdouble_MSVC_2015]: container_comparison_intdouble_MSVC_2015.png
[container_comparison_pair_MinGW_4.8.0]: container_comparison_pair_MinGW_4.8.0.png
[container_comparison_pair_MSVC_2015]: container_comparison_pair_MSVC_2015.png

## Discussion

Results showed big runtime differences between compilers and container types.
* Implementation of insert() using memmove() was over 5 times faster than std::vector\<int\>::insert() in VC2015 (32-bit).
* Using TrivialPair instead of std::pair improved insert() performance with boost::vector by factor ~2 in VC2015 (32-bit).
* With MinGW, container or pair type had no effect on the results, reason is unknown.
* With std::vector, the fastest implementation seemed to be MinGW, but fastest insert times were obtained in MSVC 2015 using either boost::vector or dfglib::Vector.

It's worth mentioning that VC2017 RC has introduced changes to std::vector::insert implementation resulting to std::vector being as fast as other container types (in resolution of these comparisons). However the difference between std::pair and TrivialPair remains making it a notable implementation detail when e.g. implementing AoS-style flat maps.

## Update 2022-09-17

Since MSVC2017, std::is_trivially_copyable_v<std::pair<int,int>> returns true, but the major difference remains in insert times: with simple insert [test code](insertTest.cpp)

    struct SimplePair
    {
        SimplePair() = default;
        SimplePair(int a, int b) : first(a), second(b) {}
        int first = 0;
        int second = 0;
    };
    ...
    using Pair = (std::pair<int, int> / std::tuple<int,int> / SimplePair)
    std::vector<Pair> cont;
    cont.reserve(100000);
    for (int i = 0; i < 100000; ++i)
        cont.insert(cont.begin(), Pair(i, i)); // Insert to beginning

on MSVC2022 (version 17.3) std::pair\<int, int\> and std::tuple\<int, int\> takes about twice the time compared to SimplePair (in a test computer typical run times were around 3.3 s for std::pair/tuple and 1.6 s for SimplePair, compiled with _cl insertTest.cpp /O2 /EHsc /std:c++17_). This seems to be caused by memmove-implementation getting used only for SimplePair: usage is determined in  [_Move_backward_unchecked()](https://github.com/microsoft/STL/blob/ac129e595f762f11551663f1c7fa5f51444a8c6c/stl/inc/xutility#L4258) (which gets called from stack vector::emplace() <- vector::insert()) using internal trait [_Bitcopy_assignable](https://github.com/microsoft/STL/blob/ac129e595f762f11551663f1c7fa5f51444a8c6c/stl/inc/xutility#L4261).

Some trait values for std::pair\<int,int\>, std::tuple\<int, int\> and SimplePair from MSVC2022

    Traits for struct std::pair<int,int>
    is_trivially_copyable              = 1
    is_trivially_default_constructible = 0
    is_trivially_destructible          = 1
    is_nothrow_constructible           = 1
    is_nothrow_move_assignable         = 1
    is_trivially_assignable<T,T>       = 0
    is_trivially_move_assignable       = 0
    is_trivially_copy_assignable       = 0

    Traits for class std::tuple<int,int>
    is_trivially_copyable              = 0
    is_trivially_default_constructible = 0
    is_trivially_destructible          = 1
    is_nothrow_constructible           = 1
    is_nothrow_move_assignable         = 1
    is_trivially_assignable<T,T>       = 0
    is_trivially_move_assignable       = 0
    is_trivially_copy_assignable       = 0

    Traits for struct SimplePair
    is_trivially_copyable              = 1
    is_trivially_default_constructible = 0
    is_trivially_destructible          = 1
    is_nothrow_constructible           = 1
    is_nothrow_move_assignable         = 1
    is_trivially_assignable<T,T>       = 1
    is_trivially_move_assignable       = 1
    is_trivially_copy_assignable       = 1
