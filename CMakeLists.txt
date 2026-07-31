cmake_minimum_required(VERSION 3.23)

project(
	SeyfCore
	VERSION 1.0.0
	LANGUAGES CXX
)

add_library(hamed_core INTERFACE)

add_library(Seyf::Core ALIAS hamed_core)

target_include_directories(
	hamed_core
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_sources(
	hamed_core
	INTERFACE
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/hamed_core/event.hpp
			include/hamed_core/configuration.hpp
)
