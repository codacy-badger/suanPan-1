include_directories(${ROOT})
include_directories(Include)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Debug Release RelWithDebInfo MinSizeRel" FORCE)

option(BUILD_DLL_EXAMPLE "BUILD DYNAMIC LIBRARY EXAMPLE" ON)
option(BUILD_MULTITHREAD "BUILD MULTI THREADED VERSION" OFF)
option(BUILD_SHARED "BUILD SHARED LIBRARY" OFF)
option(USE_SUPERLUMT "USE MULTI THREADED SUPERLU" OFF)
option(USE_EXTERNAL_VTK "USE EXTERNAL VTK LIBRARY TO ENABLE VISUALIZATION" OFF)
option(USE_HDF5 "ENABLE HDF5 SUPPORT TO RECORD DATA" ON)
option(USE_AVX "USE AVX" ON)
option(USE_AVX2 "USE AVX2" OFF)
option(USE_MKL "USE INTEL MKL" OFF)

set(COMPILER_IDENTIFIER "unknown")
if(CMAKE_SYSTEM_NAME MATCHES "Windows") # WINDOWS PLATFORM
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU") # GNU GCC COMPILER
        set(COMPILER_IDENTIFIER "gcc-win")
    elseif((CMAKE_CXX_COMPILER_ID MATCHES "MSVC") OR (CMAKE_CXX_COMPILER_ID MATCHES "Intel")) # MSVC COMPILER
        set(COMPILER_IDENTIFIER "vs")
        add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
        if(FORTRAN_STATUS)
            set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
        endif()
        option(USE_EXTERNAL_CUDA "USE EXTERNAL CUDA LIBRARY TO UTILIZE GPU" OFF)
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux") # LINUX PLATFORM
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU") # GNU GCC COMPILER
        set(COMPILER_IDENTIFIER "gcc-linux")
    endif()
    option(USE_EXTERNAL_CUDA "USE EXTERNAL CUDA LIBRARY TO UTILIZE GPU" OFF)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin") # MAC PLATFORM
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU") # GNU GCC COMPILER
        set(COMPILER_IDENTIFIER "gcc-mac")
    endif()
endif()

if(COMPILER_IDENTIFIER MATCHES "unknown")
    message(FATAL_ERROR "Cannot identify the compiler available, please use GCC or MSVC or Intel.")
endif()

link_directories(Libs/${COMPILER_IDENTIFIER})

if(USE_SUPERLUMT)
    add_definitions(-DSUANPAN_SUPERLUMT)
endif()

if(USE_MKL)
    set(MKLROOT "" CACHE PATH "MKL library path which contains /include and /lib folders.")
    find_file(MKL_HEADER NAMES mkl.h PATHS ${MKLROOT}/include)
    if(MKL_HEADER MATCHES "MKL_HEADER-NOTFOUND")
        message(FATAL_ERROR "The mkl.h is not found under the path: ${MKLROOT}/include")
    endif()
    add_definitions(-DSUANPAN_MKL)
    include_directories(${MKLROOT}/include)
    link_directories(${MKLROOT}/lib/intel64)
    if(MKLROOT MATCHES "(oneapi|oneAPI)")
        if((COMPILER_IDENTIFIER MATCHES "gcc-linux") OR (COMPILER_IDENTIFIER MATCHES "gcc-mac"))
            find_library(IOMPPATH iomp5 ${MKLROOT}/../../compiler/latest/linux/compiler/lib/intel64_lin)
            get_filename_component(IOMPPATH ${IOMPPATH} DIRECTORY)
            link_directories(${IOMPPATH})
        elseif(COMPILER_IDENTIFIER MATCHES "vs")
            find_library(IOMPPATH libiomp5md ${MKLROOT}/../../compiler/latest/windows/compiler/lib/intel64_win)
            get_filename_component(IOMPPATH ${IOMPPATH} DIRECTORY)
            link_directories(${IOMPPATH})
        endif()
    else()
        if((COMPILER_IDENTIFIER MATCHES "gcc-linux") OR (COMPILER_IDENTIFIER MATCHES "gcc-mac"))
            find_library(IOMPPATH iomp5 ${MKLROOT}/../lib/intel64)
            get_filename_component(IOMPPATH ${IOMPPATH} DIRECTORY)
            link_directories(${IOMPPATH})
        elseif(COMPILER_IDENTIFIER MATCHES "vs")
            find_library(IOMPPATH libiomp5md ${MKLROOT}/../compiler/lib/intel64)
            get_filename_component(IOMPPATH ${IOMPPATH} DIRECTORY)
            link_directories(${IOMPPATH})
        endif()
    endif()
endif()

if(USE_EXTERNAL_CUDA)
    find_package(CUDA)
    if(NOT CUDA_FOUND)
        set(CUDA_PATH "" CACHE PATH "CUDA library path which contains /include folder")
        find_package(CUDA PATHS ${CUDA_PATH})
        if(NOT CUDA_FOUND)
            message(FATAL_ERROR "CUDA library is not found, please indicate its path.")
        endif()
    endif()
    add_definitions(-DSUANPAN_CUDA)
    include_directories(${CUDA_INCLUDE_DIRS})
    link_libraries(${CUDA_LIBRARIES})
endif()

set(HAVE_VTK FALSE CACHE INTERNAL "")
if(USE_EXTERNAL_VTK)
    if(VTK_PATH MATCHES "")
        find_package(VTK)
    else()
        find_package(VTK PATHS ${VTK_PATH})
    endif()
    if(VTK_FOUND)
        add_definitions(-DSUANPAN_VTK)
        set(HAVE_VTK TRUE CACHE INTERNAL "")
    else()
        set(VTK_PATH "" CACHE PATH "VTK library path which contains /include folder")
        find_package(VTK PATHS ${VTK_PATH})
        if(NOT VTK_FOUND)
            message(FATAL_ERROR "VTK library is not found, please indicate its path.")
        endif()
    endif()
endif()

if(USE_HDF5)
    add_definitions(-DSUANPAN_HDF5)
    include_directories(Include/hdf5)
    include_directories(Include/hdf5-${COMPILER_IDENTIFIER})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        link_libraries(hdf5_hl hdf5)
    else()
        link_libraries(libhdf5_hl libhdf5)
    endif()
else()
    add_definitions(-DARMA_DONT_USE_HDF5)
endif()

if(BUILD_MULTITHREAD)
    add_definitions(-DSUANPAN_MT)
    link_libraries(tbb tbbmalloc tbbmalloc_proxy)
endif()

if(COMPILER_IDENTIFIER MATCHES "vs")
    unset(TEST_COVERAGE CACHE)

    link_directories(Libs/gcc-win)

    set(CMAKE_CXX_FLAGS "/MP /openmp /EHsc")
    set(CMAKE_C_FLAGS "/MP /openmp /EHsc")
    set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:LIBCMT")
    set(CMAKE_Fortran_FLAGS "/MP /Qopenmp /fpp /names:lowercase /assume:underscore")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /libs:dll /threads")

    if(USE_AVX2)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /arch:AVX")
    elseif(USE_AVX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /arch:AVX")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    link_libraries(dl pthread gfortran quadmath)

    set(CMAKE_CXX_FLAGS "-fexceptions -fopenmp")
    set(CMAKE_C_FLAGS "-fexceptions -fopenmp")
    if(USE_AVX2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2")
    elseif(USE_AVX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx")
    endif()

    option(TEST_COVERAGE "TEST CODE COVERAGE USING GCOV" OFF)

    if(TEST_COVERAGE) # only report coverage with gcc
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        link_libraries(gcov)
    endif()

    set(CMAKE_Fortran_FLAGS "-cpp -fopenmp -w")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "10.0.0")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fallow-argument-mismatch")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0.0")
        link_libraries(stdc++fs) # for <filesystem>
    endif()
endif()
