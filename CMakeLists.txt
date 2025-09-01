cmake_minimum_required(VERSION 3.10)
project(td)

include(CMakeToolsHelpers OPTIONAL)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(MACOSX TRUE)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(LINUX TRUE)
endif()

if(DEFINED MACOSX)
	find_library(COREFOUNDATION_LIBRARY CoreFoundation)
	find_library(CFNETWORK_LIBRARY CFNetwork)
	set(OS_LIBRARIES ${COREFOUNDATION_LIBRARY} ${CFNETWORK_LIBRARY})
elseif(DEFINED LINUX)
	find_library(AVAHI_LIBRARY-COMMON NAMES avahi-common)
	find_library(AVAHI_LIBRARY-CLIENT NAMES avahi-client)
	set(OS_LIBRARIES ${AVAHI_LIBRARY-COMMON} ${AVAHI_LIBRARY-CLIENT})
else()
	# For other platforms (Windows, etc.), no OS-specific libraries
	set(OS_LIBRARIES "")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb -DDEBUG=1 -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -DDEBUG=1 -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -pg")

include_directories(thirdparty)

add_library(td STATIC
	tournamentd/bonjour.cpp
	tournamentd/bonjour.hpp
	tournamentd/datetime.cpp
	tournamentd/datetime.hpp
	tournamentd/gameinfo.cpp
	tournamentd/gameinfo.hpp
	tournamentd/logger.hpp
	tournamentd/outputdebugstringbuf.hpp
	tournamentd/scope_timer.hpp
	tournamentd/server.cpp
	tournamentd/server.hpp
	tournamentd/shared_instance.hpp
	tournamentd/socket.cpp
	tournamentd/socket.hpp
	tournamentd/socketstream.hpp
	tournamentd/stopwatch.hpp
	tournamentd/tournament.cpp
	tournamentd/tournament.hpp
	tournamentd/types.cpp
	tournamentd/types.hpp
)
target_include_directories(td PRIVATE .)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable")
if(FALSE AND CLANG_TIDY_EXE)
	set_target_properties(td PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=.*td/tournamentd.*;-checks=-*,cert*,misc*,modernize*,-modernize-make-unique,-modernize-pass-by-value,performance*,readability*,-readability-else-after-return")
endif()

add_executable(tournamentd
	tournamentd/main.cpp
	tournamentd/program.cpp
	tournamentd/program.hpp
)
target_link_libraries(tournamentd td ${OS_LIBRARIES})

add_executable(tournamentctl
	tournamentd/main.cpp
	tournamentd/program_ctl.cpp
	tournamentd/program.hpp
)
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
	thirdparty/catch_amalgamated.cpp
	thirdparty/catch_amalgamated.hpp
)
target_link_libraries(tournamentd_tests td ${OS_LIBRARIES})

# Configure Qt5 paths for different platforms
if(DEFINED MACOSX)
	# macOS: Add Homebrew Qt5 installation path
	list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt@5")
endif()

find_package(Qt5 COMPONENTS Core Network Widgets QUIET)
if(Qt5Core_FOUND AND Qt5Network_FOUND AND Qt5Widgets_FOUND)
	find_package(Threads REQUIRED)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	qt5_wrap_cpp(TBUDDY_MOC
		TBQt/TBActionClockWidget.hpp
		TBQt/TBActionClockWindow.hpp
		TBQt/TBAnteTypeDelegate.hpp
		TBQt/TBBaseMainWindow.hpp
		TBQt/TBBlindLevelDelegate.hpp
		TBQt/TBBuddyMainWindow.hpp
		TBQt/TBChipDisplayDelegate.hpp
		TBQt/TBColorDisplayDelegate.hpp
		TBQt/TBDateEditDelegate.hpp
		TBQt/TBFlowLayout.hpp
		TBQt/TBFundingDetailsDelegate.hpp
		TBQt/TBFundingTypeDelegate.hpp
		TBQt/TBInvertableButton.hpp
		TBQt/TBInvertableImageLabel.hpp
		TBQt/TBManageButtonDelegate.hpp
		TBQt/TBMovementDialog.hpp
		TBQt/TBPlayersModel.hpp
		TBQt/TBResultsModel.hpp
		TBQt/TBRoundsModel.hpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/TBSeatingCompoundSortProxyModel.hpp
		TBQt/TBSeatingModel.hpp
		TBQt/TBSetupChipsWidget.hpp
		TBQt/TBSetupDevicesWidget.hpp
		TBQt/TBSetupDialog.hpp
		TBQt/TBSetupFundingWidget.hpp
		TBQt/TBSetupPayoutsWidget.hpp
		TBQt/TBSetupPlayersWidget.hpp
		TBQt/TBSetupRoundsWidget.hpp
		TBQt/TBSetupTablesWidget.hpp
		TBQt/TBSetupTabWidget.hpp
		TBQt/TBTableWidget.hpp
		TBQt/TBTournamentDisplayWindow.hpp
		TBQt/TBVariantListTableModel.hpp
		TBQt/TournamentConnection.hpp
		TBQt/TournamentDocument.hpp
		TBQt/TournamentSession.hpp)
	qt5_wrap_ui(TBUDDY_UI
		TBQt/TBBuddyMainWindow.ui
		TBQt/TBMovementDialog.ui
		TBQt/TBSeatingChartWindow.ui
		TBQt/TBSetupChipsWidget.ui
		TBQt/TBSetupDevicesWidget.ui
		TBQt/TBSetupDialog.ui
		TBQt/TBSetupFundingWidget.ui
		TBQt/TBSetupPayoutsWidget.ui
		TBQt/TBSetupPlayersWidget.ui
		TBQt/TBSetupRoundsWidget.ui
		TBQt/TBSetupTablesWidget.ui
		TBQt/TBTournamentDisplayWindow.ui)
	qt5_add_resources(TBUDDY_RC
		TBQt/TBQt.qrc)
	add_executable(tbuddy
		TBQt/tbuddy.cpp
		TBQt/TBActionClockWidget.cpp
		TBQt/TBActionClockWidget.hpp
		TBQt/TBActionClockWindow.cpp
		TBQt/TBActionClockWindow.hpp
		TBQt/TBAnteTypeDelegate.cpp
		TBQt/TBAnteTypeDelegate.hpp
		TBQt/TBBaseMainWindow.cpp
		TBQt/TBBaseMainWindow.hpp
		TBQt/TBBlindLevelDelegate.cpp
		TBQt/TBBlindLevelDelegate.hpp
		TBQt/TBBuddyMainWindow.cpp
		TBQt/TBBuddyMainWindow.hpp
		TBQt/TBChipDisplayDelegate.cpp
		TBQt/TBChipDisplayDelegate.hpp
		TBQt/TBColorDisplayDelegate.cpp
		TBQt/TBColorDisplayDelegate.hpp
		TBQt/TBCurrency.cpp
		TBQt/TBCurrency.hpp
		TBQt/TBDateEditDelegate.cpp
		TBQt/TBDateEditDelegate.hpp
		TBQt/TBFlowLayout.cpp
		TBQt/TBFlowLayout.hpp
		TBQt/TBFundingDetailsDelegate.cpp
		TBQt/TBFundingDetailsDelegate.hpp
		TBQt/TBFundingTypeDelegate.cpp
		TBQt/TBFundingTypeDelegate.hpp
		TBQt/TBImageInversionHelper.cpp
		TBQt/TBImageInversionHelper.hpp
		TBQt/TBInvertableButton.cpp
		TBQt/TBInvertableButton.hpp
		TBQt/TBInvertableImageLabel.cpp
		TBQt/TBInvertableImageLabel.hpp
		TBQt/TBManageButtonDelegate.cpp
		TBQt/TBManageButtonDelegate.hpp
		TBQt/TBMovementDialog.cpp
		TBQt/TBMovementDialog.hpp
		TBQt/TBPlayersModel.cpp
		TBQt/TBPlayersModel.hpp
		TBQt/TBResultsModel.cpp
		TBQt/TBResultsModel.hpp
		TBQt/TBRoundsModel.cpp
		TBQt/TBRoundsModel.hpp
		TBQt/TBRuntimeError.cpp
		TBQt/TBRuntimeError.hpp
		TBQt/TBSeatingChartWindow.cpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/TBSeatingCompoundSortProxyModel.cpp
		TBQt/TBSeatingCompoundSortProxyModel.hpp
		TBQt/TBSeatingModel.cpp
		TBQt/TBSeatingModel.hpp
		TBQt/TBSetupChipsWidget.cpp
		TBQt/TBSetupChipsWidget.hpp
		TBQt/TBSetupDevicesWidget.cpp
		TBQt/TBSetupDevicesWidget.hpp
		TBQt/TBSetupDialog.cpp
		TBQt/TBSetupDialog.hpp
		TBQt/TBSetupFundingWidget.cpp
		TBQt/TBSetupFundingWidget.hpp
		TBQt/TBSetupPayoutsWidget.cpp
		TBQt/TBSetupPayoutsWidget.hpp
		TBQt/TBSetupPlayersWidget.cpp
		TBQt/TBSetupPlayersWidget.hpp
		TBQt/TBSetupRoundsWidget.cpp
		TBQt/TBSetupRoundsWidget.hpp
		TBQt/TBSetupTablesWidget.cpp
		TBQt/TBSetupTablesWidget.hpp
		TBQt/TBSetupTabWidget.cpp
		TBQt/TBSetupTabWidget.hpp
		TBQt/TBTableViewUtils.cpp
		TBQt/TBTableViewUtils.hpp
		TBQt/TBTableWidget.cpp
		TBQt/TBTableWidget.hpp
		TBQt/TBTournamentDisplayWindow.cpp
		TBQt/TBTournamentDisplayWindow.hpp
		TBQt/TBVariantListTableModel.cpp
		TBQt/TBVariantListTableModel.hpp
		TBQt/TournamentConnection.cpp
		TBQt/TournamentConnection.hpp
		TBQt/TournamentDaemon.cpp
		TBQt/TournamentDaemon.hpp
		TBQt/TournamentDocument.cpp
		TBQt/TournamentDocument.hpp
		TBQt/TournamentService.cpp
		TBQt/TournamentService.hpp
		TBQt/TournamentSession.cpp
		TBQt/TournamentSession.hpp
		${TBUDDY_MOC}
		${TBUDDY_UI}
		${TBUDDY_RC}
	)
	target_compile_definitions(tbuddy PRIVATE QT_NO_KEYWORDS)
	target_link_libraries(tbuddy td Qt5::Network Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT} ${OS_LIBRARIES})
	set_target_properties(tbuddy PROPERTIES MACOSX_BUNDLE TRUE MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/TBQt/TBQt-Info.plist)

	qt5_wrap_cpp(TBVIEWER_MOC
		TBQt/TBActionClockWidget.hpp
		TBQt/TBActionClockWindow.hpp
		TBQt/TBBaseMainWindow.hpp
		TBQt/TBChipDisplayDelegate.hpp
		TBQt/TBConnectToDialog.hpp
		TBQt/TBFlowLayout.hpp
		TBQt/TBInvertableButton.hpp
		TBQt/TBInvertableImageLabel.hpp
		TBQt/TBPlayersModel.hpp
		TBQt/TBResultsModel.hpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/TBTableWidget.hpp
		TBQt/TBTournamentDisplayWindow.hpp
		TBQt/TBVariantListTableModel.hpp
		TBQt/TBViewerMainWindow.hpp
		TBQt/TournamentConnection.hpp
		TBQt/TournamentSession.hpp)
	qt5_wrap_ui(TBVIEWER_UI
		TBQt/TBConnectToDialog.ui
		TBQt/TBSeatingChartWindow.ui
		TBQt/TBViewerMainWindow.ui)
	qt5_add_resources(TBVIEWER_RC
		TBQt/TBQt.qrc)
	add_executable(tbviewer
		TBQt/tbviewer.cpp
		TBQt/TBActionClockWidget.cpp
		TBQt/TBActionClockWidget.hpp
		TBQt/TBActionClockWindow.cpp
		TBQt/TBActionClockWindow.hpp
		TBQt/TBBaseMainWindow.cpp
		TBQt/TBBaseMainWindow.hpp
		TBQt/TBChipDisplayDelegate.cpp
		TBQt/TBChipDisplayDelegate.hpp
		TBQt/TBConnectToDialog.cpp
		TBQt/TBConnectToDialog.hpp
		TBQt/TBCurrency.cpp
		TBQt/TBCurrency.hpp
		TBQt/TBFlowLayout.cpp
		TBQt/TBFlowLayout.hpp
		TBQt/TBImageInversionHelper.cpp
		TBQt/TBImageInversionHelper.hpp
		TBQt/TBInvertableButton.cpp
		TBQt/TBInvertableButton.hpp
		TBQt/TBInvertableImageLabel.cpp
		TBQt/TBInvertableImageLabel.hpp
		TBQt/TBPlayersModel.cpp
		TBQt/TBPlayersModel.hpp
		TBQt/TBResultsModel.cpp
		TBQt/TBResultsModel.hpp
		TBQt/TBRuntimeError.cpp
		TBQt/TBRuntimeError.hpp
		TBQt/TBSeatingChartWindow.cpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/TBTableViewUtils.cpp
		TBQt/TBTableViewUtils.hpp
		TBQt/TBTableWidget.cpp
		TBQt/TBTableWidget.hpp
		TBQt/TBTournamentDisplayWindow.cpp
		TBQt/TBTournamentDisplayWindow.hpp
		TBQt/TBVariantListTableModel.cpp
		TBQt/TBVariantListTableModel.hpp
		TBQt/TBViewerMainWindow.cpp
		TBQt/TBViewerMainWindow.hpp
		TBQt/TournamentConnection.cpp
		TBQt/TournamentConnection.hpp
		TBQt/TournamentDaemon.cpp
		TBQt/TournamentDaemon.hpp
		TBQt/TournamentService.cpp
		TBQt/TournamentService.hpp
		TBQt/TournamentSession.cpp
		TBQt/TournamentSession.hpp
		${TBVIEWER_MOC}
		${TBVIEWER_UI}
		${TBVIEWER_RC}
	)
	target_compile_definitions(tbviewer PRIVATE QT_NO_KEYWORDS)
	target_link_libraries(tbviewer td Qt5::Network Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT} ${OS_LIBRARIES})
	set_target_properties(tbviewer PROPERTIES MACOSX_BUNDLE TRUE MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/TBQt/TBQt-Info.plist)
endif()
