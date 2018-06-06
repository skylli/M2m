# 1. creat m2m library.
# cmake version request
cmake_minimum_required(VERSION 2.6)
# define project name
project (M2m)
# The version number.
set(M2M_VERSION_MAJOR 1)
set(M2M_VERSION_MINOR 0)
set(M2M_VERSION_PATCH 1) 

# 路劲设置
SET(SRC_PATH ${PROJECT_SOURCE_DIR}/src)
SET(PLATFORM_PATH ${PROJECT_SOURCE_DIR}/platform/linux)
SET(APP_PATH ${PROJECT_SOURCE_DIR}/app)
# 设置库文件名
set(m2m_lib "m2m_lib.${M2M_VERSION_MAJOR}.${M2M_VERSION_MINOR}.${M2M_VERSION_PATCH}")


# 搜索当前目录下所有的头文件
file(GLOB_RECURSE Foo_HEADERS "*.h")
set (Foo_INCLUDE_DIRS "")
foreach (_headerFile ${Foo_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND Foo_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES Foo_INCLUDE_DIRS)

# 包含工程所有的头文件
include_directories(${Foo_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include")

# 子模块生成子库
FILE(GLOB SUBLIB_PLATFORM ${PLATFORM_PATH}/api/*.c)
FILE(GLOB SUBLIB_CRYPT ${SRC_PATH}/crypt/*.c)
FILE(GLOB SUBLIB_NETWORK ${SRC_PATH}/network/*.c)
FILE(GLOB SUBLIB_UTIL ${SRC_PATH}/util/*.c)
FILE(GLOB SUBLIB_API ${SRC_PATH}/api/*.c)

# 编译器配置
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 生成共享库
ADD_LIBRARY(${m2m_lib} SHARED ${SUBLIB_CRYPT} ${SUBLIB_NETWORK} ${SUBLIB_UTIL} ${SUBLIB_API} )

# 指定生成目标 
add_executable(client ${APP_PATH}/app_client.c )
target_link_libraries(client ${m2m_lib} pthread)

add_executable(server ${APP_PATH}/app_server.c )
target_link_libraries(server ${m2m_lib} pthread)
