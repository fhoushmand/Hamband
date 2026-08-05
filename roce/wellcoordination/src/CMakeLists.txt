cmake_minimum_required( VERSION 3.9 )
project( band CXX )

include( CheckIPOSupported )
check_ipo_supported(RESULT supported OUTPUT error)

if( supported )
    set( CMAKE_INTERPROCEDURAL_OPTIMIZATION ${DORY_LTO} )
else()
    message( STATUS "IPO / LTO not supported: <${error}>" )
endif()

# --- MIN FIX (added): enforce C++17 (needed for make_unique + structured bindings) ---
set( CMAKE_CXX_STANDARD 17 )
# ------------------------------------------------------------------------------------

set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CONAN_SYSTEM_INCLUDES On )

include( ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake )
conan_basic_setup()

add_definitions( -DSPDLOG_ACTIVE_LEVEL=${SPDLOG_ACTIVE_LEVEL} )

add_compile_options(-pthread)

add_compile_options(-Wno-conversion)

# --- MIN FIX (added): don’t let Junction/Turf warnings stop the build ---
add_compile_options(-Wno-error=cast-qual -Wno-error=switch-default)
add_compile_options(-Wno-cast-qual -Wno-switch-default)
# -----------------------------------------------------------------------

MESSAGE( STATUS "CMAKE_C_FLAGS: " ${CMAKE_C_FLAGS} )
MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )
MESSAGE( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )
MESSAGE( STATUS "CMAKE_INTERPROCEDURAL_OPTIMIZATION: " ${CMAKE_INTERPROCEDURAL_OPTIMIZATION} )




# Add Junction (and Turf) build targets to the CMake project.
# This defines JUNCTION_ALL_INCLUDE_DIRS and JUNCTION_ALL_LIBRARIES.

add_subdirectory("../../junction" junction)
#add_subdirectory("../../turf" turf)

# Set include dirs and libraries for the executable.
include_directories(${JUNCTION_ALL_INCLUDE_DIRS})

#add_executable(
#    mu
#    mu-main.cpp
#)

add_executable(
    band
    band-main.cpp
)

#add_executable(
#    band-failure
#    band-main-failure.cpp
#)

add_executable(
    band-crdt
    band-main-crdt.cpp
)


target_link_libraries(band ${CONAN_LIBS} pthread)
#target_link_libraries(band-failure ${CONAN_LIBS} pthread)
target_link_libraries(band-crdt ${CONAN_LIBS} pthread)
#target_link_libraries(mu ${CONAN_LIBS} pthread)
