cmake_minimum_required(VERSION 3.18.4)

set(DROGON_GROUP_SRC
	src/Group.cpp
)

set(DROGON_GROUP_INC
	inc/drogon-group/Group.hpp
)

set(DROGON_GROUP_LIBS
	drogon-user
)

# Options

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(ENABLE_OFFLINE_CALLBACK ON)

# Dependencies

add_subdirectory(drogon-user)



# Lib

project(drogon-group
	LANGUAGES CXX
	VERSION 1.0.0
)

add_library(${PROJECT_NAME} STATIC ${DROGON_GROUP_SRC} ${DROGON_GROUP_INC})

set_target_properties(${PROJECT_NAME}
	PROPERTIES
	CXX_STANDARD 20
	CXX_EXTENSIONS OFF
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${DROGON_GROUP_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLE_GROUPS)
target_compile_definitions(drogon-user PUBLIC ENABLE_GROUPS)
