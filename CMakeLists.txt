cmake_minimum_required(VERSION 2.8.11)
project(td)

include(CMakeToolsHelpers OPTIONAL)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(MACOSX TRUE)
endif()

if(DEFINED MACOSX)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(CFNETWORK_LIBRARY CFNetwork)
	set(OS_LIBRARIES ${COREFOUNDATION_LIBRARY} ${CFNETWORK_LIBRARY})
else()
    find_library(AVAHI_LIBRARY-COMMON NAMES avahi-common)
    find_library(AVAHI_LIBRARY-CLIENT NAMES avahi-client)
	set(OS_LIBRARIES ${AVAHI_LIBRARY-COMMON} ${AVAHI_LIBRARY-CLIENT})
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb -DDEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -DDEBUG=1")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -pg")

add_library(cJSON STATIC cJSON/cJSON.c)
target_compile_options(cJSON PRIVATE -Wno-misleading-indentation)

add_library(td STATIC tournamentd/bonjour.cpp tournamentd/datetime.cpp tournamentd/gameinfo.cpp tournamentd/json.cpp tournamentd/scope_timer.cpp tournamentd/server.cpp tournamentd/socket.cpp tournamentd/tournament.cpp tournamentd/types.cpp)
target_include_directories(td PRIVATE .)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable")
if(FALSE AND CLANG_TIDY_EXE)
  set_target_properties(td PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=.*td/tournamentd.*;-checks=-*,cert*,misc*,modernize*,-modernize-make-unique,-modernize-pass-by-value,performance*,readability*,-readability-else-after-return")
endif()

add_executable(tournamentd_test tournamentd/catch.cpp)
target_link_libraries(tournamentd_test td cJSON ${OS_LIBRARIES})

add_executable(tournamentd tournamentd/main.cpp tournamentd/program.cpp)
target_link_libraries(tournamentd td cJSON ${OS_LIBRARIES})

add_executable(tournamentctl tournamentd/main.cpp tournamentd/program_ctl.cpp)
target_link_libraries(tournamentctl td cJSON ${OS_LIBRARIES})
