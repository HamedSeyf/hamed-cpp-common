cmake_minimum_required(VERSION 3.23)

project(
	hamed_common
	VERSION 1.0.0
	LANGUAGES CXX
)

add_library(hamed_common INTERFACE)

target_compile_features(
	hamed_common
	INTERFACE
		cxx_std_20
)

target_include_directories(
	hamed_common
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_sources(
	hamed_common
	INTERFACE
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/hamed_common/generic_notification_manager.h
			include/hamed_common/generic_singleton.h
			include/hamed_common/sorts.h
)
