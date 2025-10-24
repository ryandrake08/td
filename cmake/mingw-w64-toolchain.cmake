# CMake toolchain file for cross-compiling to Windows using MinGW-w64

# Target system
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Toolchain prefix
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# Compilers
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# Qt MinGW installation path
# Priority order:
#   1. CMake variable: cmake -DQT_MINGW_PATH=/path/to/qt ...
#   2. Environment variable: export QT_MINGW_PATH=/path/to/qt
#   3. Auto-detect in standard locations
if(NOT QT_MINGW_PATH)
    if(DEFINED ENV{QT_MINGW_PATH})
        set(QT_MINGW_PATH "$ENV{QT_MINGW_PATH}")
    else()
        # Search standard installation locations
        set(_QT_SEARCH_PATHS
            /opt/mingw-qt5
            /opt/qt5-mingw
            /usr/local/mingw-qt5
            /usr/local/qt5-mingw
        )

        foreach(_path ${_QT_SEARCH_PATHS})
            if(EXISTS "${_path}/lib/cmake/Qt5")
                set(QT_MINGW_PATH "${_path}")
                message(STATUS "Found Qt MinGW at: ${QT_MINGW_PATH}")
                break()
            endif()
        endforeach()

        if(NOT QT_MINGW_PATH)
            message(FATAL_ERROR
                "Qt MinGW not found. Install to /opt/mingw-qt5 or specify location:\n"
                "  cmake -DQT_MINGW_PATH=/path/to/qt ...\n"
                "Or set environment variable:\n"
                "  export QT_MINGW_PATH=/path/to/qt\n"
                "\n"
                "Download from: https://download.qt.io/official_releases/qt/5.15/")
        endif()
    endif()
endif()

set(CMAKE_PREFIX_PATH "${QT_MINGW_PATH}")

# Find root paths for cross-compilation
set(CMAKE_FIND_ROOT_PATH
    /usr/${TOOLCHAIN_PREFIX}
    ${QT_MINGW_PATH}
)

# Search mode settings
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Static linking for MinGW runtime
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")

# Windows-specific settings
set(WIN32 TRUE)
set(MINGW TRUE)
