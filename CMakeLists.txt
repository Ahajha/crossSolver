cmake_minimum_required(VERSION 3.15)
project(crossSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(-W4 -WX)
else ()
    add_compile_options(-Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

add_library(bmp STATIC)
target_sources(bmp
PUBLIC
    src/bmp.hpp
PRIVATE
    src/bmp.cpp
)

add_library(cross_solver STATIC)
target_sources(cross_solver
PUBLIC
    src/index_generator.hpp
    src/nonagram.hpp
PRIVATE
    src/nonagram.cpp
)
target_link_libraries(cross_solver PUBLIC bmp)
target_compile_definitions(cross_solver PUBLIC "$<$<CONFIG:Debug>:CPUZZLE_DEBUG>")


add_executable(solve src/solver.cpp)
target_link_libraries(solve PRIVATE cross_solver)

add_executable(batch_solve src/batch_solver.cpp)
target_link_libraries(batch_solve PRIVATE cross_solver)
