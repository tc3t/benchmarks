for i in $(seq 1 5);
do
    ./mapSimpleInsertCmake_map_gcc_libstdcpp; sleep 1
    ./mapSimpleInsertCmake_map_clang_libcpp; sleep 1

    ./mapSimpleInsertCmake_unordered_map_reserved_gcc_libstdcpp; sleep 1
    ./mapSimpleInsertCmake_unordered_map_reserved_clang_libcpp; sleep 1

    ./mapSimpleInsertCmake_unordered_map_notreserved_gcc_libstdcpp; sleep 1
    ./mapSimpleInsertCmake_unordered_map_notreserved_clang_libcpp; sleep 1

    ./mapSimpleInsertCmake_boost_flat_map_gcc_libstdcpp; sleep 1
    ./mapSimpleInsertCmake_boost_flat_map_clang_libcpp; sleep 1

    ./mapSimpleInsertCmake_MapVectorSoA_gcc_libstdcpp; sleep 1
    ./mapSimpleInsertCmake_MapVectorSoA_clang_libcpp; sleep 1
    
    ./mapSimpleInsertCmake_MapVectorAoS_gcc_libstdcpp; sleep 1
    ./mapSimpleInsertCmake_MapVectorAoS_clang_libcpp; sleep 1
done
