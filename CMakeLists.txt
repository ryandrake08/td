cmake_minimum_required(VERSION 3.10)
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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb -DDEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -DDEBUG=1")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -pg")

include_directories(thirdparty)

add_library(td STATIC tournamentd/bonjour.cpp tournamentd/bonjour.hpp tournamentd/datetime.cpp tournamentd/datetime.hpp tournamentd/gameinfo.cpp tournamentd/gameinfo.hpp tournamentd/logger.hpp tournamentd/outputdebugstringbuf.hpp tournamentd/scope_timer.hpp tournamentd/server.cpp tournamentd/server.hpp tournamentd/shared_instance.hpp tournamentd/socket.cpp tournamentd/socket.hpp tournamentd/socketstream.hpp tournamentd/stopwatch.hpp tournamentd/tournament.cpp tournamentd/tournament.hpp tournamentd/types.cpp tournamentd/types.hpp)
target_include_directories(td PRIVATE .)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable")
if(FALSE AND CLANG_TIDY_EXE)
  set_target_properties(td PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=.*td/tournamentd.*;-checks=-*,cert*,misc*,modernize*,-modernize-make-unique,-modernize-pass-by-value,performance*,readability*,-readability-else-after-return")
endif()

add_executable(tournamentd_test tournamentd/catch.cpp tournamentd/catch.hpp)
target_link_libraries(tournamentd_test td ${OS_LIBRARIES})

add_executable(tournamentd tournamentd/main.cpp tournamentd/program.cpp)
target_link_libraries(tournamentd td ${OS_LIBRARIES})

add_executable(tournamentctl tournamentd/main.cpp tournamentd/program_ctl.cpp)
target_link_libraries(tournamentctl td ${OS_LIBRARIES})

# Unit tests
add_executable(tournamentd_tests
    tournamentd/tests/test_main.cpp
    tournamentd/tests/test_tournament.cpp
    tournamentd/tests/test_types.cpp
    tournamentd/tests/test_datetime.cpp
    tournamentd/tests/test_socket.cpp
    tournamentd/tests/test_server.cpp
    tournamentd/tests/test_gameinfo.cpp
    tournamentd/tests/test_bonjour.cpp
    tournamentd/tests/test_integration.cpp
    thirdparty/catch2/catch_amalgamated.cpp
)
target_link_libraries(tournamentd_tests td ${OS_LIBRARIES})
target_include_directories(tournamentd_tests PRIVATE thirdparty/catch2)

find_package(Qt5 COMPONENTS Core Network Widgets)
if(Qt5Core_FOUND AND Qt5Network_FOUND AND Qt5Widgets_FOUND)
  find_package(Threads REQUIRED)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  qt5_wrap_cpp(TBUDDY_MOC TBQt/TBMainWindow.hpp TBQt/TournamentSession.hpp)
  qt5_wrap_ui(TBUDDY_UI TBQt/TBMainWindow.ui)
  qt5_add_resources(TBUDDY_RC TBQt/resources.qrc)
  add_executable(tbuddy TBQt/tbuddy.cpp TBQt/TBMainWindow.cpp TBQt/TBMainWindow.hpp TBQt/TBRuntimeError.cpp TBQt/TBRuntimeError.hpp TBQt/TournamentDaemon.cpp TBQt/TournamentDaemon.hpp TBQt/TournamentDocument.cpp TBQt/TournamentDocument.hpp TBQt/TournamentService.cpp TBQt/TournamentService.hpp TBQt/TournamentSession.cpp TBQt/TournamentSession.hpp ${TBUDDY_MOC} ${TBUDDY_UI} ${TBUDDY_RC})
  target_compile_definitions(tbuddy PRIVATE QT_NO_KEYWORDS)
  target_link_libraries(tbuddy td Qt5::Network Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT} ${OS_LIBRARIES})
endif()
