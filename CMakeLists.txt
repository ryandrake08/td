cmake_minimum_required(VERSION 3.10)
project(td)

include(CMakeToolsHelpers OPTIONAL)

if(APPLE)
	find_library(COREFOUNDATION_LIBRARY CoreFoundation)
	find_library(CFNETWORK_LIBRARY CFNetwork)
	set(OS_LIBRARIES ${COREFOUNDATION_LIBRARY} ${CFNETWORK_LIBRARY})
elseif(WIN32)
	# TBD for Windows, for now no OS-specific libraries
	set(OS_LIBRARIES "")
else()
	# Assume Linux
	find_library(AVAHI_LIBRARY-COMMON NAMES avahi-common)
	find_library(AVAHI_LIBRARY-CLIENT NAMES avahi-client)
	set(OS_LIBRARIES ${AVAHI_LIBRARY-COMMON} ${AVAHI_LIBRARY-CLIENT})
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
target_compile_features(td PUBLIC cxx_std_11)

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
if(APPLE)
	# macOS: Add Homebrew Qt5 installation path
	list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt@5")
endif()

# Force Qt to use shared libraries for license compliance
set(Qt5_USE_STATIC_LIBS OFF)
set(Qt5_USE_STATIC_RUNTIME OFF)

find_package(Qt5 COMPONENTS Core Network Widgets Multimedia)
if(Qt5Core_FOUND AND Qt5Network_FOUND AND Qt5Widgets_FOUND AND Qt5Multimedia_FOUND)
	# Define application metadata variables (shared between C++ and Info.plist)
	set(ORGANIZATION_NAME "HDna Studio")
	set(APPLICATION_VERSION "1.0")
	set(BUILD_VERSION "1")

	# tbuddy configuration
	set(TBUDDY_APPLICATION_NAME "Poker Buddy")
	set(TBUDDY_BUNDLE_NAME "Poker Buddy")
	set(TBUDDY_BUNDLE_IDENTIFIER "com.hdnastudio.Poker-Buddy.qt")
	set(TBUDDY_BUNDLE_EXECUTABLE "tbuddy")

	# tbviewer configuration
	set(TBVIEWER_APPLICATION_NAME "Poker Remote")
	set(TBVIEWER_BUNDLE_NAME "Poker Remote")
	set(TBVIEWER_BUNDLE_IDENTIFIER "com.hdnastudio.Poker-Remote.qt")
	set(TBVIEWER_BUNDLE_EXECUTABLE "tbviewer")

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

	# Resource generation system - replaces tools/make-resources
	# Set build resources directory
	set(BUILD_RESOURCES_DIR "${CMAKE_BINARY_DIR}/Resources")

	# Find required tools for resource generation
	find_program(PDF2SVG_EXECUTABLE pdf2svg DOC "PDF to SVG converter")
	find_program(MAGICK_EXECUTABLE magick DOC "ImageMagick image processing tool")
	if(NOT MAGICK_EXECUTABLE)
		# Try the legacy 'convert' command
		find_program(MAGICK_EXECUTABLE convert DOC "ImageMagick convert tool (legacy)")
	endif()
	# Find icon-generating tool
	find_program(ICONUTIL_EXECUTABLE iconutil DOC "macOS icon utility")
	if(NOT ICONUTIL_EXECUTABLE)
		find_program(ICONUTIL_EXECUTABLE icnsutil DOC "Linux ICNS utility")
	endif()

	# Function to convert PDF to SVG with light/dark theme variants
	function(add_pdf_to_svg_icon pdf_file svg_base_name)
		if(PDF2SVG_EXECUTABLE)
			# Output paths
			set(LIGHT_SVG "${BUILD_RESOURCES_DIR}/icons/light_theme/svg/${svg_base_name}.svg")
			set(DARK_SVG "${BUILD_RESOURCES_DIR}/icons/dark_theme/svg/${svg_base_name}.svg")
			set(TEMP_SVG "${BUILD_RESOURCES_DIR}/temp_${svg_base_name}.svg")

			add_custom_command(
				OUTPUT "${LIGHT_SVG}" "${DARK_SVG}"
				# Create output directories
				COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_RESOURCES_DIR}/icons/light_theme/svg"
				COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_RESOURCES_DIR}/icons/dark_theme/svg"
				# Convert PDF to temporary SVG
				COMMAND "${PDF2SVG_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/${pdf_file}" "${TEMP_SVG}"
				# Convert stroke colors to theme colors
				COMMAND "${CMAKE_SOURCE_DIR}/tools/make-light-and-dark-svg" "${TEMP_SVG}" "${LIGHT_SVG}" "${DARK_SVG}"
				# Cleanup
				COMMAND ${CMAKE_COMMAND} -E remove "${TEMP_SVG}"
				DEPENDS "${CMAKE_SOURCE_DIR}/${pdf_file}"
				COMMENT "Generating SVG icons from ${pdf_file}"
			)

			# Add files to a custom target for dependency tracking
			list(APPEND QRC_RESOURCE_FILES "${LIGHT_SVG}" "${DARK_SVG}")
			set(QRC_RESOURCE_FILES "${QRC_RESOURCE_FILES}" PARENT_SCOPE)
		else()
			message(WARNING "pdf2svg or xmlstarlet not found - SVG icons ${svg_base_name} will not be generated")
		endif()
	endfunction()

	# Function to convert AI file to ICNS icon (macOS app icons)
	function(add_pdf_to_icns_icon pdf_file icns_name)
		if(MAGICK_EXECUTABLE AND ICONUTIL_EXECUTABLE)
			# Output path
			set(ICNS_FILE "${BUILD_RESOURCES_DIR}/${icns_name}.icns")
			set(ICONSET_DIR "${CMAKE_BINARY_DIR}/temp_${icns_name}.iconset")

			add_custom_command(
				OUTPUT "${ICNS_FILE}"
				# Create output directory
				COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_RESOURCES_DIR}"
				# Create temporary iconset directory
				COMMAND ${CMAKE_COMMAND} -E make_directory "${ICONSET_DIR}"
				# Convert pdf to png icons of various sizes
				COMMAND "${MAGICK_EXECUTABLE}" -density 576 "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${ICONSET_DIR}/icon_1024x1024.png"
				COMMAND "${MAGICK_EXECUTABLE}" -density 288 "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${ICONSET_DIR}/icon_512x512.png"
				COMMAND "${MAGICK_EXECUTABLE}" -density 144 "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${ICONSET_DIR}/icon_256x256.png"
				COMMAND "${MAGICK_EXECUTABLE}" -density 72  "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${ICONSET_DIR}/icon_128x128.png"
				COMMAND "${MAGICK_EXECUTABLE}" -density 36  "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${ICONSET_DIR}/icon_64x64.png"
				COMMAND "${MAGICK_EXECUTABLE}" -density 18  "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${ICONSET_DIR}/icon_32x32.png"
				COMMAND "${MAGICK_EXECUTABLE}" -density 9   "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${ICONSET_DIR}/icon_16x16.png"
				# Copy/Rename to produce @2x resoultion versions
				COMMAND ${CMAKE_COMMAND} -E rename "${ICONSET_DIR}/icon_1024x1024.png" "${ICONSET_DIR}/icon_512x512@2x.png"
				COMMAND ${CMAKE_COMMAND} -E copy "${ICONSET_DIR}/icon_512x512.png" "${ICONSET_DIR}/icon_256x256@2x.png"
				COMMAND ${CMAKE_COMMAND} -E copy "${ICONSET_DIR}/icon_256x256.png" "${ICONSET_DIR}/icon_128x128@2x.png"
				COMMAND ${CMAKE_COMMAND} -E rename "${ICONSET_DIR}/icon_64x64.png" "${ICONSET_DIR}/icon_32x32@2x.png"
				COMMAND ${CMAKE_COMMAND} -E copy "${ICONSET_DIR}/icon_32x32.png" "${ICONSET_DIR}/icon_16x16@2x.png"
				# Convert iconset to ICNS using iconutil
				COMMAND "${ICONUTIL_EXECUTABLE}" -c icns -o "${ICNS_FILE}" "${ICONSET_DIR}"
				# Cleanup temporary files
				COMMAND ${CMAKE_COMMAND} -E remove_directory "${ICONSET_DIR}"
				DEPENDS "${CMAKE_SOURCE_DIR}/${pdf_file}"
				COMMENT "Generating ICNS icon from ${pdf_file}"
			)
		else()
			message(WARNING "ImageMagick or iconutil not found - ICNS icon ${icns_name} will not be generated")
		endif()
	endfunction()

	# Function to convert PDF file to PNG icon (Linux app icons)
	function(add_pdf_to_png_icon pdf_file png_name)
		if(MAGICK_EXECUTABLE)
			# Output path
			set(PNG_FILE "${BUILD_RESOURCES_DIR}/${png_name}.png")

			add_custom_command(
				OUTPUT "${PNG_FILE}"
				# Create output directory
				COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_RESOURCES_DIR}"
				# Convert pdf to png icon
				COMMAND "${MAGICK_EXECUTABLE}" -density 576 "${CMAKE_SOURCE_DIR}/${pdf_file}" -depth 8 -colors 256 -strip "${PNG_FILE}"
				DEPENDS "${CMAKE_SOURCE_DIR}/${pdf_file}"
				COMMENT "Generating PNG icon from ${pdf_file}"
			)
		else()
			message(WARNING "ImageMagick not found - PNG icon ${png_name} will not be generated")
		endif()
	endfunction()

	# Generate all ICNS icons from AI files (matches tools/make-resources)
	add_pdf_to_icns_icon("Art/piranha_grad.pdf" "tbuddy")
	add_pdf_to_icns_icon("Art/piranha_grad.pdf" "tbviewer")
	add_pdf_to_icns_icon("Art/piranha_bare.pdf" "tbfile")

	# Generate all PNG icons from AI files (matches tools/make-resources)
	add_pdf_to_png_icon("Art/piranha_bare.pdf" "tbuddy")
	add_pdf_to_png_icon("Art/piranha_bare.pdf" "tbviewer")
	add_pdf_to_png_icon("Art/piranha_bare.pdf" "tbfile")

	# Generate all SVG icons from PDF files (matches tools/make-resources)
	set(QRC_RESOURCE_FILES "")
	add_pdf_to_svg_icon("Art/combination_lock.pdf" "b_auth")
	add_pdf_to_svg_icon("Art/past.pdf" "b_call_clock")
	add_pdf_to_svg_icon("Art/checkbox.pdf" "b_configure")
	add_pdf_to_svg_icon("Art/tv_demand.pdf" "b_display")
	add_pdf_to_svg_icon("Art/file_send.pdf" "b_export")
	add_pdf_to_svg_icon("Art/walking.pdf" "b_move")
	add_pdf_to_svg_icon("Art/new.pdf" "b_new")
	add_pdf_to_svg_icon("Art/controls_forward.pdf" "b_next")
	add_pdf_to_svg_icon("Art/note_dollar.pdf" "b_note_dollar")
	add_pdf_to_svg_icon("Art/note_euro.pdf" "b_note_euro")
	add_pdf_to_svg_icon("Art/note_rupee.pdf" "b_note_rupee")
	add_pdf_to_svg_icon("Art/note_sterling.pdf" "b_note_sterling")
	add_pdf_to_svg_icon("Art/note_yen_yuan.pdf" "b_note_yen_yuan")
	add_pdf_to_svg_icon("Art/controls_end.pdf" "b_play_pause")
	add_pdf_to_svg_icon("Art/controls_rewind.pdf" "b_previous")
	add_pdf_to_svg_icon("Art/scales_justice.pdf" "b_rebalance")
	add_pdf_to_svg_icon("Art/repeat.pdf" "b_restart")
	add_pdf_to_svg_icon("Art/rocket.pdf" "b_rocket")
	add_pdf_to_svg_icon("Art/armchair.pdf" "b_seat")
	add_pdf_to_svg_icon("Art/b_seating.pdf" "b_seating")
	add_pdf_to_svg_icon("Art/controls_stop.pdf" "b_stop")
	add_pdf_to_svg_icon("Art/table.pdf" "b_table")
	add_pdf_to_svg_icon("Art/i_chip.pdf" "i_chip")
	add_pdf_to_svg_icon("Art/piranha_bare.pdf" "i_application")
	add_pdf_to_svg_icon("Art/redo_arrow.pdf" "m_addon")
	add_pdf_to_svg_icon("Art/arrows_in.pdf" "m_buyin")
	add_pdf_to_svg_icon("Art/reset.pdf" "m_rebuy")
	add_pdf_to_svg_icon("Art/coins.pdf" "t_chips")
	add_pdf_to_svg_icon("Art/clock.pdf" "t_clock")
	add_pdf_to_svg_icon("Art/bullet_list.pdf" "t_connect")
	add_pdf_to_svg_icon("Art/mobile_monitor.pdf" "t_devices")
	add_pdf_to_svg_icon("Art/notes_coins.pdf" "t_funding")
	add_pdf_to_svg_icon("Art/dollar_bag.pdf" "t_payouts")
	add_pdf_to_svg_icon("Art/users_three.pdf" "t_players")
	add_pdf_to_svg_icon("Art/winners_podium.pdf" "t_results")
	add_pdf_to_svg_icon("Art/timer.pdf" "t_rounds")

	# Copy static resources to build directory
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/IBMPlexSans-Bold.otf"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Resources/IBMPlexSans-Bold.otf" "${CMAKE_BINARY_DIR}/Resources/IBMPlexSans-Bold.otf"
		DEPENDS "${CMAKE_SOURCE_DIR}/Resources/IBMPlexSans-Bold.otf"
		COMMENT "Copying IBMPlexSans-Bold.otf to build directory"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/IBMPlexSans-Bold.otf")

	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/IBMPlexSans-Regular.otf"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Resources/IBMPlexSans-Regular.otf" "${CMAKE_BINARY_DIR}/Resources/IBMPlexSans-Regular.otf"
		DEPENDS "${CMAKE_SOURCE_DIR}/Resources/IBMPlexSans-Regular.otf"
		COMMENT "Copying IBMPlexSans-Regular.otf to build directory"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/IBMPlexSans-Regular.otf")

	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/s_break.wav"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Art/Sonar Song.wav" "${CMAKE_BINARY_DIR}/Resources/s_break.wav"
		DEPENDS "${CMAKE_SOURCE_DIR}/Art/Sonar Song.wav"
		COMMENT "Copying Sonar Song.wav from Art/ to s_break.wav"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/s_break.wav")

	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/s_next.wav"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Art/Sonar Descent.wav" "${CMAKE_BINARY_DIR}/Resources/s_next.wav"
		DEPENDS "${CMAKE_SOURCE_DIR}/Art/Sonar Descent.wav"
		COMMENT "Copying Sonar Descent.wav from Art/ to s_next.wav"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/s_next.wav")

	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/s_rebalance.wav"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Art/Fancy Toast.wav" "${CMAKE_BINARY_DIR}/Resources/s_rebalance.wav"
		DEPENDS "${CMAKE_SOURCE_DIR}/Art/Fancy Toast.wav"
		COMMENT "Copying Fancy Toast.wav from Art/ to s_rebalance.wav"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/s_rebalance.wav")

	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/s_start.wav"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Art/Double Sonar Ping.wav" "${CMAKE_BINARY_DIR}/Resources/s_start.wav"
		DEPENDS "${CMAKE_SOURCE_DIR}/Art/Double Sonar Ping.wav"
		COMMENT "Copying Double Sonar Ping.wav from Art/ to s_start.wav"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/s_start.wav")

	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/s_warning.wav"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Art/Sonar Ping.wav" "${CMAKE_BINARY_DIR}/Resources/s_warning.wav"
		DEPENDS "${CMAKE_SOURCE_DIR}/Art/Sonar Ping.wav"
		COMMENT "Copying Sonar Ping.wav from Art/ to s_warning.wav"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/s_warning.wav")

	# Copy theme index files to build directory
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/icons/light_theme/index.theme"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources/icons/light_theme"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Resources/light.theme" "${CMAKE_BINARY_DIR}/Resources/icons/light_theme/index.theme"
		DEPENDS "${CMAKE_SOURCE_DIR}/Resources/light.theme"
		COMMENT "Copying light theme index to build directory"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/icons/light_theme/index.theme")

	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/Resources/icons/dark_theme/index.theme"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources/icons/dark_theme"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Resources/dark.theme" "${CMAKE_BINARY_DIR}/Resources/icons/dark_theme/index.theme"
		DEPENDS "${CMAKE_SOURCE_DIR}/Resources/dark.theme"
		COMMENT "Copying dark theme index to build directory"
	)
	list(APPEND QRC_RESOURCE_FILES "${CMAKE_BINARY_DIR}/Resources/icons/dark_theme/index.theme")

	# Copy QRC file to build directory
	set(COPIED_QRC_FILE "${CMAKE_BINARY_DIR}/Resources/TBQt.qrc")
	add_custom_command(
		OUTPUT "${COPIED_QRC_FILE}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Resources/TBQt.qrc" "${COPIED_QRC_FILE}"
		DEPENDS "${CMAKE_SOURCE_DIR}/Resources/TBQt.qrc"
		COMMENT "Generating TBQt.qrc in build directory"
	)

	# Create shared TBQt library with common functionality
	qt5_wrap_cpp(TBQT_COMMON_MOC
		TBQt/TBActionClockWidget.hpp
		TBQt/TBActionClockWindow.hpp
		TBQt/TBBaseAuxiliaryWindow.hpp
		TBQt/TBBaseMainWindow.hpp
		TBQt/TBChipDisplayDelegate.hpp
		TBQt/TBFlowLayout.hpp
		TBQt/TBPlayersModel.hpp
		TBQt/TBResultsModel.hpp
		TBQt/TBScalableLabel.hpp
		TBQt/TBSeatingChartWindow.hpp
		TBQt/TBSettingsDialog.hpp
		TBQt/SignalHandler.hpp
		TBQt/TBSoundPlayer.hpp
		TBQt/TBTableWidget.hpp
		TBQt/TBTournamentDisplayWindow.hpp
		TBQt/TBVariantListTableModel.hpp
		TBQt/TournamentConnection.hpp
		TBQt/TournamentSession.hpp)
	qt5_wrap_ui(TBQT_COMMON_UI
		TBQt/TBSeatingChartWindow.ui
		TBQt/TBSettingsDialog.ui
		TBQt/TBTableWidget.ui
		TBQt/TBTournamentDisplayWindow.ui)
	qt5_add_resources(TBQT_COMMON_RC
		"${COPIED_QRC_FILE}")

	add_library(tbqt_common STATIC
		TBQt/SignalHandler.cpp
		TBQt/SignalHandler.hpp
		TBQt/TBActionClockWidget.cpp
		TBQt/TBActionClockWidget.hpp
		TBQt/TBActionClockWindow.cpp
		TBQt/TBActionClockWindow.hpp
		TBQt/TBBaseAuxiliaryWindow.cpp
		TBQt/TBBaseAuxiliaryWindow.hpp
		TBQt/TBBaseMainWindow.cpp
		TBQt/TBBaseMainWindow.hpp
		TBQt/TBChipDisplayDelegate.cpp
		TBQt/TBChipDisplayDelegate.hpp
		TBQt/TBCurrency.cpp
		TBQt/TBCurrency.hpp
		TBQt/TBFlowLayout.cpp
		TBQt/TBFlowLayout.hpp
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
		TBQt/TBSettingsDialog.cpp
		TBQt/TBSettingsDialog.hpp
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
		TBQt/TournamentConnection.cpp
		TBQt/TournamentConnection.hpp
		TBQt/TournamentDaemon.cpp
		TBQt/TournamentDaemon.hpp
		TBQt/TournamentService.cpp
		TBQt/TournamentService.hpp
		TBQt/TournamentSession.cpp
		TBQt/TournamentSession.hpp
		${TBQT_COMMON_MOC}
		${TBQT_COMMON_UI}
		${COPIED_QRC_FILE}
	)
	target_compile_definitions(tbqt_common PRIVATE QT_NO_KEYWORDS)
	target_include_directories(tbqt_common PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(tbqt_common td qmdnsengine Qt5::Network Qt5::Widgets Qt5::Multimedia ${CMAKE_THREAD_LIBS_INIT} ${OS_LIBRARIES})
	target_compile_features(tbqt_common PUBLIC cxx_std_11)

	# Always build resources
	add_custom_target(qrc_resources DEPENDS ${QRC_RESOURCE_FILES})
	add_dependencies(tbqt_common qrc_resources)

	qt5_wrap_cpp(TBUDDY_MOC
		TBQt/TBAnteTypeDelegate.hpp
		TBQt/TBAuthCodeDialog.hpp
		TBQt/TBBlindLevelDelegate.hpp
		TBQt/TBBuddyMainWindow.hpp
		TBQt/TBColorDisplayDelegate.hpp
		TBQt/TBDateEditDelegate.hpp
		TBQt/TBFundingDetailsDelegate.hpp
		TBQt/TBFundingTypeDelegate.hpp
		TBQt/TBManageButtonDelegate.hpp
		TBQt/TBMovementDialog.hpp
		TBQt/TBRoundsModel.hpp
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
		TBQt/TournamentDocument.hpp)
	qt5_wrap_ui(TBUDDY_UI
		TBQt/TBBuddyMainWindow.ui
		TBQt/TBMovementDialog.ui
		TBQt/TBSetupChipsWidget.ui
		TBQt/TBSetupDevicesWidget.ui
		TBQt/TBSetupDialog.ui
		TBQt/TBSetupFundingWidget.ui
		TBQt/TBSetupPayoutsWidget.ui
		TBQt/TBSetupPlayersWidget.ui
		TBQt/TBSetupRoundsWidget.ui
		TBQt/TBSetupTablesWidget.ui)
	add_executable(tbuddy
		TBQt/tbuddy.cpp
		TBQt/TBAnteTypeDelegate.cpp
		TBQt/TBAnteTypeDelegate.hpp
		TBQt/TBAuthCodeDialog.cpp
		TBQt/TBAuthCodeDialog.hpp
		TBQt/TBBlindLevelDelegate.cpp
		TBQt/TBBlindLevelDelegate.hpp
		TBQt/TBBuddyMainWindow.cpp
		TBQt/TBBuddyMainWindow.hpp
		TBQt/TBColorDisplayDelegate.cpp
		TBQt/TBColorDisplayDelegate.hpp
		TBQt/TBDateEditDelegate.cpp
		TBQt/TBDateEditDelegate.hpp
		TBQt/TBFundingDetailsDelegate.cpp
		TBQt/TBFundingDetailsDelegate.hpp
		TBQt/TBFundingTypeDelegate.cpp
		TBQt/TBFundingTypeDelegate.hpp
		TBQt/TBManageButtonDelegate.cpp
		TBQt/TBManageButtonDelegate.hpp
		TBQt/TBMovementDialog.cpp
		TBQt/TBMovementDialog.hpp
		TBQt/TBRoundsModel.cpp
		TBQt/TBRoundsModel.hpp
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
		TBQt/TournamentDocument.cpp
		TBQt/TournamentDocument.hpp
		${TBUDDY_MOC}
		${TBUDDY_UI}
		${TBQT_COMMON_RC}
	)
	target_compile_definitions(tbuddy PRIVATE QT_NO_KEYWORDS)
	target_include_directories(tbuddy PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(tbuddy tbqt_common)
	target_compile_features(tbuddy PUBLIC cxx_std_11)

	# Platform-specific target properties and icon handling
	if(APPLE)
		configure_file(TBQt/tbuddy-Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/tbuddy-Info.plist" @ONLY)

		set_target_properties(tbuddy PROPERTIES
			MACOSX_BUNDLE TRUE
			MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/tbuddy-Info.plist
			MACOSX_BUNDLE_ICON_FILE tbuddy.icns
		)

		add_custom_target(tbuddy_icns_resources DEPENDS "${CMAKE_BINARY_DIR}/Resources/tbuddy.icns" ${CMAKE_BINARY_DIR}/Resources/tbfile.icns)
		add_dependencies(tbuddy tbuddy_icns_resources)

		# Copy icon file to macOS bundle
		add_custom_command(TARGET tbuddy POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Resources/tbuddy.icns $<TARGET_BUNDLE_DIR:tbuddy>/Contents/Resources/tbuddy.icns
		)

		# Copy document type icon to macOS bundle
		add_custom_command(TARGET tbuddy POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Resources/tbfile.icns $<TARGET_BUNDLE_DIR:tbuddy>/Contents/Resources/tbfile.icns
		)
	elseif(WIN32)
		# Windows icon handling (requires .ico file and resource script)
		# set_target_properties(tbuddy PROPERTIES WIN32_EXECUTABLE TRUE)
		# TODO: Add Windows resource script (.rc) with icon reference
	else()
		configure_file(TBQt/tbuddy.desktop.in "${CMAKE_CURRENT_BINARY_DIR}/tbuddy.desktop" @ONLY)

		add_custom_target(tbuddy_png_resources DEPENDS "${CMAKE_BINARY_DIR}/Resources/tbuddy.png" "${CMAKE_BINARY_DIR}/Resources/tbfile.png")
		add_dependencies(tbuddy tbuddy_png_resources)

		# Linux: Install desktop file and icon
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tbuddy.desktop"
			DESTINATION share/applications)
		install(FILES "${CMAKE_BINARY_DIR}/Resources/tbuddy.png"
			DESTINATION share/icons/hicolor/1024x1024/apps
			RENAME "${TBUDDY_BUNDLE_EXECUTABLE}.png")
	endif()

	qt5_wrap_cpp(TBVIEWER_MOC
		TBQt/TBConnectToDialog.hpp
		TBQt/TournamentBrowser.hpp
		TBQt/TBViewerMainWindow.hpp)
	qt5_wrap_ui(TBVIEWER_UI
		TBQt/TBConnectToDialog.ui
		TBQt/TBViewerMainWindow.ui)
	add_executable(tbviewer
		TBQt/tbviewer.cpp
		TBQt/TBConnectToDialog.cpp
		TBQt/TBConnectToDialog.hpp
		TBQt/TBViewerMainWindow.cpp
		TBQt/TBViewerMainWindow.hpp
		TBQt/TournamentBrowser.cpp
		TBQt/TournamentBrowser.hpp
		${TBVIEWER_MOC}
		${TBVIEWER_UI}
		${TBQT_COMMON_RC}
	)
	target_compile_definitions(tbviewer PRIVATE QT_NO_KEYWORDS)
	target_include_directories(tbviewer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(tbviewer tbqt_common)
	target_compile_features(tbviewer PUBLIC cxx_std_11)

	# Platform-specific target properties and icon handling
	if(APPLE)
		configure_file(TBQt/tbviewer-Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/tbviewer-Info.plist" @ONLY)

		set_target_properties(tbviewer PROPERTIES
			MACOSX_BUNDLE TRUE
			MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/tbviewer-Info.plist
			MACOSX_BUNDLE_ICON_FILE tbviewer.icns
		)

		add_custom_target(tbviewer_icns_resources DEPENDS "${CMAKE_BINARY_DIR}/Resources/tbviewer.icns")
		add_dependencies(tbviewer tbviewer_icns_resources)

		# Copy icon file to macOS bundle
		add_custom_command(TARGET tbviewer POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Resources/tbviewer.icns $<TARGET_BUNDLE_DIR:tbviewer>/Contents/Resources/tbviewer.icns
		)
	elseif(WIN32)
		# Windows icon handling (requires .ico file and resource script)
		# set_target_properties(tbviewer PROPERTIES WIN32_EXECUTABLE TRUE)
		# TODO: Add Windows resource script (.rc) with icon reference
	else()
		configure_file(TBQt/tbviewer.desktop.in "${CMAKE_CURRENT_BINARY_DIR}/tbviewer.desktop" @ONLY)

		add_custom_target(tbviewer_png_resources DEPENDS "${CMAKE_BINARY_DIR}/Resources/tbviewer.png")
		add_dependencies(tbviewer tbviewer_png_resources)

		# Linux: Install desktop file and icon
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tbviewer.desktop"
			DESTINATION share/applications)
		install(FILES "${CMAKE_BINARY_DIR}/Resources/tbviewer.png"
			DESTINATION share/icons/hicolor/1024x1024/apps
			RENAME "${TBVIEWER_BUNDLE_EXECUTABLE}.png")
	endif()
endif()
