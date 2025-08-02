include_directories(include)
file(GLOB_RECURSE FQHLL_SOURCES src/*.cpp)

add_library(fq_hll STATIC ${FQHLL_SOURCES})

target_include_directories(fq_hll PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Public alias
add_library(fq_hll::fq_hll ALIAS fq_hll)