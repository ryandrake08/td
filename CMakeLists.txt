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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
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
target_compile_features(td PUBLIC cxx_std_11)

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
target_compile_features(tournamentd PUBLIC cxx_std_11)

add_executable(tournamentctl
	tournamentd/main.cpp
	tournamentd/program_ctl.cpp
	tournamentd/program.hpp
)
target_link_libraries(tournamentctl td ${OS_LIBRARIES})
target_compile_features(tournamentctl PUBLIC cxx_std_11)

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
	thirdparty/Catch2/catch.hpp
)
target_link_libraries(tournamentd_tests td ${OS_LIBRARIES})
target_compile_features(tournamentd_tests PUBLIC cxx_std_11)

# Configure Qt5 paths for different platforms
if(DEFINED MACOSX)
	# macOS: Add Homebrew Qt5 installation path
	list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt@5")
endif()

find_package(Qt5 COMPONENTS Core Network Widgets Multimedia)
if(Qt5Core_FOUND AND Qt5Network_FOUND AND Qt5Widgets_FOUND AND Qt5Multimedia_FOUND)
	# Define application metadata variables (shared between C++ and Info.plist)
	set(ORGANIZATION_NAME "HDna Studio")
	set(APPLICATION_VERSION "1.0")
	set(BUILD_VERSION "1")
	
	# Platform-specific icon file extensions
	if(APPLE)
		set(ICON_EXTENSION "icns")
	elseif(WIN32)
		set(ICON_EXTENSION "ico")
	else()
		set(ICON_EXTENSION "png")
	endif()
	
	# tbuddy configuration
	set(TBUDDY_APPLICATION_NAME "Poker Buddy")
	set(TBUDDY_BUNDLE_NAME "Poker Buddy")
	set(TBUDDY_BUNDLE_IDENTIFIER "com.hdnastudio.Poker-Buddy.qt")
	set(TBUDDY_BUNDLE_EXECUTABLE "tbuddy")
	set(TBUDDY_ICON_FILE "tbuddy.${ICON_EXTENSION}")
	
	# tbviewer configuration
	set(TBVIEWER_APPLICATION_NAME "Poker Remote")
	set(TBVIEWER_BUNDLE_NAME "Poker Remote")
	set(TBVIEWER_BUNDLE_IDENTIFIER "com.hdnastudio.Poker-Remote.qt")
	set(TBVIEWER_BUNDLE_EXECUTABLE "tbviewer")
	set(TBVIEWER_ICON_FILE "tbviewer.${ICON_EXTENSION}")
	
	# Configure Info.plist files from templates
	configure_file(TBQt/tbuddy-Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/tbuddy-Info.plist" @ONLY)
	configure_file(TBQt/tbviewer-Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/tbviewer-Info.plist" @ONLY)
	
	# Configure C++ header from template
	configure_file(TBQt/TBQtAppConfig.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/TBQtAppConfig.hpp" @ONLY)

	# Configure qmdnsengine library
	configure_file(thirdparty/qmdnsengine/qmdnsengine_export.h.in "${CMAKE_CURRENT_BINARY_DIR}/qmdnsengine_export.h")

	add_library(qmdnsengine STATIC
		"${CMAKE_CURRENT_BINARY_DIR}/qmdnsengine_export.h"
		thirdparty/qmdnsengine/include/qmdnsengine/abstractserver.h
		thirdparty/qmdnsengine/include/qmdnsengine/bitmap.h
		thirdparty/qmdnsengine/include/qmdnsengine/browser.h
		thirdparty/qmdnsengine/include/qmdnsengine/cache.h
		thirdparty/qmdnsengine/include/qmdnsengine/dns.h
		thirdparty/qmdnsengine/include/qmdnsengine/hostname.h
		thirdparty/qmdnsengine/include/qmdnsengine/mdns.h
		thirdparty/qmdnsengine/include/qmdnsengine/message.h
		thirdparty/qmdnsengine/include/qmdnsengine/prober.h
		thirdparty/qmdnsengine/include/qmdnsengine/provider.h
		thirdparty/qmdnsengine/include/qmdnsengine/query.h
		thirdparty/qmdnsengine/include/qmdnsengine/record.h
		thirdparty/qmdnsengine/include/qmdnsengine/resolver.h
		thirdparty/qmdnsengine/include/qmdnsengine/server.h
		thirdparty/qmdnsengine/include/qmdnsengine/service.h
		thirdparty/qmdnsengine/src/abstractserver.cpp
		thirdparty/qmdnsengine/src/bitmap.cpp
		thirdparty/qmdnsengine/src/browser.cpp
		thirdparty/qmdnsengine/src/cache.cpp
		thirdparty/qmdnsengine/src/dns.cpp
		thirdparty/qmdnsengine/src/hostname.cpp
		thirdparty/qmdnsengine/src/mdns.cpp
		thirdparty/qmdnsengine/src/message.cpp
		thirdparty/qmdnsengine/src/prober.cpp
		thirdparty/qmdnsengine/src/provider.cpp
		thirdparty/qmdnsengine/src/query.cpp
		thirdparty/qmdnsengine/src/record.cpp
		thirdparty/qmdnsengine/src/resolver.cpp
		thirdparty/qmdnsengine/src/server.cpp
		thirdparty/qmdnsengine/src/service.cpp
	)
	target_compile_features(qmdnsengine PUBLIC cxx_std_11)
	target_include_directories(qmdnsengine PUBLIC
		thirdparty/qmdnsengine/include
		"${CMAKE_CURRENT_BINARY_DIR}"
	)
	target_compile_definitions(qmdnsengine PRIVATE QMDNSENGINE_LIBRARY)

	# Generate MOC files for qmdnsengine
	qt5_wrap_cpp(QMDNSENGINE_MOC
		thirdparty/qmdnsengine/include/qmdnsengine/abstractserver.h
		thirdparty/qmdnsengine/include/qmdnsengine/browser.h
		thirdparty/qmdnsengine/include/qmdnsengine/cache.h
		thirdparty/qmdnsengine/include/qmdnsengine/hostname.h
		thirdparty/qmdnsengine/include/qmdnsengine/prober.h
		thirdparty/qmdnsengine/include/qmdnsengine/provider.h
		thirdparty/qmdnsengine/include/qmdnsengine/resolver.h
		thirdparty/qmdnsengine/include/qmdnsengine/server.h
		thirdparty/qmdnsengine/src/browser_p.h
		thirdparty/qmdnsengine/src/cache_p.h
		thirdparty/qmdnsengine/src/server_p.h
	)

	# Add MOC files to qmdnsengine target
	target_sources(qmdnsengine PRIVATE ${QMDNSENGINE_MOC})

	# Link qmdnsengine to Qt5 modules now that Qt5 is found
	target_link_libraries(qmdnsengine Qt5::Core Qt5::Network)

	find_package(Threads REQUIRED)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	qt5_wrap_cpp(TBUDDY_MOC
		TBQt/TBActionClockWidget.hpp
		TBQt/TBActionClockWindow.hpp
		TBQt/TBAnteTypeDelegate.hpp
		TBQt/TBAuthCodeDialog.hpp
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
		TBQt/TBScalableLabel.hpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/TBSeatingCompoundSortProxyModel.hpp
		TBQt/TBSeatingModel.hpp
		TBQt/SignalHandler.hpp
		TBQt/TBSoundPlayer.hpp
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
		TBQt/TBAuthCodeDialog.cpp
		TBQt/TBAuthCodeDialog.hpp
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
		TBQt/TBScalableLabel.cpp
		TBQt/TBScalableLabel.hpp
		TBQt/TBSeatingChartWindow.cpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/TBSeatingCompoundSortProxyModel.cpp
		TBQt/TBSeatingCompoundSortProxyModel.hpp
		TBQt/TBSeatingModel.cpp
		TBQt/TBSeatingModel.hpp
		TBQt/SignalHandler.cpp
		TBQt/SignalHandler.hpp
		TBQt/TBSoundPlayer.cpp
		TBQt/TBSoundPlayer.hpp
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
	target_include_directories(tbuddy PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(tbuddy td qmdnsengine Qt5::Network Qt5::Widgets Qt5::Multimedia ${CMAKE_THREAD_LIBS_INIT} ${OS_LIBRARIES})
	target_compile_features(tbuddy PUBLIC cxx_std_11)
	# Platform-specific target properties and icon handling
	if(APPLE)
		set_target_properties(tbuddy PROPERTIES 
			MACOSX_BUNDLE TRUE 
			MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/tbuddy-Info.plist
			MACOSX_BUNDLE_ICON_FILE ${TBUDDY_ICON_FILE}
		)
		
		# Copy icon file to macOS bundle
		add_custom_command(TARGET tbuddy POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/Resources/${TBUDDY_ICON_FILE}
			$<TARGET_BUNDLE_DIR:tbuddy>/Contents/Resources/${TBUDDY_ICON_FILE}
		)
	elseif(WIN32)
		# Windows icon handling (requires .ico file and resource script)
		# set_target_properties(tbuddy PROPERTIES WIN32_EXECUTABLE TRUE)
		# TODO: Add Windows resource script (.rc) with icon reference
	else()
		# Linux: Set application icon through desktop file or Qt resources
		# TODO: Add .desktop file generation with Icon= entry
	endif()

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
		TBQt/TBScalableLabel.hpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/SignalHandler.hpp
		TBQt/TBSoundPlayer.hpp
		TBQt/TBTableWidget.hpp
		TBQt/TBTournamentDisplayWindow.hpp
		TBQt/TBVariantListTableModel.hpp
		TBQt/TournamentBrowser.hpp
		TBQt/TBViewerMainWindow.hpp
		TBQt/TournamentConnection.hpp
		TBQt/TournamentSession.hpp)
	qt5_wrap_ui(TBVIEWER_UI
		TBQt/TBConnectToDialog.ui
		TBQt/TBSeatingChartWindow.ui
		TBQt/TBTournamentDisplayWindow.ui
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
		TBQt/TBScalableLabel.cpp
		TBQt/TBScalableLabel.hpp
		TBQt/TBSeatingChartWindow.cpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/SignalHandler.cpp
		TBQt/SignalHandler.hpp
		TBQt/TBSoundPlayer.cpp
		TBQt/TBSoundPlayer.hpp
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
		TBQt/TournamentBrowser.cpp
		TBQt/TournamentBrowser.hpp
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
	target_include_directories(tbviewer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(tbviewer td qmdnsengine Qt5::Network Qt5::Widgets Qt5::Multimedia ${CMAKE_THREAD_LIBS_INIT} ${OS_LIBRARIES})
	target_compile_features(tbviewer PUBLIC cxx_std_11)
	# Platform-specific target properties and icon handling
	if(APPLE)
		set_target_properties(tbviewer PROPERTIES 
			MACOSX_BUNDLE TRUE 
			MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/tbviewer-Info.plist
			MACOSX_BUNDLE_ICON_FILE ${TBVIEWER_ICON_FILE}
		)
		
		# Copy icon file to macOS bundle
		add_custom_command(TARGET tbviewer POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/Resources/${TBVIEWER_ICON_FILE}
			$<TARGET_BUNDLE_DIR:tbviewer>/Contents/Resources/${TBVIEWER_ICON_FILE}
		)
	elseif(WIN32)
		# Windows icon handling (requires .ico file and resource script)
		# set_target_properties(tbviewer PROPERTIES WIN32_EXECUTABLE TRUE)
		# TODO: Add Windows resource script (.rc) with icon reference
	else()
		# Linux: Set application icon through desktop file or Qt resources
		# TODO: Add .desktop file generation with Icon= entry
	endif()
endif()
