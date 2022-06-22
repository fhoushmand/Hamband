include(CMakeParseArguments)

macro(conan_find_apple_frameworks FRAMEWORKS_FOUND FRAMEWORKS SUFFIX BUILD_TYPE)
    if(APPLE)
        if(CMAKE_BUILD_TYPE)
            set(_BTYPE ${CMAKE_BUILD_TYPE})
        elseif(NOT BUILD_TYPE STREQUAL "")
            set(_BTYPE ${BUILD_TYPE})
        endif()
        if(_BTYPE)
            if(${_BTYPE} MATCHES "Debug|_DEBUG")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_DEBUG} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_DEBUG} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            elseif(${_BTYPE} MATCHES "Release|_RELEASE")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_RELEASE} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_RELEASE} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            elseif(${_BTYPE} MATCHES "RelWithDebInfo|_RELWITHDEBINFO")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_RELWITHDEBINFO} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_RELWITHDEBINFO} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            elseif(${_BTYPE} MATCHES "MinSizeRel|_MINSIZEREL")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_MINSIZEREL} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_MINSIZEREL} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            endif()
        endif()
        foreach(_FRAMEWORK ${FRAMEWORKS})
            # https://cmake.org/pipermail/cmake-developers/2017-August/030199.html
            find_library(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND NAME ${_FRAMEWORK} PATHS ${CONAN_FRAMEWORK_DIRS${SUFFIX}} CMAKE_FIND_ROOT_PATH_BOTH)
            if(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND)
                list(APPEND ${FRAMEWORKS_FOUND} ${CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND})
            else()
                message(FATAL_ERROR "Framework library ${_FRAMEWORK} not found in paths: ${CONAN_FRAMEWORK_DIRS${SUFFIX}}")
            endif()
        endforeach()
    endif()
endmacro()


#################
###  DORY-MEMORY
#################
set(CONAN_DORY-MEMORY_ROOT "/rhome/fhous001/.conan/data/dory-memory/0.0.1/_/_/package/4949b8a042c8a276fecbdf742b8ffc951aa97499")
set(CONAN_INCLUDE_DIRS_DORY-MEMORY "/rhome/fhous001/.conan/data/dory-memory/0.0.1/_/_/package/4949b8a042c8a276fecbdf742b8ffc951aa97499/include")
set(CONAN_LIB_DIRS_DORY-MEMORY "/rhome/fhous001/.conan/data/dory-memory/0.0.1/_/_/package/4949b8a042c8a276fecbdf742b8ffc951aa97499/lib")
set(CONAN_BIN_DIRS_DORY-MEMORY )
set(CONAN_RES_DIRS_DORY-MEMORY )
set(CONAN_SRC_DIRS_DORY-MEMORY )
set(CONAN_BUILD_DIRS_DORY-MEMORY "/rhome/fhous001/.conan/data/dory-memory/0.0.1/_/_/package/4949b8a042c8a276fecbdf742b8ffc951aa97499/")
set(CONAN_FRAMEWORK_DIRS_DORY-MEMORY )
set(CONAN_LIBS_DORY-MEMORY dorymem)
set(CONAN_PKG_LIBS_DORY-MEMORY dorymem)
set(CONAN_SYSTEM_LIBS_DORY-MEMORY )
set(CONAN_FRAMEWORKS_DORY-MEMORY )
set(CONAN_FRAMEWORKS_FOUND_DORY-MEMORY "")  # Will be filled later
set(CONAN_DEFINES_DORY-MEMORY )
set(CONAN_BUILD_MODULES_PATHS_DORY-MEMORY )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_DORY-MEMORY )

set(CONAN_C_FLAGS_DORY-MEMORY "")
set(CONAN_CXX_FLAGS_DORY-MEMORY "-Wconversion -Wfloat-equal -Wpedantic -Wpointer-arith -Wswitch-default -Wpacked -Wextra -Winvalid-pch -Wmissing-field-initializers -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wuninitialized -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wstrict-overflow=5 -Wno-unused -Wno-conversion -Wctor-dtor-privacy -Wsign-promo -Woverloaded-virtual -Wlogical-op -Wstrict-null-sentinel -Wnoexcept -O0 -Winline -g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY "")
set(CONAN_EXE_LINKER_FLAGS_DORY-MEMORY "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_DORY-MEMORY_LIST "")
set(CONAN_CXX_FLAGS_DORY-MEMORY_LIST "-Wconversion;-Wfloat-equal;-Wpedantic;-Wpointer-arith;-Wswitch-default;-Wpacked;-Wextra;-Winvalid-pch;-Wmissing-field-initializers;-Wunreachable-code;-Wcast-align;-Wcast-qual;-Wdisabled-optimization;-Wformat=2;-Wformat-nonliteral;-Wuninitialized;-Wformat-security;-Wformat-y2k;-Winit-self;-Wmissing-declarations;-Wmissing-include-dirs;-Wredundant-decls;-Wstrict-overflow=5;-Wno-unused;-Wno-conversion;-Wctor-dtor-privacy;-Wsign-promo;-Woverloaded-virtual;-Wlogical-op;-Wstrict-null-sentinel;-Wnoexcept;-O0;-Winline;-g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_LIST "")
set(CONAN_EXE_LINKER_FLAGS_DORY-MEMORY_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_DORY-MEMORY "${CONAN_FRAMEWORKS_DORY-MEMORY}" "_DORY-MEMORY" "")
# Append to aggregated values variable
set(CONAN_LIBS_DORY-MEMORY ${CONAN_PKG_LIBS_DORY-MEMORY} ${CONAN_SYSTEM_LIBS_DORY-MEMORY} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMORY})


#################
###  DORY-CONNECTION
#################
set(CONAN_DORY-CONNECTION_ROOT "/rhome/fhous001/.conan/data/dory-connection/0.0.1/_/_/package/ef4fb424620497e1e0b7f0186a98dec0be1eeabe")
set(CONAN_INCLUDE_DIRS_DORY-CONNECTION "/rhome/fhous001/.conan/data/dory-connection/0.0.1/_/_/package/ef4fb424620497e1e0b7f0186a98dec0be1eeabe/include")
set(CONAN_LIB_DIRS_DORY-CONNECTION "/rhome/fhous001/.conan/data/dory-connection/0.0.1/_/_/package/ef4fb424620497e1e0b7f0186a98dec0be1eeabe/lib")
set(CONAN_BIN_DIRS_DORY-CONNECTION )
set(CONAN_RES_DIRS_DORY-CONNECTION )
set(CONAN_SRC_DIRS_DORY-CONNECTION )
set(CONAN_BUILD_DIRS_DORY-CONNECTION "/rhome/fhous001/.conan/data/dory-connection/0.0.1/_/_/package/ef4fb424620497e1e0b7f0186a98dec0be1eeabe/")
set(CONAN_FRAMEWORK_DIRS_DORY-CONNECTION )
set(CONAN_LIBS_DORY-CONNECTION doryconn)
set(CONAN_PKG_LIBS_DORY-CONNECTION doryconn)
set(CONAN_SYSTEM_LIBS_DORY-CONNECTION )
set(CONAN_FRAMEWORKS_DORY-CONNECTION )
set(CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION "")  # Will be filled later
set(CONAN_DEFINES_DORY-CONNECTION )
set(CONAN_BUILD_MODULES_PATHS_DORY-CONNECTION )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_DORY-CONNECTION )

set(CONAN_C_FLAGS_DORY-CONNECTION "")
set(CONAN_CXX_FLAGS_DORY-CONNECTION "-Wconversion -Wfloat-equal -Wpedantic -Wpointer-arith -Wswitch-default -Wpacked -Wextra -Winvalid-pch -Wmissing-field-initializers -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wuninitialized -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wstrict-overflow=5 -Wno-unused -Wno-conversion -Wctor-dtor-privacy -Wsign-promo -Woverloaded-virtual -Wlogical-op -Wstrict-null-sentinel -Wnoexcept -O0 -Winline -g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION "")
set(CONAN_EXE_LINKER_FLAGS_DORY-CONNECTION "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_DORY-CONNECTION_LIST "")
set(CONAN_CXX_FLAGS_DORY-CONNECTION_LIST "-Wconversion;-Wfloat-equal;-Wpedantic;-Wpointer-arith;-Wswitch-default;-Wpacked;-Wextra;-Winvalid-pch;-Wmissing-field-initializers;-Wunreachable-code;-Wcast-align;-Wcast-qual;-Wdisabled-optimization;-Wformat=2;-Wformat-nonliteral;-Wuninitialized;-Wformat-security;-Wformat-y2k;-Winit-self;-Wmissing-declarations;-Wmissing-include-dirs;-Wredundant-decls;-Wstrict-overflow=5;-Wno-unused;-Wno-conversion;-Wctor-dtor-privacy;-Wsign-promo;-Woverloaded-virtual;-Wlogical-op;-Wstrict-null-sentinel;-Wnoexcept;-O0;-Winline;-g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_LIST "")
set(CONAN_EXE_LINKER_FLAGS_DORY-CONNECTION_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION "${CONAN_FRAMEWORKS_DORY-CONNECTION}" "_DORY-CONNECTION" "")
# Append to aggregated values variable
set(CONAN_LIBS_DORY-CONNECTION ${CONAN_PKG_LIBS_DORY-CONNECTION} ${CONAN_SYSTEM_LIBS_DORY-CONNECTION} ${CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION})


#################
###  DORY-CTRL
#################
set(CONAN_DORY-CTRL_ROOT "/rhome/fhous001/.conan/data/dory-ctrl/0.0.1/_/_/package/fb4ac4a542d099bfa0033d44442128dcb9054402")
set(CONAN_INCLUDE_DIRS_DORY-CTRL "/rhome/fhous001/.conan/data/dory-ctrl/0.0.1/_/_/package/fb4ac4a542d099bfa0033d44442128dcb9054402/include")
set(CONAN_LIB_DIRS_DORY-CTRL "/rhome/fhous001/.conan/data/dory-ctrl/0.0.1/_/_/package/fb4ac4a542d099bfa0033d44442128dcb9054402/lib")
set(CONAN_BIN_DIRS_DORY-CTRL )
set(CONAN_RES_DIRS_DORY-CTRL )
set(CONAN_SRC_DIRS_DORY-CTRL )
set(CONAN_BUILD_DIRS_DORY-CTRL "/rhome/fhous001/.conan/data/dory-ctrl/0.0.1/_/_/package/fb4ac4a542d099bfa0033d44442128dcb9054402/")
set(CONAN_FRAMEWORK_DIRS_DORY-CTRL )
set(CONAN_LIBS_DORY-CTRL doryctrl)
set(CONAN_PKG_LIBS_DORY-CTRL doryctrl)
set(CONAN_SYSTEM_LIBS_DORY-CTRL ibverbs)
set(CONAN_FRAMEWORKS_DORY-CTRL )
set(CONAN_FRAMEWORKS_FOUND_DORY-CTRL "")  # Will be filled later
set(CONAN_DEFINES_DORY-CTRL )
set(CONAN_BUILD_MODULES_PATHS_DORY-CTRL )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_DORY-CTRL )

set(CONAN_C_FLAGS_DORY-CTRL "")
set(CONAN_CXX_FLAGS_DORY-CTRL "-Wconversion -Wfloat-equal -Wpedantic -Wpointer-arith -Wswitch-default -Wpacked -Wextra -Winvalid-pch -Wmissing-field-initializers -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wuninitialized -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wstrict-overflow=5 -Wno-unused -Wno-conversion -Wctor-dtor-privacy -Wsign-promo -Woverloaded-virtual -Wlogical-op -Wstrict-null-sentinel -Wnoexcept -O0 -Winline -g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-CTRL "")
set(CONAN_EXE_LINKER_FLAGS_DORY-CTRL "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_DORY-CTRL_LIST "")
set(CONAN_CXX_FLAGS_DORY-CTRL_LIST "-Wconversion;-Wfloat-equal;-Wpedantic;-Wpointer-arith;-Wswitch-default;-Wpacked;-Wextra;-Winvalid-pch;-Wmissing-field-initializers;-Wunreachable-code;-Wcast-align;-Wcast-qual;-Wdisabled-optimization;-Wformat=2;-Wformat-nonliteral;-Wuninitialized;-Wformat-security;-Wformat-y2k;-Winit-self;-Wmissing-declarations;-Wmissing-include-dirs;-Wredundant-decls;-Wstrict-overflow=5;-Wno-unused;-Wno-conversion;-Wctor-dtor-privacy;-Wsign-promo;-Woverloaded-virtual;-Wlogical-op;-Wstrict-null-sentinel;-Wnoexcept;-O0;-Winline;-g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_LIST "")
set(CONAN_EXE_LINKER_FLAGS_DORY-CTRL_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_DORY-CTRL "${CONAN_FRAMEWORKS_DORY-CTRL}" "_DORY-CTRL" "")
# Append to aggregated values variable
set(CONAN_LIBS_DORY-CTRL ${CONAN_PKG_LIBS_DORY-CTRL} ${CONAN_SYSTEM_LIBS_DORY-CTRL} ${CONAN_FRAMEWORKS_FOUND_DORY-CTRL})


#################
###  DORY-MEMSTORE
#################
set(CONAN_DORY-MEMSTORE_ROOT "/rhome/fhous001/.conan/data/dory-memstore/0.0.1/_/_/package/479b2606695b53c353efe0e9c4c15c21ac6eab11")
set(CONAN_INCLUDE_DIRS_DORY-MEMSTORE "/rhome/fhous001/.conan/data/dory-memstore/0.0.1/_/_/package/479b2606695b53c353efe0e9c4c15c21ac6eab11/include")
set(CONAN_LIB_DIRS_DORY-MEMSTORE "/rhome/fhous001/.conan/data/dory-memstore/0.0.1/_/_/package/479b2606695b53c353efe0e9c4c15c21ac6eab11/lib")
set(CONAN_BIN_DIRS_DORY-MEMSTORE )
set(CONAN_RES_DIRS_DORY-MEMSTORE )
set(CONAN_SRC_DIRS_DORY-MEMSTORE )
set(CONAN_BUILD_DIRS_DORY-MEMSTORE "/rhome/fhous001/.conan/data/dory-memstore/0.0.1/_/_/package/479b2606695b53c353efe0e9c4c15c21ac6eab11/")
set(CONAN_FRAMEWORK_DIRS_DORY-MEMSTORE )
set(CONAN_LIBS_DORY-MEMSTORE dorymemstore)
set(CONAN_PKG_LIBS_DORY-MEMSTORE dorymemstore)
set(CONAN_SYSTEM_LIBS_DORY-MEMSTORE )
set(CONAN_FRAMEWORKS_DORY-MEMSTORE )
set(CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE "")  # Will be filled later
set(CONAN_DEFINES_DORY-MEMSTORE )
set(CONAN_BUILD_MODULES_PATHS_DORY-MEMSTORE )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_DORY-MEMSTORE )

set(CONAN_C_FLAGS_DORY-MEMSTORE "")
set(CONAN_CXX_FLAGS_DORY-MEMSTORE "-Wconversion -Wfloat-equal -Wpedantic -Wpointer-arith -Wswitch-default -Wpacked -Wextra -Winvalid-pch -Wmissing-field-initializers -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wuninitialized -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wstrict-overflow=5 -Wno-unused -Wno-conversion -Wctor-dtor-privacy -Wsign-promo -Woverloaded-virtual -Wlogical-op -Wstrict-null-sentinel -Wnoexcept -O0 -Winline -g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE "")
set(CONAN_EXE_LINKER_FLAGS_DORY-MEMSTORE "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_DORY-MEMSTORE_LIST "")
set(CONAN_CXX_FLAGS_DORY-MEMSTORE_LIST "-Wconversion;-Wfloat-equal;-Wpedantic;-Wpointer-arith;-Wswitch-default;-Wpacked;-Wextra;-Winvalid-pch;-Wmissing-field-initializers;-Wunreachable-code;-Wcast-align;-Wcast-qual;-Wdisabled-optimization;-Wformat=2;-Wformat-nonliteral;-Wuninitialized;-Wformat-security;-Wformat-y2k;-Winit-self;-Wmissing-declarations;-Wmissing-include-dirs;-Wredundant-decls;-Wstrict-overflow=5;-Wno-unused;-Wno-conversion;-Wctor-dtor-privacy;-Wsign-promo;-Woverloaded-virtual;-Wlogical-op;-Wstrict-null-sentinel;-Wnoexcept;-O0;-Winline;-g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_LIST "")
set(CONAN_EXE_LINKER_FLAGS_DORY-MEMSTORE_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE "${CONAN_FRAMEWORKS_DORY-MEMSTORE}" "_DORY-MEMSTORE" "")
# Append to aggregated values variable
set(CONAN_LIBS_DORY-MEMSTORE ${CONAN_PKG_LIBS_DORY-MEMSTORE} ${CONAN_SYSTEM_LIBS_DORY-MEMSTORE} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE})


#################
###  DORY-SHARED
#################
set(CONAN_DORY-SHARED_ROOT "/rhome/fhous001/.conan/data/dory-shared/0.0.1/_/_/package/f63326f19ac8d49f2cfd9ad63b7055485f3ba8e0")
set(CONAN_INCLUDE_DIRS_DORY-SHARED "/rhome/fhous001/.conan/data/dory-shared/0.0.1/_/_/package/f63326f19ac8d49f2cfd9ad63b7055485f3ba8e0/include")
set(CONAN_LIB_DIRS_DORY-SHARED "/rhome/fhous001/.conan/data/dory-shared/0.0.1/_/_/package/f63326f19ac8d49f2cfd9ad63b7055485f3ba8e0/lib")
set(CONAN_BIN_DIRS_DORY-SHARED )
set(CONAN_RES_DIRS_DORY-SHARED )
set(CONAN_SRC_DIRS_DORY-SHARED )
set(CONAN_BUILD_DIRS_DORY-SHARED "/rhome/fhous001/.conan/data/dory-shared/0.0.1/_/_/package/f63326f19ac8d49f2cfd9ad63b7055485f3ba8e0/")
set(CONAN_FRAMEWORK_DIRS_DORY-SHARED )
set(CONAN_LIBS_DORY-SHARED doryshared)
set(CONAN_PKG_LIBS_DORY-SHARED doryshared)
set(CONAN_SYSTEM_LIBS_DORY-SHARED )
set(CONAN_FRAMEWORKS_DORY-SHARED )
set(CONAN_FRAMEWORKS_FOUND_DORY-SHARED "")  # Will be filled later
set(CONAN_DEFINES_DORY-SHARED )
set(CONAN_BUILD_MODULES_PATHS_DORY-SHARED )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_DORY-SHARED )

set(CONAN_C_FLAGS_DORY-SHARED "")
set(CONAN_CXX_FLAGS_DORY-SHARED "-Wconversion -Wfloat-equal -Wpedantic -Wpointer-arith -Wswitch-default -Wpacked -Wextra -Winvalid-pch -Wmissing-field-initializers -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wuninitialized -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wstrict-overflow=5 -Wno-unused -Wno-conversion -Wctor-dtor-privacy -Wsign-promo -Woverloaded-virtual -Wlogical-op -Wstrict-null-sentinel -Wnoexcept -O0 -Winline -g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-SHARED "")
set(CONAN_EXE_LINKER_FLAGS_DORY-SHARED "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_DORY-SHARED_LIST "")
set(CONAN_CXX_FLAGS_DORY-SHARED_LIST "-Wconversion;-Wfloat-equal;-Wpedantic;-Wpointer-arith;-Wswitch-default;-Wpacked;-Wextra;-Winvalid-pch;-Wmissing-field-initializers;-Wunreachable-code;-Wcast-align;-Wcast-qual;-Wdisabled-optimization;-Wformat=2;-Wformat-nonliteral;-Wuninitialized;-Wformat-security;-Wformat-y2k;-Winit-self;-Wmissing-declarations;-Wmissing-include-dirs;-Wredundant-decls;-Wstrict-overflow=5;-Wno-unused;-Wno-conversion;-Wctor-dtor-privacy;-Wsign-promo;-Woverloaded-virtual;-Wlogical-op;-Wstrict-null-sentinel;-Wnoexcept;-O0;-Winline;-g")
set(CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_LIST "")
set(CONAN_EXE_LINKER_FLAGS_DORY-SHARED_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_DORY-SHARED "${CONAN_FRAMEWORKS_DORY-SHARED}" "_DORY-SHARED" "")
# Append to aggregated values variable
set(CONAN_LIBS_DORY-SHARED ${CONAN_PKG_LIBS_DORY-SHARED} ${CONAN_SYSTEM_LIBS_DORY-SHARED} ${CONAN_FRAMEWORKS_FOUND_DORY-SHARED})


#################
###  DORY-EXTERNAL
#################
set(CONAN_DORY-EXTERNAL_ROOT "/rhome/fhous001/.conan/data/dory-external/0.0.1/_/_/package/3b414b6f1701e2c06d0d214fcdf14d6d1e03d36d")
set(CONAN_INCLUDE_DIRS_DORY-EXTERNAL "/rhome/fhous001/.conan/data/dory-external/0.0.1/_/_/package/3b414b6f1701e2c06d0d214fcdf14d6d1e03d36d/include")
set(CONAN_LIB_DIRS_DORY-EXTERNAL )
set(CONAN_BIN_DIRS_DORY-EXTERNAL )
set(CONAN_RES_DIRS_DORY-EXTERNAL )
set(CONAN_SRC_DIRS_DORY-EXTERNAL )
set(CONAN_BUILD_DIRS_DORY-EXTERNAL "/rhome/fhous001/.conan/data/dory-external/0.0.1/_/_/package/3b414b6f1701e2c06d0d214fcdf14d6d1e03d36d/")
set(CONAN_FRAMEWORK_DIRS_DORY-EXTERNAL )
set(CONAN_LIBS_DORY-EXTERNAL )
set(CONAN_PKG_LIBS_DORY-EXTERNAL )
set(CONAN_SYSTEM_LIBS_DORY-EXTERNAL ibverbs memcached)
set(CONAN_FRAMEWORKS_DORY-EXTERNAL )
set(CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL "")  # Will be filled later
set(CONAN_DEFINES_DORY-EXTERNAL )
set(CONAN_BUILD_MODULES_PATHS_DORY-EXTERNAL )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_DORY-EXTERNAL )

set(CONAN_C_FLAGS_DORY-EXTERNAL "")
set(CONAN_CXX_FLAGS_DORY-EXTERNAL "")
set(CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL "")
set(CONAN_EXE_LINKER_FLAGS_DORY-EXTERNAL "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_DORY-EXTERNAL_LIST "")
set(CONAN_CXX_FLAGS_DORY-EXTERNAL_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_LIST "")
set(CONAN_EXE_LINKER_FLAGS_DORY-EXTERNAL_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL "${CONAN_FRAMEWORKS_DORY-EXTERNAL}" "_DORY-EXTERNAL" "")
# Append to aggregated values variable
set(CONAN_LIBS_DORY-EXTERNAL ${CONAN_PKG_LIBS_DORY-EXTERNAL} ${CONAN_SYSTEM_LIBS_DORY-EXTERNAL} ${CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL})


#################
###  SPDLOG
#################
set(CONAN_SPDLOG_ROOT "/rhome/fhous001/.conan/data/spdlog/1.5.0/_/_/package/e79d06d3eb96a851113589601aefad4c6b2c5f52")
set(CONAN_INCLUDE_DIRS_SPDLOG "/rhome/fhous001/.conan/data/spdlog/1.5.0/_/_/package/e79d06d3eb96a851113589601aefad4c6b2c5f52/include")
set(CONAN_LIB_DIRS_SPDLOG "/rhome/fhous001/.conan/data/spdlog/1.5.0/_/_/package/e79d06d3eb96a851113589601aefad4c6b2c5f52/lib")
set(CONAN_BIN_DIRS_SPDLOG )
set(CONAN_RES_DIRS_SPDLOG )
set(CONAN_SRC_DIRS_SPDLOG )
set(CONAN_BUILD_DIRS_SPDLOG "/rhome/fhous001/.conan/data/spdlog/1.5.0/_/_/package/e79d06d3eb96a851113589601aefad4c6b2c5f52/")
set(CONAN_FRAMEWORK_DIRS_SPDLOG )
set(CONAN_LIBS_SPDLOG spdlogd)
set(CONAN_PKG_LIBS_SPDLOG spdlogd)
set(CONAN_SYSTEM_LIBS_SPDLOG pthread)
set(CONAN_FRAMEWORKS_SPDLOG )
set(CONAN_FRAMEWORKS_FOUND_SPDLOG "")  # Will be filled later
set(CONAN_DEFINES_SPDLOG "-DSPDLOG_COMPILED_LIB"
			"-DSPDLOG_FMT_EXTERNAL")
set(CONAN_BUILD_MODULES_PATHS_SPDLOG )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_SPDLOG "SPDLOG_COMPILED_LIB"
			"SPDLOG_FMT_EXTERNAL")

set(CONAN_C_FLAGS_SPDLOG "")
set(CONAN_CXX_FLAGS_SPDLOG "")
set(CONAN_SHARED_LINKER_FLAGS_SPDLOG "")
set(CONAN_EXE_LINKER_FLAGS_SPDLOG "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_SPDLOG_LIST "")
set(CONAN_CXX_FLAGS_SPDLOG_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_SPDLOG_LIST "")
set(CONAN_EXE_LINKER_FLAGS_SPDLOG_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_SPDLOG "${CONAN_FRAMEWORKS_SPDLOG}" "_SPDLOG" "")
# Append to aggregated values variable
set(CONAN_LIBS_SPDLOG ${CONAN_PKG_LIBS_SPDLOG} ${CONAN_SYSTEM_LIBS_SPDLOG} ${CONAN_FRAMEWORKS_FOUND_SPDLOG})


#################
###  FMT
#################
set(CONAN_FMT_ROOT "/rhome/fhous001/.conan/data/fmt/6.2.1/_/_/package/467844f290ff4af1df60720f1607bc6095cf829c")
set(CONAN_INCLUDE_DIRS_FMT "/rhome/fhous001/.conan/data/fmt/6.2.1/_/_/package/467844f290ff4af1df60720f1607bc6095cf829c/include")
set(CONAN_LIB_DIRS_FMT "/rhome/fhous001/.conan/data/fmt/6.2.1/_/_/package/467844f290ff4af1df60720f1607bc6095cf829c/lib")
set(CONAN_BIN_DIRS_FMT )
set(CONAN_RES_DIRS_FMT )
set(CONAN_SRC_DIRS_FMT )
set(CONAN_BUILD_DIRS_FMT "/rhome/fhous001/.conan/data/fmt/6.2.1/_/_/package/467844f290ff4af1df60720f1607bc6095cf829c/")
set(CONAN_FRAMEWORK_DIRS_FMT )
set(CONAN_LIBS_FMT fmtd)
set(CONAN_PKG_LIBS_FMT fmtd)
set(CONAN_SYSTEM_LIBS_FMT )
set(CONAN_FRAMEWORKS_FMT )
set(CONAN_FRAMEWORKS_FOUND_FMT "")  # Will be filled later
set(CONAN_DEFINES_FMT )
set(CONAN_BUILD_MODULES_PATHS_FMT )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_FMT )

set(CONAN_C_FLAGS_FMT "")
set(CONAN_CXX_FLAGS_FMT "")
set(CONAN_SHARED_LINKER_FLAGS_FMT "")
set(CONAN_EXE_LINKER_FLAGS_FMT "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_FMT_LIST "")
set(CONAN_CXX_FLAGS_FMT_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_FMT_LIST "")
set(CONAN_EXE_LINKER_FLAGS_FMT_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_FMT "${CONAN_FRAMEWORKS_FMT}" "_FMT" "")
# Append to aggregated values variable
set(CONAN_LIBS_FMT ${CONAN_PKG_LIBS_FMT} ${CONAN_SYSTEM_LIBS_FMT} ${CONAN_FRAMEWORKS_FOUND_FMT})


### Definition of global aggregated variables ###

set(CONAN_PACKAGE_NAME dory-log)
set(CONAN_PACKAGE_VERSION 0.0.1)

set(CONAN_SETTINGS_ARCH "x86_64")
set(CONAN_SETTINGS_BUILD_TYPE "Debug")
set(CONAN_SETTINGS_COMPILER "gcc")
set(CONAN_SETTINGS_COMPILER_CPPSTD "17")
set(CONAN_SETTINGS_COMPILER_LIBCXX "libstdc++11")
set(CONAN_SETTINGS_COMPILER_VERSION "7")
set(CONAN_SETTINGS_OS "Linux")

set(CONAN_DEPENDENCIES dory-memory dory-connection dory-ctrl dory-memstore dory-shared dory-external spdlog fmt)
# Storing original command line args (CMake helper) flags
set(CONAN_CMD_CXX_FLAGS ${CONAN_CXX_FLAGS})

set(CONAN_CMD_SHARED_LINKER_FLAGS ${CONAN_SHARED_LINKER_FLAGS})
set(CONAN_CMD_C_FLAGS ${CONAN_C_FLAGS})
# Defining accumulated conan variables for all deps

set(CONAN_INCLUDE_DIRS "/rhome/fhous001/.conan/data/dory-memory/0.0.1/_/_/package/4949b8a042c8a276fecbdf742b8ffc951aa97499/include"
			"/rhome/fhous001/.conan/data/dory-connection/0.0.1/_/_/package/ef4fb424620497e1e0b7f0186a98dec0be1eeabe/include"
			"/rhome/fhous001/.conan/data/dory-ctrl/0.0.1/_/_/package/fb4ac4a542d099bfa0033d44442128dcb9054402/include"
			"/rhome/fhous001/.conan/data/dory-memstore/0.0.1/_/_/package/479b2606695b53c353efe0e9c4c15c21ac6eab11/include"
			"/rhome/fhous001/.conan/data/dory-shared/0.0.1/_/_/package/f63326f19ac8d49f2cfd9ad63b7055485f3ba8e0/include"
			"/rhome/fhous001/.conan/data/dory-external/0.0.1/_/_/package/3b414b6f1701e2c06d0d214fcdf14d6d1e03d36d/include"
			"/rhome/fhous001/.conan/data/spdlog/1.5.0/_/_/package/e79d06d3eb96a851113589601aefad4c6b2c5f52/include"
			"/rhome/fhous001/.conan/data/fmt/6.2.1/_/_/package/467844f290ff4af1df60720f1607bc6095cf829c/include" ${CONAN_INCLUDE_DIRS})
set(CONAN_LIB_DIRS "/rhome/fhous001/.conan/data/dory-memory/0.0.1/_/_/package/4949b8a042c8a276fecbdf742b8ffc951aa97499/lib"
			"/rhome/fhous001/.conan/data/dory-connection/0.0.1/_/_/package/ef4fb424620497e1e0b7f0186a98dec0be1eeabe/lib"
			"/rhome/fhous001/.conan/data/dory-ctrl/0.0.1/_/_/package/fb4ac4a542d099bfa0033d44442128dcb9054402/lib"
			"/rhome/fhous001/.conan/data/dory-memstore/0.0.1/_/_/package/479b2606695b53c353efe0e9c4c15c21ac6eab11/lib"
			"/rhome/fhous001/.conan/data/dory-shared/0.0.1/_/_/package/f63326f19ac8d49f2cfd9ad63b7055485f3ba8e0/lib"
			"/rhome/fhous001/.conan/data/spdlog/1.5.0/_/_/package/e79d06d3eb96a851113589601aefad4c6b2c5f52/lib"
			"/rhome/fhous001/.conan/data/fmt/6.2.1/_/_/package/467844f290ff4af1df60720f1607bc6095cf829c/lib" ${CONAN_LIB_DIRS})
set(CONAN_BIN_DIRS  ${CONAN_BIN_DIRS})
set(CONAN_RES_DIRS  ${CONAN_RES_DIRS})
set(CONAN_FRAMEWORK_DIRS  ${CONAN_FRAMEWORK_DIRS})
set(CONAN_LIBS dorymem doryconn doryctrl dorymemstore doryshared spdlogd fmtd ${CONAN_LIBS})
set(CONAN_PKG_LIBS dorymem doryconn doryctrl dorymemstore doryshared spdlogd fmtd ${CONAN_PKG_LIBS})
set(CONAN_SYSTEM_LIBS ibverbs memcached pthread ${CONAN_SYSTEM_LIBS})
set(CONAN_FRAMEWORKS  ${CONAN_FRAMEWORKS})
set(CONAN_FRAMEWORKS_FOUND "")  # Will be filled later
set(CONAN_DEFINES "-DSPDLOG_COMPILED_LIB"
			"-DSPDLOG_FMT_EXTERNAL" ${CONAN_DEFINES})
set(CONAN_BUILD_MODULES_PATHS  ${CONAN_BUILD_MODULES_PATHS})
set(CONAN_CMAKE_MODULE_PATH "/rhome/fhous001/.conan/data/dory-memory/0.0.1/_/_/package/4949b8a042c8a276fecbdf742b8ffc951aa97499/"
			"/rhome/fhous001/.conan/data/dory-connection/0.0.1/_/_/package/ef4fb424620497e1e0b7f0186a98dec0be1eeabe/"
			"/rhome/fhous001/.conan/data/dory-ctrl/0.0.1/_/_/package/fb4ac4a542d099bfa0033d44442128dcb9054402/"
			"/rhome/fhous001/.conan/data/dory-memstore/0.0.1/_/_/package/479b2606695b53c353efe0e9c4c15c21ac6eab11/"
			"/rhome/fhous001/.conan/data/dory-shared/0.0.1/_/_/package/f63326f19ac8d49f2cfd9ad63b7055485f3ba8e0/"
			"/rhome/fhous001/.conan/data/dory-external/0.0.1/_/_/package/3b414b6f1701e2c06d0d214fcdf14d6d1e03d36d/"
			"/rhome/fhous001/.conan/data/spdlog/1.5.0/_/_/package/e79d06d3eb96a851113589601aefad4c6b2c5f52/"
			"/rhome/fhous001/.conan/data/fmt/6.2.1/_/_/package/467844f290ff4af1df60720f1607bc6095cf829c/" ${CONAN_CMAKE_MODULE_PATH})

set(CONAN_CXX_FLAGS "-Wconversion -Wfloat-equal -Wpedantic -Wpointer-arith -Wswitch-default -Wpacked -Wextra -Winvalid-pch -Wmissing-field-initializers -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wuninitialized -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wstrict-overflow=5 -Wno-unused -Wno-conversion -Wctor-dtor-privacy -Wsign-promo -Woverloaded-virtual -Wlogical-op -Wstrict-null-sentinel -Wnoexcept -O0 -Winline -g ${CONAN_CXX_FLAGS}")
set(CONAN_SHARED_LINKER_FLAGS " ${CONAN_SHARED_LINKER_FLAGS}")
set(CONAN_EXE_LINKER_FLAGS " ${CONAN_EXE_LINKER_FLAGS}")
set(CONAN_C_FLAGS " ${CONAN_C_FLAGS}")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND "${CONAN_FRAMEWORKS}" "" "")
# Append to aggregated values variable: Use CONAN_LIBS instead of CONAN_PKG_LIBS to include user appended vars
set(CONAN_LIBS ${CONAN_LIBS} ${CONAN_SYSTEM_LIBS} ${CONAN_FRAMEWORKS_FOUND})


###  Definition of macros and functions ###

macro(conan_define_targets)
    if(${CMAKE_VERSION} VERSION_LESS "3.1.2")
        message(FATAL_ERROR "TARGETS not supported by your CMake version!")
    endif()  # CMAKE > 3.x
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CONAN_CMD_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CONAN_CMD_C_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CONAN_CMD_SHARED_LINKER_FLAGS}")


    set(_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES "${CONAN_SYSTEM_LIBS_DORY-MEMORY} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMORY} CONAN_PKG::dory-shared")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMORY}" "${CONAN_LIB_DIRS_DORY-MEMORY}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMORY "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES}"
                                  "" dory-memory)
    set(_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_DORY-MEMORY_DEBUG} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMORY_DEBUG} CONAN_PKG::dory-shared")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMORY_DEBUG}" "${CONAN_LIB_DIRS_DORY-MEMORY_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMORY_DEBUG "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_DEBUG}"
                                  "debug" dory-memory)
    set(_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_DORY-MEMORY_RELEASE} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMORY_RELEASE} CONAN_PKG::dory-shared")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMORY_RELEASE}" "${CONAN_LIB_DIRS_DORY-MEMORY_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMORY_RELEASE "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELEASE}"
                                  "release" dory-memory)
    set(_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_DORY-MEMORY_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMORY_RELWITHDEBINFO} CONAN_PKG::dory-shared")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMORY_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_DORY-MEMORY_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMORY_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" dory-memory)
    set(_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_DORY-MEMORY_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMORY_MINSIZEREL} CONAN_PKG::dory-shared")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMORY_MINSIZEREL}" "${CONAN_LIB_DIRS_DORY-MEMORY_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMORY_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" dory-memory)

    add_library(CONAN_PKG::dory-memory INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::dory-memory PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_DORY-MEMORY} ${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMORY_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_DORY-MEMORY_RELEASE} ${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMORY_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_DORY-MEMORY_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMORY_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_DORY-MEMORY_MINSIZEREL} ${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMORY_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_DORY-MEMORY_DEBUG} ${_CONAN_PKG_LIBS_DORY-MEMORY_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMORY_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMORY_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::dory-memory PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_DORY-MEMORY}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_DORY-MEMORY_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_DORY-MEMORY_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_DORY-MEMORY_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DORY-MEMORY_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-memory PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_DORY-MEMORY}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMORY_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMORY_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMORY_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMORY_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-memory PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_DORY-MEMORY_LIST} ${CONAN_CXX_FLAGS_DORY-MEMORY_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_DORY-MEMORY_RELEASE_LIST} ${CONAN_CXX_FLAGS_DORY-MEMORY_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_DORY-MEMORY_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_DORY-MEMORY_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_DORY-MEMORY_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_DORY-MEMORY_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_DORY-MEMORY_DEBUG_LIST}  ${CONAN_CXX_FLAGS_DORY-MEMORY_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES "${CONAN_SYSTEM_LIBS_DORY-CONNECTION} ${CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION} CONAN_PKG::dory-shared CONAN_PKG::dory-ctrl CONAN_PKG::dory-memstore")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CONNECTION}" "${CONAN_LIB_DIRS_DORY-CONNECTION}"
                                  CONAN_PACKAGE_TARGETS_DORY-CONNECTION "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES}"
                                  "" dory-connection)
    set(_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_DORY-CONNECTION_DEBUG} ${CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION_DEBUG} CONAN_PKG::dory-shared CONAN_PKG::dory-ctrl CONAN_PKG::dory-memstore")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CONNECTION_DEBUG}" "${CONAN_LIB_DIRS_DORY-CONNECTION_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_DORY-CONNECTION_DEBUG "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_DEBUG}"
                                  "debug" dory-connection)
    set(_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_DORY-CONNECTION_RELEASE} ${CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION_RELEASE} CONAN_PKG::dory-shared CONAN_PKG::dory-ctrl CONAN_PKG::dory-memstore")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CONNECTION_RELEASE}" "${CONAN_LIB_DIRS_DORY-CONNECTION_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_DORY-CONNECTION_RELEASE "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELEASE}"
                                  "release" dory-connection)
    set(_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_DORY-CONNECTION_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION_RELWITHDEBINFO} CONAN_PKG::dory-shared CONAN_PKG::dory-ctrl CONAN_PKG::dory-memstore")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CONNECTION_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_DORY-CONNECTION_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_DORY-CONNECTION_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" dory-connection)
    set(_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_DORY-CONNECTION_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_DORY-CONNECTION_MINSIZEREL} CONAN_PKG::dory-shared CONAN_PKG::dory-ctrl CONAN_PKG::dory-memstore")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CONNECTION_MINSIZEREL}" "${CONAN_LIB_DIRS_DORY-CONNECTION_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_DORY-CONNECTION_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" dory-connection)

    add_library(CONAN_PKG::dory-connection INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::dory-connection PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_DORY-CONNECTION} ${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CONNECTION_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_DORY-CONNECTION_RELEASE} ${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CONNECTION_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_DORY-CONNECTION_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CONNECTION_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_DORY-CONNECTION_MINSIZEREL} ${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CONNECTION_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_DORY-CONNECTION_DEBUG} ${_CONAN_PKG_LIBS_DORY-CONNECTION_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CONNECTION_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CONNECTION_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::dory-connection PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_DORY-CONNECTION}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_DORY-CONNECTION_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_DORY-CONNECTION_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_DORY-CONNECTION_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DORY-CONNECTION_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-connection PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_DORY-CONNECTION}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_DORY-CONNECTION_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_DORY-CONNECTION_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_DORY-CONNECTION_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_DORY-CONNECTION_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-connection PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_DORY-CONNECTION_LIST} ${CONAN_CXX_FLAGS_DORY-CONNECTION_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_DORY-CONNECTION_RELEASE_LIST} ${CONAN_CXX_FLAGS_DORY-CONNECTION_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_DORY-CONNECTION_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_DORY-CONNECTION_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_DORY-CONNECTION_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_DORY-CONNECTION_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_DORY-CONNECTION_DEBUG_LIST}  ${CONAN_CXX_FLAGS_DORY-CONNECTION_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES "${CONAN_SYSTEM_LIBS_DORY-CTRL} ${CONAN_FRAMEWORKS_FOUND_DORY-CTRL} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CTRL}" "${CONAN_LIB_DIRS_DORY-CTRL}"
                                  CONAN_PACKAGE_TARGETS_DORY-CTRL "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES}"
                                  "" dory-ctrl)
    set(_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_DORY-CTRL_DEBUG} ${CONAN_FRAMEWORKS_FOUND_DORY-CTRL_DEBUG} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CTRL_DEBUG}" "${CONAN_LIB_DIRS_DORY-CTRL_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_DORY-CTRL_DEBUG "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_DEBUG}"
                                  "debug" dory-ctrl)
    set(_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_DORY-CTRL_RELEASE} ${CONAN_FRAMEWORKS_FOUND_DORY-CTRL_RELEASE} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CTRL_RELEASE}" "${CONAN_LIB_DIRS_DORY-CTRL_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_DORY-CTRL_RELEASE "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELEASE}"
                                  "release" dory-ctrl)
    set(_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_DORY-CTRL_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_DORY-CTRL_RELWITHDEBINFO} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CTRL_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_DORY-CTRL_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_DORY-CTRL_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" dory-ctrl)
    set(_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_DORY-CTRL_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_DORY-CTRL_MINSIZEREL} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-CTRL_MINSIZEREL}" "${CONAN_LIB_DIRS_DORY-CTRL_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_DORY-CTRL_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" dory-ctrl)

    add_library(CONAN_PKG::dory-ctrl INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::dory-ctrl PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_DORY-CTRL} ${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CTRL_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_DORY-CTRL_RELEASE} ${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CTRL_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_DORY-CTRL_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CTRL_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_DORY-CTRL_MINSIZEREL} ${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CTRL_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_DORY-CTRL_DEBUG} ${_CONAN_PKG_LIBS_DORY-CTRL_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-CTRL_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-CTRL_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::dory-ctrl PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_DORY-CTRL}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_DORY-CTRL_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_DORY-CTRL_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_DORY-CTRL_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DORY-CTRL_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-ctrl PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_DORY-CTRL}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_DORY-CTRL_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_DORY-CTRL_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_DORY-CTRL_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_DORY-CTRL_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-ctrl PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_DORY-CTRL_LIST} ${CONAN_CXX_FLAGS_DORY-CTRL_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_DORY-CTRL_RELEASE_LIST} ${CONAN_CXX_FLAGS_DORY-CTRL_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_DORY-CTRL_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_DORY-CTRL_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_DORY-CTRL_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_DORY-CTRL_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_DORY-CTRL_DEBUG_LIST}  ${CONAN_CXX_FLAGS_DORY-CTRL_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES "${CONAN_SYSTEM_LIBS_DORY-MEMSTORE} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMSTORE}" "${CONAN_LIB_DIRS_DORY-MEMSTORE}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMSTORE "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES}"
                                  "" dory-memstore)
    set(_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_DORY-MEMSTORE_DEBUG} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE_DEBUG} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMSTORE_DEBUG}" "${CONAN_LIB_DIRS_DORY-MEMSTORE_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_DEBUG "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_DEBUG}"
                                  "debug" dory-memstore)
    set(_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_DORY-MEMSTORE_RELEASE} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE_RELEASE} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMSTORE_RELEASE}" "${CONAN_LIB_DIRS_DORY-MEMSTORE_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_RELEASE "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELEASE}"
                                  "release" dory-memstore)
    set(_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_DORY-MEMSTORE_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE_RELWITHDEBINFO} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMSTORE_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_DORY-MEMSTORE_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" dory-memstore)
    set(_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_DORY-MEMSTORE_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_DORY-MEMSTORE_MINSIZEREL} CONAN_PKG::dory-shared CONAN_PKG::dory-external")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-MEMSTORE_MINSIZEREL}" "${CONAN_LIB_DIRS_DORY-MEMSTORE_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" dory-memstore)

    add_library(CONAN_PKG::dory-memstore INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::dory-memstore PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_DORY-MEMSTORE} ${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMSTORE_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_RELEASE} ${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMSTORE_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMSTORE_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_MINSIZEREL} ${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMSTORE_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_DORY-MEMSTORE_DEBUG} ${_CONAN_PKG_LIBS_DORY-MEMSTORE_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-MEMSTORE_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-MEMSTORE_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::dory-memstore PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_DORY-MEMSTORE}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_DORY-MEMSTORE_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_DORY-MEMSTORE_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_DORY-MEMSTORE_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DORY-MEMSTORE_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-memstore PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_DORY-MEMSTORE}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMSTORE_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMSTORE_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMSTORE_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_DORY-MEMSTORE_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-memstore PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_DORY-MEMSTORE_LIST} ${CONAN_CXX_FLAGS_DORY-MEMSTORE_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_DORY-MEMSTORE_RELEASE_LIST} ${CONAN_CXX_FLAGS_DORY-MEMSTORE_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_DORY-MEMSTORE_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_DORY-MEMSTORE_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_DORY-MEMSTORE_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_DORY-MEMSTORE_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_DORY-MEMSTORE_DEBUG_LIST}  ${CONAN_CXX_FLAGS_DORY-MEMSTORE_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES "${CONAN_SYSTEM_LIBS_DORY-SHARED} ${CONAN_FRAMEWORKS_FOUND_DORY-SHARED} CONAN_PKG::spdlog")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-SHARED}" "${CONAN_LIB_DIRS_DORY-SHARED}"
                                  CONAN_PACKAGE_TARGETS_DORY-SHARED "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES}"
                                  "" dory-shared)
    set(_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_DORY-SHARED_DEBUG} ${CONAN_FRAMEWORKS_FOUND_DORY-SHARED_DEBUG} CONAN_PKG::spdlog")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-SHARED_DEBUG}" "${CONAN_LIB_DIRS_DORY-SHARED_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_DORY-SHARED_DEBUG "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_DEBUG}"
                                  "debug" dory-shared)
    set(_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_DORY-SHARED_RELEASE} ${CONAN_FRAMEWORKS_FOUND_DORY-SHARED_RELEASE} CONAN_PKG::spdlog")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-SHARED_RELEASE}" "${CONAN_LIB_DIRS_DORY-SHARED_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_DORY-SHARED_RELEASE "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELEASE}"
                                  "release" dory-shared)
    set(_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_DORY-SHARED_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_DORY-SHARED_RELWITHDEBINFO} CONAN_PKG::spdlog")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-SHARED_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_DORY-SHARED_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_DORY-SHARED_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" dory-shared)
    set(_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_DORY-SHARED_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_DORY-SHARED_MINSIZEREL} CONAN_PKG::spdlog")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-SHARED_MINSIZEREL}" "${CONAN_LIB_DIRS_DORY-SHARED_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_DORY-SHARED_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" dory-shared)

    add_library(CONAN_PKG::dory-shared INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::dory-shared PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_DORY-SHARED} ${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-SHARED_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_DORY-SHARED_RELEASE} ${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-SHARED_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_DORY-SHARED_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-SHARED_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_DORY-SHARED_MINSIZEREL} ${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-SHARED_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_DORY-SHARED_DEBUG} ${_CONAN_PKG_LIBS_DORY-SHARED_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-SHARED_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-SHARED_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::dory-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_DORY-SHARED}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_DORY-SHARED_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_DORY-SHARED_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_DORY-SHARED_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DORY-SHARED_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-shared PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_DORY-SHARED}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_DORY-SHARED_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_DORY-SHARED_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_DORY-SHARED_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_DORY-SHARED_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-shared PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_DORY-SHARED_LIST} ${CONAN_CXX_FLAGS_DORY-SHARED_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_DORY-SHARED_RELEASE_LIST} ${CONAN_CXX_FLAGS_DORY-SHARED_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_DORY-SHARED_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_DORY-SHARED_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_DORY-SHARED_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_DORY-SHARED_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_DORY-SHARED_DEBUG_LIST}  ${CONAN_CXX_FLAGS_DORY-SHARED_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES "${CONAN_SYSTEM_LIBS_DORY-EXTERNAL} ${CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-EXTERNAL}" "${CONAN_LIB_DIRS_DORY-EXTERNAL}"
                                  CONAN_PACKAGE_TARGETS_DORY-EXTERNAL "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES}"
                                  "" dory-external)
    set(_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_DORY-EXTERNAL_DEBUG} ${CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-EXTERNAL_DEBUG}" "${CONAN_LIB_DIRS_DORY-EXTERNAL_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_DEBUG "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_DEBUG}"
                                  "debug" dory-external)
    set(_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_DORY-EXTERNAL_RELEASE} ${CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-EXTERNAL_RELEASE}" "${CONAN_LIB_DIRS_DORY-EXTERNAL_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_RELEASE "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELEASE}"
                                  "release" dory-external)
    set(_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_DORY-EXTERNAL_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-EXTERNAL_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_DORY-EXTERNAL_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_RELWITHDEBINFO "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" dory-external)
    set(_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_DORY-EXTERNAL_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_DORY-EXTERNAL_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_DORY-EXTERNAL_MINSIZEREL}" "${CONAN_LIB_DIRS_DORY-EXTERNAL_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_MINSIZEREL "${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" dory-external)

    add_library(CONAN_PKG::dory-external INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::dory-external PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_DORY-EXTERNAL} ${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-EXTERNAL_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_RELEASE} ${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-EXTERNAL_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-EXTERNAL_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_MINSIZEREL} ${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-EXTERNAL_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_DORY-EXTERNAL_DEBUG} ${_CONAN_PKG_LIBS_DORY-EXTERNAL_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_DORY-EXTERNAL_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_DORY-EXTERNAL_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::dory-external PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_DORY-EXTERNAL}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_DORY-EXTERNAL_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_DORY-EXTERNAL_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_DORY-EXTERNAL_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DORY-EXTERNAL_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-external PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_DORY-EXTERNAL}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_DORY-EXTERNAL_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_DORY-EXTERNAL_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_DORY-EXTERNAL_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_DORY-EXTERNAL_DEBUG}>)
    set_property(TARGET CONAN_PKG::dory-external PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_DORY-EXTERNAL_LIST} ${CONAN_CXX_FLAGS_DORY-EXTERNAL_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_DORY-EXTERNAL_RELEASE_LIST} ${CONAN_CXX_FLAGS_DORY-EXTERNAL_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_DORY-EXTERNAL_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_DORY-EXTERNAL_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_DORY-EXTERNAL_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_DORY-EXTERNAL_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_DORY-EXTERNAL_DEBUG_LIST}  ${CONAN_CXX_FLAGS_DORY-EXTERNAL_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES "${CONAN_SYSTEM_LIBS_SPDLOG} ${CONAN_FRAMEWORKS_FOUND_SPDLOG} CONAN_PKG::fmt")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SPDLOG}" "${CONAN_LIB_DIRS_SPDLOG}"
                                  CONAN_PACKAGE_TARGETS_SPDLOG "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES}"
                                  "" spdlog)
    set(_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_SPDLOG_DEBUG} ${CONAN_FRAMEWORKS_FOUND_SPDLOG_DEBUG} CONAN_PKG::fmt")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SPDLOG_DEBUG}" "${CONAN_LIB_DIRS_SPDLOG_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_SPDLOG_DEBUG "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_DEBUG}"
                                  "debug" spdlog)
    set(_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_SPDLOG_RELEASE} ${CONAN_FRAMEWORKS_FOUND_SPDLOG_RELEASE} CONAN_PKG::fmt")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SPDLOG_RELEASE}" "${CONAN_LIB_DIRS_SPDLOG_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_SPDLOG_RELEASE "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELEASE}"
                                  "release" spdlog)
    set(_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_SPDLOG_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_SPDLOG_RELWITHDEBINFO} CONAN_PKG::fmt")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SPDLOG_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_SPDLOG_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_SPDLOG_RELWITHDEBINFO "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" spdlog)
    set(_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_SPDLOG_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_SPDLOG_MINSIZEREL} CONAN_PKG::fmt")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SPDLOG_MINSIZEREL}" "${CONAN_LIB_DIRS_SPDLOG_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_SPDLOG_MINSIZEREL "${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" spdlog)

    add_library(CONAN_PKG::spdlog INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::spdlog PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_SPDLOG} ${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SPDLOG_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_SPDLOG_RELEASE} ${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SPDLOG_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_SPDLOG_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SPDLOG_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_SPDLOG_MINSIZEREL} ${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SPDLOG_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_SPDLOG_DEBUG} ${_CONAN_PKG_LIBS_SPDLOG_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SPDLOG_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SPDLOG_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::spdlog PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_SPDLOG}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_SPDLOG_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_SPDLOG_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_SPDLOG_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_SPDLOG_DEBUG}>)
    set_property(TARGET CONAN_PKG::spdlog PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_SPDLOG}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_SPDLOG_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_SPDLOG_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_SPDLOG_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_SPDLOG_DEBUG}>)
    set_property(TARGET CONAN_PKG::spdlog PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_SPDLOG_LIST} ${CONAN_CXX_FLAGS_SPDLOG_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_SPDLOG_RELEASE_LIST} ${CONAN_CXX_FLAGS_SPDLOG_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_SPDLOG_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_SPDLOG_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_SPDLOG_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_SPDLOG_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_SPDLOG_DEBUG_LIST}  ${CONAN_CXX_FLAGS_SPDLOG_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES "${CONAN_SYSTEM_LIBS_FMT} ${CONAN_FRAMEWORKS_FOUND_FMT} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT}" "${CONAN_LIB_DIRS_FMT}"
                                  CONAN_PACKAGE_TARGETS_FMT "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES}"
                                  "" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_FMT_DEBUG} ${CONAN_FRAMEWORKS_FOUND_FMT_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_DEBUG}" "${CONAN_LIB_DIRS_FMT_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_FMT_DEBUG "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG}"
                                  "debug" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_FMT_RELEASE} ${CONAN_FRAMEWORKS_FOUND_FMT_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_RELEASE}" "${CONAN_LIB_DIRS_FMT_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_FMT_RELEASE "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE}"
                                  "release" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_FMT_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_FMT_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_FMT_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_FMT_RELWITHDEBINFO "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_FMT_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_FMT_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_MINSIZEREL}" "${CONAN_LIB_DIRS_FMT_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_FMT_MINSIZEREL "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" fmt)

    add_library(CONAN_PKG::fmt INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_FMT} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_FMT_RELEASE} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_FMT_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_FMT_MINSIZEREL} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_FMT_DEBUG} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_FMT}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_FMT_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_FMT_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_FMT_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_FMT_DEBUG}>)
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_FMT}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_FMT_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_FMT_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_FMT_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_FMT_DEBUG}>)
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_FMT_LIST} ${CONAN_CXX_FLAGS_FMT_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_FMT_RELEASE_LIST} ${CONAN_CXX_FLAGS_FMT_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_FMT_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_FMT_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_FMT_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_FMT_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_FMT_DEBUG_LIST}  ${CONAN_CXX_FLAGS_FMT_DEBUG_LIST}>)

    set(CONAN_TARGETS CONAN_PKG::dory-memory CONAN_PKG::dory-connection CONAN_PKG::dory-ctrl CONAN_PKG::dory-memstore CONAN_PKG::dory-shared CONAN_PKG::dory-external CONAN_PKG::spdlog CONAN_PKG::fmt)

endmacro()


macro(conan_basic_setup)
    set(options TARGETS NO_OUTPUT_DIRS SKIP_RPATH KEEP_RPATHS SKIP_STD SKIP_FPIC)
    cmake_parse_arguments(ARGUMENTS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if(CONAN_EXPORTED)
        conan_message(STATUS "Conan: called by CMake conan helper")
    endif()

    if(CONAN_IN_LOCAL_CACHE)
        conan_message(STATUS "Conan: called inside local cache")
    endif()

    if(NOT ARGUMENTS_NO_OUTPUT_DIRS)
        conan_message(STATUS "Conan: Adjusting output directories")
        conan_output_dirs_setup()
    endif()

    if(NOT ARGUMENTS_TARGETS)
        conan_message(STATUS "Conan: Using cmake global configuration")
        conan_global_flags()
    else()
        conan_message(STATUS "Conan: Using cmake targets configuration")
        conan_define_targets()
    endif()

    if(ARGUMENTS_SKIP_RPATH)
        # Change by "DEPRECATION" or "SEND_ERROR" when we are ready
        conan_message(WARNING "Conan: SKIP_RPATH is deprecated, it has been renamed to KEEP_RPATHS")
    endif()

    if(NOT ARGUMENTS_SKIP_RPATH AND NOT ARGUMENTS_KEEP_RPATHS)
        # Parameter has renamed, but we keep the compatibility with old SKIP_RPATH
        conan_set_rpath()
    endif()

    if(NOT ARGUMENTS_SKIP_STD)
        conan_set_std()
    endif()

    if(NOT ARGUMENTS_SKIP_FPIC)
        conan_set_fpic()
    endif()

    conan_check_compiler()
    conan_set_libcxx()
    conan_set_vs_runtime()
    conan_set_find_paths()
    conan_include_build_modules()
    conan_set_find_library_paths()
endmacro()


macro(conan_set_find_paths)
    # CMAKE_MODULE_PATH does not have Debug/Release config, but there are variables
    # CONAN_CMAKE_MODULE_PATH_DEBUG to be used by the consumer
    # CMake can find findXXX.cmake files in the root of packages
    set(CMAKE_MODULE_PATH ${CONAN_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})

    # Make find_package() to work
    set(CMAKE_PREFIX_PATH ${CONAN_CMAKE_MODULE_PATH} ${CMAKE_PREFIX_PATH})

    # Set the find root path (cross build)
    set(CMAKE_FIND_ROOT_PATH ${CONAN_CMAKE_FIND_ROOT_PATH} ${CMAKE_FIND_ROOT_PATH})
    if(CONAN_CMAKE_FIND_ROOT_PATH_MODE_PROGRAM)
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ${CONAN_CMAKE_FIND_ROOT_PATH_MODE_PROGRAM})
    endif()
    if(CONAN_CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${CONAN_CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
    endif()
    if(CONAN_CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${CONAN_CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
    endif()
endmacro()


macro(conan_set_find_library_paths)
    # CMAKE_INCLUDE_PATH, CMAKE_LIBRARY_PATH does not have Debug/Release config, but there are variables
    # CONAN_INCLUDE_DIRS_DEBUG/RELEASE CONAN_LIB_DIRS_DEBUG/RELEASE to be used by the consumer
    # For find_library
    set(CMAKE_INCLUDE_PATH ${CONAN_INCLUDE_DIRS} ${CMAKE_INCLUDE_PATH})
    set(CMAKE_LIBRARY_PATH ${CONAN_LIB_DIRS} ${CMAKE_LIBRARY_PATH})
endmacro()


macro(conan_set_vs_runtime)
    if(CONAN_LINK_RUNTIME)
        conan_get_policy(CMP0091 policy_0091)
        if(policy_0091 STREQUAL "NEW")
            if(CONAN_LINK_RUNTIME MATCHES "MTd")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
            elseif(CONAN_LINK_RUNTIME MATCHES "MDd")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
            elseif(CONAN_LINK_RUNTIME MATCHES "MT")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
            elseif(CONAN_LINK_RUNTIME MATCHES "MD")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
            endif()
        else()
            foreach(flag CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE
                         CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO
                         CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_MINSIZEREL)
                if(DEFINED ${flag})
                    string(REPLACE "/MD" ${CONAN_LINK_RUNTIME} ${flag} "${${flag}}")
                endif()
            endforeach()
            foreach(flag CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
                if(DEFINED ${flag})
                    string(REPLACE "/MDd" ${CONAN_LINK_RUNTIME} ${flag} "${${flag}}")
                endif()
            endforeach()
        endif()
    endif()
endmacro()


macro(conan_flags_setup)
    # Macro maintained for backwards compatibility
    conan_set_find_library_paths()
    conan_global_flags()
    conan_set_rpath()
    conan_set_vs_runtime()
    conan_set_libcxx()
endmacro()


function(conan_message MESSAGE_OUTPUT)
    if(NOT CONAN_CMAKE_SILENT_OUTPUT)
        message(${ARGV${0}})
    endif()
endfunction()


function(conan_get_policy policy_id policy)
    if(POLICY "${policy_id}")
        cmake_policy(GET "${policy_id}" _policy)
        set(${policy} "${_policy}" PARENT_SCOPE)
    else()
        set(${policy} "" PARENT_SCOPE)
    endif()
endfunction()


function(conan_find_libraries_abs_path libraries package_libdir libraries_abs_path)
    foreach(_LIBRARY_NAME ${libraries})
        find_library(CONAN_FOUND_LIBRARY NAME ${_LIBRARY_NAME} PATHS ${package_libdir}
                     NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(CONAN_FOUND_LIBRARY)
            conan_message(STATUS "Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${CONAN_FOUND_LIBRARY})
        else()
            conan_message(STATUS "Library ${_LIBRARY_NAME} not found in package, might be system one")
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${_LIBRARY_NAME})
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()
    set(${libraries_abs_path} ${CONAN_FULLPATH_LIBS} PARENT_SCOPE)
endfunction()


function(conan_package_library_targets libraries package_libdir libraries_abs_path deps build_type package_name)
    unset(_CONAN_ACTUAL_TARGETS CACHE)
    unset(_CONAN_FOUND_SYSTEM_LIBS CACHE)
    foreach(_LIBRARY_NAME ${libraries})
        find_library(CONAN_FOUND_LIBRARY NAME ${_LIBRARY_NAME} PATHS ${package_libdir}
                     NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(CONAN_FOUND_LIBRARY)
            conan_message(STATUS "Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")
            set(_LIB_NAME CONAN_LIB::${package_name}_${_LIBRARY_NAME}${build_type})
            add_library(${_LIB_NAME} UNKNOWN IMPORTED)
            set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION ${CONAN_FOUND_LIBRARY})
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${_LIB_NAME})
            set(_CONAN_ACTUAL_TARGETS ${_CONAN_ACTUAL_TARGETS} ${_LIB_NAME})
        else()
            conan_message(STATUS "Library ${_LIBRARY_NAME} not found in package, might be system one")
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${_LIBRARY_NAME})
            set(_CONAN_FOUND_SYSTEM_LIBS "${_CONAN_FOUND_SYSTEM_LIBS};${_LIBRARY_NAME}")
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()

    # Add all dependencies to all targets
    string(REPLACE " " ";" deps_list "${deps}")
    foreach(_CONAN_ACTUAL_TARGET ${_CONAN_ACTUAL_TARGETS})
        set_property(TARGET ${_CONAN_ACTUAL_TARGET} PROPERTY INTERFACE_LINK_LIBRARIES "${_CONAN_FOUND_SYSTEM_LIBS};${deps_list}")
    endforeach()

    set(${libraries_abs_path} ${CONAN_FULLPATH_LIBS} PARENT_SCOPE)
endfunction()


macro(conan_set_libcxx)
    if(DEFINED CONAN_LIBCXX)
        conan_message(STATUS "Conan: C++ stdlib: ${CONAN_LIBCXX}")
        if(CONAN_COMPILER STREQUAL "clang" OR CONAN_COMPILER STREQUAL "apple-clang")
            if(CONAN_LIBCXX STREQUAL "libstdc++" OR CONAN_LIBCXX STREQUAL "libstdc++11" )
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
            elseif(CONAN_LIBCXX STREQUAL "libc++")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            endif()
        endif()
        if(CONAN_COMPILER STREQUAL "sun-cc")
            if(CONAN_LIBCXX STREQUAL "libCstd")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=Cstd")
            elseif(CONAN_LIBCXX STREQUAL "libstdcxx")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stdcxx4")
            elseif(CONAN_LIBCXX STREQUAL "libstlport")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stlport4")
            elseif(CONAN_LIBCXX STREQUAL "libstdc++")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stdcpp")
            endif()
        endif()
        if(CONAN_LIBCXX STREQUAL "libstdc++11")
            add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
        elseif(CONAN_LIBCXX STREQUAL "libstdc++")
            add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
        endif()
    endif()
endmacro()


macro(conan_set_std)
    conan_message(STATUS "Conan: Adjusting language standard")
    # Do not warn "Manually-specified variables were not used by the project"
    set(ignorevar "${CONAN_STD_CXX_FLAG}${CONAN_CMAKE_CXX_STANDARD}${CONAN_CMAKE_CXX_EXTENSIONS}")
    if (CMAKE_VERSION VERSION_LESS "3.1" OR
        (CMAKE_VERSION VERSION_LESS "3.12" AND ("${CONAN_CMAKE_CXX_STANDARD}" STREQUAL "20" OR "${CONAN_CMAKE_CXX_STANDARD}" STREQUAL "gnu20")))
        if(CONAN_STD_CXX_FLAG)
            conan_message(STATUS "Conan setting CXX_FLAGS flags: ${CONAN_STD_CXX_FLAG}")
            set(CMAKE_CXX_FLAGS "${CONAN_STD_CXX_FLAG} ${CMAKE_CXX_FLAGS}")
        endif()
    else()
        if(CONAN_CMAKE_CXX_STANDARD)
            conan_message(STATUS "Conan setting CPP STANDARD: ${CONAN_CMAKE_CXX_STANDARD} WITH EXTENSIONS ${CONAN_CMAKE_CXX_EXTENSIONS}")
            set(CMAKE_CXX_STANDARD ${CONAN_CMAKE_CXX_STANDARD})
            set(CMAKE_CXX_EXTENSIONS ${CONAN_CMAKE_CXX_EXTENSIONS})
        endif()
    endif()
endmacro()


macro(conan_set_rpath)
    conan_message(STATUS "Conan: Adjusting default RPATHs Conan policies")
    if(APPLE)
        # https://cmake.org/Wiki/CMake_RPATH_handling
        # CONAN GUIDE: All generated libraries should have the id and dependencies to other
        # dylibs without path, just the name, EX:
        # libMyLib1.dylib:
        #     libMyLib1.dylib (compatibility version 0.0.0, current version 0.0.0)
        #     libMyLib0.dylib (compatibility version 0.0.0, current version 0.0.0)
        #     /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 120.0.0)
        #     /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1197.1.1)
        # AVOID RPATH FOR *.dylib, ALL LIBS BETWEEN THEM AND THE EXE
        # SHOULD BE ON THE LINKER RESOLVER PATH (./ IS ONE OF THEM)
        set(CMAKE_SKIP_RPATH 1 CACHE BOOL "rpaths" FORCE)
        # Policy CMP0068
        # We want the old behavior, in CMake >= 3.9 CMAKE_SKIP_RPATH won't affect the install_name in OSX
        set(CMAKE_INSTALL_NAME_DIR "")
    endif()
endmacro()


macro(conan_set_fpic)
    if(DEFINED CONAN_CMAKE_POSITION_INDEPENDENT_CODE)
        conan_message(STATUS "Conan: Adjusting fPIC flag (${CONAN_CMAKE_POSITION_INDEPENDENT_CODE})")
        set(CMAKE_POSITION_INDEPENDENT_CODE ${CONAN_CMAKE_POSITION_INDEPENDENT_CODE})
    endif()
endmacro()


macro(conan_output_dirs_setup)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endmacro()


macro(conan_split_version VERSION_STRING MAJOR MINOR)
    #make a list from the version string
    string(REPLACE "." ";" VERSION_LIST "${VERSION_STRING}")

    #write output values
    list(LENGTH VERSION_LIST _version_len)
    list(GET VERSION_LIST 0 ${MAJOR})
    if(${_version_len} GREATER 1)
        list(GET VERSION_LIST 1 ${MINOR})
    endif()
endmacro()


macro(conan_error_compiler_version)
    message(FATAL_ERROR "Detected a mismatch for the compiler version between your conan profile settings and CMake: \n"
                        "Compiler version specified in your conan profile: ${CONAN_COMPILER_VERSION}\n"
                        "Compiler version detected in CMake: ${VERSION_MAJOR}.${VERSION_MINOR}\n"
                        "Please check your conan profile settings (conan profile show [default|your_profile_name])\n"
                        "P.S. You may set CONAN_DISABLE_CHECK_COMPILER CMake variable in order to disable this check."
           )
endmacro()

set(_CONAN_CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR})

function(conan_get_compiler CONAN_INFO_COMPILER CONAN_INFO_COMPILER_VERSION)
    conan_message(STATUS "Current conanbuildinfo.cmake directory: " ${_CONAN_CURRENT_DIR})
    if(NOT EXISTS ${_CONAN_CURRENT_DIR}/conaninfo.txt)
        conan_message(STATUS "WARN: conaninfo.txt not found")
        return()
    endif()

    file (READ "${_CONAN_CURRENT_DIR}/conaninfo.txt" CONANINFO)

    # MATCHALL will match all, including the last one, which is the full_settings one
    string(REGEX MATCH "full_settings.*" _FULL_SETTINGS_MATCHED ${CONANINFO})
    string(REGEX MATCH "compiler=([-A-Za-z0-9_ ]+)" _MATCHED ${_FULL_SETTINGS_MATCHED})
    if(DEFINED CMAKE_MATCH_1)
        string(STRIP "${CMAKE_MATCH_1}" _CONAN_INFO_COMPILER)
        set(${CONAN_INFO_COMPILER} ${_CONAN_INFO_COMPILER} PARENT_SCOPE)
    endif()

    string(REGEX MATCH "compiler.version=([-A-Za-z0-9_.]+)" _MATCHED ${_FULL_SETTINGS_MATCHED})
    if(DEFINED CMAKE_MATCH_1)
        string(STRIP "${CMAKE_MATCH_1}" _CONAN_INFO_COMPILER_VERSION)
        set(${CONAN_INFO_COMPILER_VERSION} ${_CONAN_INFO_COMPILER_VERSION} PARENT_SCOPE)
    endif()
endfunction()


function(check_compiler_version)
    conan_split_version(${CMAKE_CXX_COMPILER_VERSION} VERSION_MAJOR VERSION_MINOR)
    if(DEFINED CONAN_SETTINGS_COMPILER_TOOLSET)
       conan_message(STATUS "Conan: Skipping compiler check: Declared 'compiler.toolset'")
       return()
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        # MSVC_VERSION is defined since 2.8.2 at least
        # https://cmake.org/cmake/help/v2.8.2/cmake.html#variable:MSVC_VERSION
        # https://cmake.org/cmake/help/v3.14/variable/MSVC_VERSION.html
        if(
            # 1920-1929 = VS 16.0 (v142 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "16" AND NOT((MSVC_VERSION GREATER 1919) AND (MSVC_VERSION LESS 1930))) OR
            # 1910-1919 = VS 15.0 (v141 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "15" AND NOT((MSVC_VERSION GREATER 1909) AND (MSVC_VERSION LESS 1920))) OR
            # 1900      = VS 14.0 (v140 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "14" AND NOT(MSVC_VERSION EQUAL 1900)) OR
            # 1800      = VS 12.0 (v120 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "12" AND NOT VERSION_MAJOR STREQUAL "18") OR
            # 1700      = VS 11.0 (v110 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "11" AND NOT VERSION_MAJOR STREQUAL "17") OR
            # 1600      = VS 10.0 (v100 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "10" AND NOT VERSION_MAJOR STREQUAL "16") OR
            # 1500      = VS  9.0 (v90 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "9" AND NOT VERSION_MAJOR STREQUAL "15") OR
            # 1400      = VS  8.0 (v80 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "8" AND NOT VERSION_MAJOR STREQUAL "14") OR
            # 1310      = VS  7.1, 1300      = VS  7.0
            (CONAN_COMPILER_VERSION STREQUAL "7" AND NOT VERSION_MAJOR STREQUAL "13") OR
            # 1200      = VS  6.0
            (CONAN_COMPILER_VERSION STREQUAL "6" AND NOT VERSION_MAJOR STREQUAL "12") )
            conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "gcc")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        set(_CHECK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
        set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
        if(NOT ${CONAN_COMPILER_VERSION} VERSION_LESS 5.0)
            conan_message(STATUS "Conan: Compiler GCC>=5, checking major version ${CONAN_COMPILER_VERSION}")
            conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
            if("${CONAN_COMPILER_MINOR}" STREQUAL "")
                set(_CHECK_VERSION ${VERSION_MAJOR})
                set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR})
            endif()
        endif()
        conan_message(STATUS "Conan: Checking correct version: ${_CHECK_VERSION}")
        if(NOT ${_CHECK_VERSION} VERSION_EQUAL ${_CONAN_VERSION})
            conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "clang")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        set(_CHECK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
        set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
        if(NOT ${CONAN_COMPILER_VERSION} VERSION_LESS 8.0)
            conan_message(STATUS "Conan: Compiler Clang>=8, checking major version ${CONAN_COMPILER_VERSION}")
            if("${CONAN_COMPILER_MINOR}" STREQUAL "")
                set(_CHECK_VERSION ${VERSION_MAJOR})
                set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR})
            endif()
        endif()
        conan_message(STATUS "Conan: Checking correct version: ${_CHECK_VERSION}")
        if(NOT ${_CHECK_VERSION} VERSION_EQUAL ${_CONAN_VERSION})
            conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "apple-clang" OR CONAN_COMPILER STREQUAL "sun-cc" OR CONAN_COMPILER STREQUAL "mcst-lcc")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        if(NOT ${VERSION_MAJOR}.${VERSION_MINOR} VERSION_EQUAL ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
           conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "intel")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        if(NOT ${CONAN_COMPILER_VERSION} VERSION_LESS 19.1)
            if(NOT ${VERSION_MAJOR}.${VERSION_MINOR} VERSION_EQUAL ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
               conan_error_compiler_version()
            endif()
        else()
            if(NOT ${VERSION_MAJOR} VERSION_EQUAL ${CONAN_COMPILER_MAJOR})
               conan_error_compiler_version()
            endif()
        endif()
    else()
        conan_message(STATUS "WARN: Unknown compiler '${CONAN_COMPILER}', skipping the version check...")
    endif()
endfunction()


function(conan_check_compiler)
    if(CONAN_DISABLE_CHECK_COMPILER)
        conan_message(STATUS "WARN: Disabled conan compiler checks")
        return()
    endif()
    if(NOT DEFINED CMAKE_CXX_COMPILER_ID)
        if(DEFINED CMAKE_C_COMPILER_ID)
            conan_message(STATUS "This project seems to be plain C, using '${CMAKE_C_COMPILER_ID}' compiler")
            set(CMAKE_CXX_COMPILER_ID ${CMAKE_C_COMPILER_ID})
            set(CMAKE_CXX_COMPILER_VERSION ${CMAKE_C_COMPILER_VERSION})
        else()
            message(FATAL_ERROR "This project seems to be plain C, but no compiler defined")
        endif()
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID AND NOT CMAKE_C_COMPILER_ID)
        # This use case happens when compiler is not identified by CMake, but the compilers are there and work
        conan_message(STATUS "*** WARN: CMake was not able to identify a C or C++ compiler ***")
        conan_message(STATUS "*** WARN: Disabling compiler checks. Please make sure your settings match your environment ***")
        return()
    endif()
    if(NOT DEFINED CONAN_COMPILER)
        conan_get_compiler(CONAN_COMPILER CONAN_COMPILER_VERSION)
        if(NOT DEFINED CONAN_COMPILER)
            conan_message(STATUS "WARN: CONAN_COMPILER variable not set, please make sure yourself that "
                          "your compiler and version matches your declared settings")
            return()
        endif()
    endif()

    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL ${CMAKE_SYSTEM_NAME})
        set(CROSS_BUILDING 1)
    endif()

    # If using VS, verify toolset
    if (CONAN_COMPILER STREQUAL "Visual Studio")
        if (CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "LLVM" OR
            CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "llvm" OR
            CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "clang" OR
            CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "Clang")
            set(EXPECTED_CMAKE_CXX_COMPILER_ID "Clang")
        elseif (CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "Intel")
            set(EXPECTED_CMAKE_CXX_COMPILER_ID "Intel")
        else()
            set(EXPECTED_CMAKE_CXX_COMPILER_ID "MSVC")
        endif()

        if (NOT CMAKE_CXX_COMPILER_ID MATCHES ${EXPECTED_CMAKE_CXX_COMPILER_ID})
            message(FATAL_ERROR "Incorrect '${CONAN_COMPILER}'. Toolset specifies compiler as '${EXPECTED_CMAKE_CXX_COMPILER_ID}' "
                                "but CMake detected '${CMAKE_CXX_COMPILER_ID}'")
        endif()

    # Avoid checks when cross compiling, apple-clang crashes because its APPLE but not apple-clang
    # Actually CMake is detecting "clang" when you are using apple-clang, only if CMP0025 is set to NEW will detect apple-clang
    elseif((CONAN_COMPILER STREQUAL "gcc" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR
        (CONAN_COMPILER STREQUAL "apple-clang" AND NOT CROSS_BUILDING AND (NOT APPLE OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")) OR
        (CONAN_COMPILER STREQUAL "clang" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR
        (CONAN_COMPILER STREQUAL "sun-cc" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "SunPro") )
        message(FATAL_ERROR "Incorrect '${CONAN_COMPILER}', is not the one detected by CMake: '${CMAKE_CXX_COMPILER_ID}'")
    endif()


    if(NOT DEFINED CONAN_COMPILER_VERSION)
        conan_message(STATUS "WARN: CONAN_COMPILER_VERSION variable not set, please make sure yourself "
                             "that your compiler version matches your declared settings")
        return()
    endif()
    check_compiler_version()
endfunction()


macro(conan_set_flags build_type)
    set(CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}} ${CONAN_CXX_FLAGS${build_type}}")
    set(CMAKE_C_FLAGS${build_type} "${CMAKE_C_FLAGS${build_type}} ${CONAN_C_FLAGS${build_type}}")
    set(CMAKE_SHARED_LINKER_FLAGS${build_type} "${CMAKE_SHARED_LINKER_FLAGS${build_type}} ${CONAN_SHARED_LINKER_FLAGS${build_type}}")
    set(CMAKE_EXE_LINKER_FLAGS${build_type} "${CMAKE_EXE_LINKER_FLAGS${build_type}} ${CONAN_EXE_LINKER_FLAGS${build_type}}")
endmacro()


macro(conan_global_flags)
    if(CONAN_SYSTEM_INCLUDES)
        include_directories(SYSTEM ${CONAN_INCLUDE_DIRS}
                                   "$<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_RELEASE}>"
                                   "$<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_RELWITHDEBINFO}>"
                                   "$<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_MINSIZEREL}>"
                                   "$<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DEBUG}>")
    else()
        include_directories(${CONAN_INCLUDE_DIRS}
                            "$<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_RELEASE}>"
                            "$<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_RELWITHDEBINFO}>"
                            "$<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_MINSIZEREL}>"
                            "$<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DEBUG}>")
    endif()

    link_directories(${CONAN_LIB_DIRS})

    conan_find_libraries_abs_path("${CONAN_LIBS_DEBUG}" "${CONAN_LIB_DIRS_DEBUG}"
                                  CONAN_LIBS_DEBUG)
    conan_find_libraries_abs_path("${CONAN_LIBS_RELEASE}" "${CONAN_LIB_DIRS_RELEASE}"
                                  CONAN_LIBS_RELEASE)
    conan_find_libraries_abs_path("${CONAN_LIBS_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_RELWITHDEBINFO}"
                                  CONAN_LIBS_RELWITHDEBINFO)
    conan_find_libraries_abs_path("${CONAN_LIBS_MINSIZEREL}" "${CONAN_LIB_DIRS_MINSIZEREL}"
                                  CONAN_LIBS_MINSIZEREL)

    add_compile_options(${CONAN_DEFINES}
                        "$<$<CONFIG:Debug>:${CONAN_DEFINES_DEBUG}>"
                        "$<$<CONFIG:Release>:${CONAN_DEFINES_RELEASE}>"
                        "$<$<CONFIG:RelWithDebInfo>:${CONAN_DEFINES_RELWITHDEBINFO}>"
                        "$<$<CONFIG:MinSizeRel>:${CONAN_DEFINES_MINSIZEREL}>")

    conan_set_flags("")
    conan_set_flags("_RELEASE")
    conan_set_flags("_DEBUG")

endmacro()


macro(conan_target_link_libraries target)
    if(CONAN_TARGETS)
        target_link_libraries(${target} ${CONAN_TARGETS})
    else()
        target_link_libraries(${target} ${CONAN_LIBS})
        foreach(_LIB ${CONAN_LIBS_RELEASE})
            target_link_libraries(${target} optimized ${_LIB})
        endforeach()
        foreach(_LIB ${CONAN_LIBS_DEBUG})
            target_link_libraries(${target} debug ${_LIB})
        endforeach()
    endif()
endmacro()


macro(conan_include_build_modules)
    if(CMAKE_BUILD_TYPE)
        if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_DEBUG} ${CONAN_BUILD_MODULES_PATHS})
        elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_RELEASE} ${CONAN_BUILD_MODULES_PATHS})
        elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_RELWITHDEBINFO} ${CONAN_BUILD_MODULES_PATHS})
        elseif(${CMAKE_BUILD_TYPE} MATCHES "MinSizeRel")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_MINSIZEREL} ${CONAN_BUILD_MODULES_PATHS})
        endif()
    endif()

    foreach(_BUILD_MODULE_PATH ${CONAN_BUILD_MODULES_PATHS})
        include(${_BUILD_MODULE_PATH})
    endforeach()
endmacro()


### Definition of user declared vars (user_info) ###

