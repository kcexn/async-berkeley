# Compile benchmarks.
find_package(Boost REQUIRED)

set(BENCHMARK_NAMES echo_benchmark)

foreach(BENCHMARK_NAME IN LISTS BENCHMARK_NAMES)
  add_executable(${BENCHMARK_NAME} ${BENCHMARK_NAME}.cpp)

  target_include_directories(${BENCHMARK_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

  target_link_libraries(${BENCHMARK_NAME} PRIVATE asyncberk benchmark)
endforeach()
