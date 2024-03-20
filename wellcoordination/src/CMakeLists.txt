cmake_minimum_required( VERSION 3.9 )
project( band CXX )

include( CheckIPOSupported )
check_ipo_supported(RESULT supported OUTPUT error)

if( supported )
    set( CMAKE_INTERPROCEDURAL_OPTIMIZATION ${DORY_LTO} )
else()
    message( STATUS "IPO / LTO not supported: <${error}>" )
endif()

set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CONAN_SYSTEM_INCLUDES On )

include( ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake )
conan_basic_setup()

add_definitions( -DSPDLOG_ACTIVE_LEVEL=${SPDLOG_ACTIVE_LEVEL} )
add_definitions( -DCMAKE_TOOLCHAIN_FILE="/rhome/fhous001/farzin/vcpkg/scripts/buildsystems/vcpkg.cmake")

add_compile_options(-pthread)

add_compile_options(-Wno-conversion)

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
include_directories("/rhome/fhous001/farzin/vcpkg/packages/libcds_x64-linux/inlcude")



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

#add_executable(
#    band-crdt
#    band-main-crdt.cpp
#)


set(LibCDS_DIR "/rhome/fhous001/farzin/vcpkg/packages/libcds_x64-linux/share/libcds")
find_package(LibCDS CONFIG REQUIRED)

target_link_libraries(band ${CONAN_LIBS} pthread)
#target_link_libraries(band-failure ${CONAN_LIBS} pthread)
#target_link_libraries(band-crdt ${CONAN_LIBS} pthread junction turf LibCDS::cds-s)
#target_link_libraries(mu ${CONAN_LIBS} pthread)

