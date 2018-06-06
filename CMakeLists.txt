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
SET( SRC_PATH ${PROJECT_SOURCE_DIR}/src)
SET( PLATFORM_PATH_LINUX ${PROJECT_SOURCE_DIR}/platform/linux)
SET( APP_PATH ${PROJECT_SOURCE_DIR}/app)
SET( TST_PATH ${PROJECT_SOURCE_DIR}/test)

#目标文件放置到 bin 文件夹
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置库输出路径
SET( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置库文件名
set( m2m_lib "m2m_lib.${M2M_VERSION_MAJOR}.${M2M_VERSION_MINOR}.${M2M_VERSION_PATCH}")


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
FILE(GLOB SUBLIB_PLATFORM ${PLATFORM_PATH_LINUX}/*.c)
FILE(GLOB SUBLIB_CRYPT ${SRC_PATH}/crypt/*.c)
FILE(GLOB SUBLIB_NETWORK ${SRC_PATH}/network/*.c ${SRC_PATH}/network/m2m/*.c)
FILE(GLOB SUBLIB_UTIL ${SRC_PATH}/util/*.c)
FILE(GLOB SUBLIB_API ${SRC_PATH}/api/*.c)
FILE(GLOB SUBLIB_API ${SRC_PATH}/api/*.c)
# 编译器配置
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 生成共享库
ADD_LIBRARY(${m2m_lib} STATIC ${SUBLIB_UTIL} ${SUBLIB_PLATFORM} ${SUBLIB_CRYPT} ${SUBLIB_NETWORK} ${SUBLIB_API} )

# sample 应用实例
# If necessary, use the RELATIVE flag, otherwise each source file may be listed 
# with full pathname. RELATIVE may makes it easier to extract an executable name

# test appliction
FILE( GLOB TST_SOURCES  ${TST_PATH}/*.c )
foreach( testsourcefile ${TST_SOURCES} )
    # I used a simple string replace, to cut off .cpp.
    #string( REPLACE ".c" "" testname ${testsourcefile} )
    get_filename_component(TST_EXE ${testsourcefile} NAME_WE)
    add_executable( ${TST_EXE} ${testsourcefile} )
    # Make sure YourLib is linked to each app
    target_link_libraries( ${TST_EXE} ${m2m_lib} pthread )
endforeach( testsourcefile ${TST_SOURCES} )

# sample 应用实例
FILE( GLOB APP_SOURCES  ${APP_PATH}/*.c )
foreach( applicationfile ${APP_SOURCES} )
    # I used a simple string replace, to cut off .cpp.
    #string( REPLACE ".c" "" testname ${testsourcefile} )
    get_filename_component(APP_EXE ${applicationfile} NAME_WE)
    add_executable( ${APP_EXE} ${applicationfile} )
    # Make sure YourLib is linked to each app
    target_link_libraries( ${APP_EXE} ${m2m_lib} pthread )
endforeach( applicationfile ${APP_SOURCES} )
