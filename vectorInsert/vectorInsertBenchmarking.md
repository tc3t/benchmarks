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


#### Key Observations
* There are massive differences between std::vector<T>::insert() performance between tested compilers, for example with std::vector\<int\>, runtime ratio between VC2013 and MinGW was about 4.5 (i.e. MinGW over 4 times faster than VC2013)
    * insert() in tested VC versions, which is based on std::rotate, seems slow. Note though that it seems that the implementation has changed to VC2017 RC making it as fast as boost::vector.
* In tested VC-compilers, insert-performance for trivial types can be highly improved by using a simple memmove() implementation: insert with dfg::Vector\<int\> (that uses memmove-insert) was about 5x faster than std::vector\<int\> on VC2015.
* Using TrivialPair\<int,int\> instead of std::pair\<int,int\> could be used to improve runtime performance by factor of 2 or more in most cases in MSVC versions. The relevant difference is that the two have different value for std::is_trivially_copyable -type trait, which in turn may affect whether insert() is implementated with std::memmove() or not.


## Benchmark implementation

The detailed  test code can be found from [here](dfgTestContMapVectorPerformance.cpp), but essentially the code measured the time taken by the following loop:
```C++
for (int i = 0; i < 50000; ++i)
    cont.insert(cont.begin() + randomIndex, randomElement);
```

The random indexes were read from a file (i.e. were the same for all implementations, the list was initially generated with a random generator). The randomElement was the same as the random index for int and double and [randomIndex, randomIndex] for pairs. This was run 5 times and when a single value is referred to, it's median time of these runs. Values of the contructed containers were printed to file and manually verified that they were identical.

## Results

The following figures show runtimes in various tests cases (the lower the faster). Each test was run 5 times so there are 5 points for each implementation giving some indication of the variance. The raw result table can be found from [here](benchmarkVectorInsert.csv).

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