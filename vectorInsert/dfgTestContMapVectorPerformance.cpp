#include <stdafx.h>

// Note: this file does not use DFG_CLASS_NAME-macros on purpose.

#include <dfg/typeTraits.hpp>
#include <dfg/cont/TrivialPair.hpp>

// This could be used to mark TrivialPair<int,int> as trivially copyable for compilers that do not support the type trait. 
// For now not marked to avoid giving impression that this optimization was available by default on those compilers.
#if 0 // !DFG_LANGFEAT_HAS_IS_TRIVIALLY_COPYABLE
DFG_ROOT_NS_BEGIN{ DFG_SUB_NS(TypeTraits)
{
    template <> struct IsTriviallyCopyable<DFG_MODULE_NS(cont)::TrivialPair<int,int>> : public std::true_type { };
} }
#endif

#include <dfg/build/compilerDetails.hpp>
#include <dfg/build/languageFeatureInfo.hpp>
#include <dfg/typeTraits.hpp>
#include <dfg/cont/tableCsv.hpp>
#include <dfg/cont/valuearray.hpp>
#include <dfg/str/strTo.hpp>

#include <dfg/cont/MapVector.hpp>
#include <dfg/cont/TrivialPair.hpp>
#include <dfg/cont/Vector.hpp>
#include <dfg/rand.hpp>
#include <dfg/str/format_fmt.hpp>
#include <dfg/time/timerCpu.hpp>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>

namespace
{

template <class T>
struct typeToName
{
    static std::string name() { return typeid(T).name(); }
};

template <> struct typeToName<int> { static std::string name() { return "int"; } };
template <> struct typeToName<double> { static std::string name() { return "double"; } };
template <> struct typeToName<std::string> { static std::string name() { return "std::string"; } };
template <class T0, class T1> struct typeToName<std::pair<T0, T1>> { static std::string name() { return "std::pair<" + typeToName<T0>::name() + ", " + typeToName<T1>::name() + ">"; } };
template <class T0, class T1> struct typeToName<DFG_MODULE_NS(cont)::TrivialPair<T0, T1>> { static std::string name() { return "TrivialPair<" + typeToName<T0>::name() + ", " + typeToName<T1>::name() + ">"; } };

template <class Key_T, class Val_T>
std::string containerDescription(const std::map<Key_T, Val_T>&) { return "std::map<" + typeToName<Key_T>::name() + "," + typeToName<Val_T>::name() + ">"; }

template <class Key_T, class Val_T>
std::string containerDescription(const std::unordered_map<Key_T, Val_T>&) { return "std::unordered_map<" + typeToName<Key_T>::name() + "," + typeToName<Val_T>::name() + ">"; }

template <class Key_T, class Val_T>
std::string containerDescription(const boost::container::flat_map<Key_T, Val_T>&) { return "boost::flat_map<" + typeToName<Key_T>::name() + "," + typeToName<Val_T>::name() + ">"; }
template <class Key_T, class Val_T>
std::string containerDescription(const DFG_MODULE_NS(cont)::MapVectorAoS<Key_T, Val_T>& cont)
{
    return DFG_ROOT_NS::format_fmt("MapVectorAoS<{},{}>, sorted: {}", typeToName<Key_T>::name(), typeToName<Val_T>::name(), int(cont.isSorted()));
}
template <class Key_T, class Val_T>
std::string containerDescription(const DFG_MODULE_NS(cont)::MapVectorSoA<Key_T, Val_T>& cont)
{
    return DFG_ROOT_NS::format_fmt("MapVectorSoA<{},{}>, sorted: {}", typeToName<Key_T>::name(), typeToName<Val_T>::name(), int(cont.isSorted()));
}

template <class Val_T>
std::string containerDescription(const DFG_MODULE_NS(cont)::Vector<Val_T>&)
{
    return DFG_ROOT_NS::format_fmt("Vector<{}>", typeToName<Val_T>::name());
}

template <class Val_T> std::string containerDescription(const std::vector<Val_T>&) { return "std::vector<" + typeToName<Val_T>::name() + ">"; }
template <class Val_T> std::string containerDescription(const boost::container::vector<Val_T>&) { return "boost::vector<" + typeToName<Val_T>::name() + ">"; }

namespace
{
    class BenchmarkResultTable : public DFG_MODULE_NS(cont)::TableCsv<char, DFG_ROOT_NS::uint32>
    {
        typedef DFG_ROOT_NS::uint32 uint32;

    public:
        void addReducedValues(const uint32 firstResultCol)
        {
            using namespace DFG_ROOT_NS;
            using namespace DFG_MODULE_NS(str);

            const auto nRowCount = this->rowCountByMaxRowIndex();
            const auto nColCount = this->colCountByMaxColIndex();

            this->addString(DFG_ASCII("avg"),       0, nColCount);
            this->addString(DFG_ASCII("median"),    0, nColCount + 1);
            this->addString(DFG_ASCII("sum"),       0, nColCount + 2);
            
            DFG_MODULE_NS(cont)::ValueVector<double> vals;
            for (uint32 r = 1; r < nRowCount; ++r)
            {
                vals.clear();
                for (uint32 c = firstResultCol; c < nColCount; ++c)
                {
                    auto p = (*this)(r, c);
                    if (!p)
                        continue;
                    vals.push_back(DFG_MODULE_NS(str)::strTo<double>(p.rawPtr()));
                }

                const auto avg = vals.average();
                const auto median = vals.median();
                const auto sum = vals.sum();

                char sz[32];
                this->addString(SzPtrUtf8(toStr(avg, sz, 6)), r, nColCount);
                this->addString(SzPtrUtf8(toStr(median, sz, 6)), r, nColCount + 1);
                this->addString(SzPtrUtf8(toStr(sum, sz, 6)), r, nColCount + 2);
            }
        }
    };

    std::string generateCompilerInfoForOutputFilename()
    {
        return dfg::format_fmt("{}_{}_{}", DFG_COMPILER_NAME_SIMPLE, 8 * sizeof(void*), DFG_BUILD_DEBUG_RELEASE_TYPE);
    }

    template <class T>
    std::string GenerateOutputFilePathForVectorInsert(const dfg::StringViewSzC& s)
    {
        std::string sTypeSuffix = typeToName<T>::name();
        for (size_t j = 0; j < sTypeSuffix.size(); ++j)
        {
            auto ch = sTypeSuffix[j];
            if (ch == '<' || ch == '>' || ch == ',' || ch == ':')
                sTypeSuffix[j] = '_';
        }
        return dfg::format_fmt("testfiles/generated/{}{}_{}.txt", s.c_str(), sTypeSuffix, generateCompilerInfoForOutputFilename());
    }

} // unnamed namespace

template <class Cont_T, class Generator_T, class InsertPosGenerator_T>
Cont_T VectorInsertImpl(Generator_T generator, InsertPosGenerator_T indexGenerator, const int nCount, BenchmarkResultTable* pTable, const int nRow)
{
    using namespace DFG_ROOT_NS;
    using namespace DFG_MODULE_NS(str);

    Cont_T cont;
    DFG_MODULE_NS(time)::TimerCpu timer;
    cont.reserve(nCount);
    cont.push_back(generator(1));
    for (int i = 1; i < nCount; ++i)
    {
        const auto nPos = indexGenerator(cont.size());
        cont.insert(cont.begin() + nPos, generator(nPos));
    }
    const auto elapsedTime = timer.elapsedWallSeconds();
    //const auto sReservationInfo = (capacity != NumericTraits<size_t>::maxValue) ? format_fmt(", reserved: {}", int(capacity >= cont.size())) : "";
    if (pTable)
        pTable->addString(floatingPointToStr<StringUtf8>(elapsedTime, 4 /*number of significant digits*/), nRow, pTable->colCountByMaxColIndex() - 1);

    if (nCount > 100)
        std::cout << "Insert time " << containerDescription(cont) /*<< sReservationInfo*/ << ": " << elapsedTime << '\n';
    return cont;
}

template <class T> T generate(size_t randVal);

template <> int generate<int>(size_t randVal) { return static_cast<int>(randVal); }
template <> double generate<double>(size_t randVal) { return static_cast<double>(randVal); }
template <> std::pair<int, int> generate<std::pair<int, int>>(size_t randVal)
{
    auto val = generate<int>(randVal);
    return std::pair<int, int>(val, val);
}

template <> DFG_MODULE_NS(cont)::TrivialPair<int, int> generate<DFG_MODULE_NS(cont)::TrivialPair<int, int>>(size_t randVal)
{
    auto val = generate<int>(randVal);
    return DFG_MODULE_NS(cont)::TrivialPair<int, int>(val, val);
}

template <class Pair_T>
std::ostream& pairLikeItemStreaming(std::ostream& ostrm, const Pair_T& a)
{
    ostrm << a.first << "," << a.second;
    return ostrm;
}

template <class T0, class T1>
std::ostream& operator<<(std::ostream& ostrm, const std::pair<T0, T1>& a)
{
    return pairLikeItemStreaming(ostrm, a);
}

template <class T0, class T1>
std::ostream& operator<<(std::ostream& ostrm, const dfg::cont::TrivialPair<T0, T1>& a)
{
    return pairLikeItemStreaming(ostrm, a);
}

template <class T>
void VectorInsertImpl(const int nCount, BenchmarkResultTable* pTable = nullptr, const int nRow = 0, const int nTypeCol = 0)
{
    using namespace DFG_ROOT_NS;
    if (pTable)
    {
        pTable->addString(SzPtrUtf8(containerDescription(std::vector<T>()).c_str()), nRow, nTypeCol);
        pTable->addString(SzPtrUtf8(containerDescription(boost::container::vector<T>()).c_str()), nRow + 1, nTypeCol);
        pTable->addString(SzPtrUtf8(containerDescription(DFG_MODULE_NS(cont)::Vector<T>()).c_str()), nRow + 2, nTypeCol);
    }

#if 1 // If true, using file-based insert positions.
    std::vector<int> contInsertIndexes;
    contInsertIndexes.reserve(50000);
    std::ifstream istrm("testfiles/vectorInsertIndexes_50000.txt");
    {
        int i;
        while (istrm >> i)
        {
            contInsertIndexes.push_back(i);
        }
    }

    const auto indexGenerator = [&](const size_t nContSize)
                                {
                                    return contInsertIndexes[(nContSize - 1) % contInsertIndexes.size()];
                                };
#else // Case: using random generator based insert positions.
    const unsigned long nRandEngSeed = 12345678;
    auto randEng = DFG_MODULE_NS(rand)::createDefaultRandEngineUnseeded();
    auto distr = DFG_MODULE_NS(rand)::makeDistributionEngineUniform(&randEng, 0, NumericTraits<int>::maxValue);
    const auto indexGenerator = [&](const size_t nContSize) -> ptrdiff_t
                                {
                                    if (nContSize == 1)
                                        randEng.seed(nRandEngSeed);
                                    return distr() % nContSize;
                                };
#endif

    const auto stdVec = VectorInsertImpl<std::vector<T>>(generate<T>, indexGenerator, nCount, pTable, nRow);
    const auto boostVec = VectorInsertImpl<boost::container::vector<T>>(generate<T>, indexGenerator, nCount, pTable, nRow + 1);
    const auto dfgVec = VectorInsertImpl<DFG_MODULE_NS(cont)::Vector<T>>(generate<T>, indexGenerator, nCount, pTable, nRow + 2);
    ASSERT_EQ(nCount, stdVec.size());
    ASSERT_EQ(stdVec.size(), boostVec.size());
    ASSERT_EQ(stdVec.size(), dfgVec.size());

    EXPECT_TRUE(std::equal(stdVec.begin(), stdVec.end(), boostVec.begin()));
    EXPECT_TRUE(std::equal(stdVec.begin(), stdVec.end(), dfgVec.begin()));
    
    if (pTable)
    {
        dfg::io::OfStream ostrm(GenerateOutputFilePathForVectorInsert<T>("generatedVectorInsertValues"));
        for (size_t i = 0; i < stdVec.size(); ++i)
            ostrm << stdVec[i] << '\n';
    }
}

} // unnamed namespace

#if 1 // On/off switch for the whole performance test.

namespace
{
    int generateKey(decltype(DFG_MODULE_NS(rand)::createDefaultRandEngineUnseeded())& re)
    {
        return DFG_MODULE_NS(rand)::rand<int>(re, -10000000, 10000000);
    }

    int generateValue(decltype(DFG_MODULE_NS(rand)::createDefaultRandEngineUnseeded())& re)
    {
        return DFG_MODULE_NS(rand)::rand<int>(re, -10000000, 10000000);
    }

    template <class Cont_T>
    void insertImpl(Cont_T& cont, decltype(DFG_MODULE_NS(rand)::createDefaultRandEngineUnseeded())& re)
    {
        auto key = generateKey(re);
        cont.insert(std::pair<int, int>(key, key));
    }
}

#include <dfg/io/ofstream.hpp>
#include <dfg/time.hpp>
#include <dfg/time/DateTime.hpp>
#include <dfg/str/string.hpp>
#include <dfg/str.hpp>

TEST(dfgCont, VectorInsertPerformance)
{
    using namespace DFG_ROOT_NS;
    using namespace DFG_MODULE_NS(str);

#ifdef _DEBUG
    const int nCount = 100;
#else
    const int nCount = 50000;
#endif

    BenchmarkResultTable table;
    table.addString(SzPtrUtf8("Date"), 0, 0);
    table.addString(SzPtrUtf8("Test machine"), 0, 1);
    table.addString(SzPtrUtf8("Test Compiler"), 0, 2);
    table.addString(SzPtrUtf8("Pointer size"), 0, 3);
    table.addString(SzPtrUtf8("Build type"), 0, 4);
    table.addString(DFG_UTF8("Insert count"), 0, 5);
    table.addString(SzPtrUtf8("Test type"), 0, 6);
    const auto nLastStaticColumn = 6;

    const auto nElementTypeCount = 4;
    const auto nContainerCount = 3;

    for (size_t i = 0; i < 5; ++i) // Iterations.
    {
        if (i == 0)
        {
            const StringUtf8 sTime(SzPtrUtf8(DFG_MODULE_NS(time)::localDate_yyyy_mm_dd_C().c_str()));
            const auto sCompiler = SzPtrUtf8(DFG_COMPILER_NAME_SIMPLE);
            const StringUtf8 sPointerSize(SzPtrUtf8(DFG_MODULE_NS(str)::toStrC(sizeof(void*)).c_str()));
            const auto sBuildType = SzPtrUtf8(DFG_BUILD_DEBUG_RELEASE_TYPE);
            const StringUtf8 sInsertCount(SzPtrUtf8(DFG_MODULE_NS(str)::toStrC(nCount).c_str()));
            for (int et = 1; et <= nElementTypeCount; ++et)
            {
                for (int ct = 0; ct < nContainerCount; ++ct)
                {
                    const auto r = table.rowCountByMaxRowIndex();
                    table.addString(sTime, r, 0);
                    table.addString(sCompiler, r, 2);
                    table.addString(sPointerSize, r, 3);
                    table.addString(sBuildType, r, 4);
                    table.addString(sInsertCount, r, 5);
                }
            }
        }

        table.addString(SzPtrUtf8(("Time#" + toStrC(i)).c_str()), 0, table.colCountByMaxColIndex());
        VectorInsertImpl<int>(nCount, &table, 1, nLastStaticColumn);
        VectorInsertImpl<double>(nCount, &table, 1 + 1 * nContainerCount, nLastStaticColumn);
        VectorInsertImpl<std::pair<int, int>>(nCount, &table, 1 + 2 * nContainerCount, nLastStaticColumn);
        VectorInsertImpl<DFG_MODULE_NS(cont)::TrivialPair<int, int>>(nCount, &table, 1 + 3 * nContainerCount, nLastStaticColumn);
    }

    // Calculate averages etc.
    table.addReducedValues(nLastStaticColumn + 1);

    DFG_MODULE_NS(io)::OfStream ostrm(format_fmt("testfiles/generated/benchmarkVectorInsert_{}.csv", generateCompilerInfoForOutputFilename()));
    table.writeToStream(ostrm);
}

#endif // on/off switch for performance tests.

