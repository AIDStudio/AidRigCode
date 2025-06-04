# First check XMRIG_DEPS environment/parameter
find_path(
    UV_INCLUDE_DIR
    NAMES uv.h
    PATHS "${XMRIG_DEPS}" ENV "XMRIG_DEPS"
    PATH_SUFFIXES "include"
    NO_DEFAULT_PATH
)

# Then check system paths
if(NOT UV_INCLUDE_DIR)
    find_path(UV_INCLUDE_DIR NAMES uv.h)
endif()

# First check XMRIG_DEPS environment/parameter
find_library(
    UV_LIBRARY
    NAMES libuv.a uv libuv
    PATHS "${XMRIG_DEPS}" ENV "XMRIG_DEPS"
    PATH_SUFFIXES "lib"
    NO_DEFAULT_PATH
)

# Then check system paths
if(NOT UV_LIBRARY)
    find_library(UV_LIBRARY NAMES libuv.a uv libuv)
endif()

if(ANDROID AND XMRIG_DEPS)
    message(STATUS "Android build: Looking for UV in XMRIG_DEPS=${XMRIG_DEPS}")
    
    # For Android, we explicitly set the paths based on XMRIG_DEPS
    set(UV_INCLUDE_DIR "${XMRIG_DEPS}/include")
    set(UV_LIBRARY "${XMRIG_DEPS}/lib/libuv.a")
    
    if(EXISTS "${UV_INCLUDE_DIR}/uv.h" AND EXISTS "${UV_LIBRARY}")
        message(STATUS "Found UV headers at: ${UV_INCLUDE_DIR}")
        message(STATUS "Found UV library at: ${UV_LIBRARY}")
        set(UV_FOUND TRUE)
    else()
        message(FATAL_ERROR "UV not found in XMRIG_DEPS=${XMRIG_DEPS}")
    endif()
endif()

set(UV_LIBRARIES ${UV_LIBRARY})
set(UV_INCLUDE_DIRS ${UV_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UV DEFAULT_MSG UV_LIBRARY UV_INCLUDE_DIR)
