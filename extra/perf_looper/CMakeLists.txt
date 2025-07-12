cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME perf_looper)

project(${PROJECT_NAME} LANGUAGES CXX)

# Include Google Benchmark
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
add_subdirectory(deps/google_benchmark)

# Set up benchmark executable
add_executable(${PROJECT_NAME}
#    enum_stringify.h
    ../../MeshProc/data/HashableEdge.cpp
    ../../MeshProc/data/HashableEdge.h
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)
