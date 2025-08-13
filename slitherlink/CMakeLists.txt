add_library(slitherlink_solver STATIC)
target_sources(slitherlink_solver
PUBLIC
    slitherlink.hpp
PRIVATE
    slitherlink.cpp
)
