cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(DCMAKE_C_COMPILER $ENV{CC})
set(CMAKE_C_COMPILER_FORCED TRUE)

project(lwip C)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DLWIP_DEBUG")
include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${LWIP_DIR}/.."
    "${LWIP_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/"
)

include(${LWIP_DIR}/src/Filelists.cmake)

add_library(lwip
    sys.c
    ${LWIP_DIR}/src/api/err.c
    ${lwipcore_SRCS}
    ${lwipcore4_SRCS}
    ${lwipcore6_SRCS}
    ${lwipnetif_SRCS}
)

set_target_properties(lwip
	PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${USER_LIBDIR} OUTPUT_NAME "${LWIP_LIBNAME}"
)

target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_include_directories(lwip PRIVATE ${LWIP_INCLUDE_DIRS} ${LWIP_MBEDTLS_INCLUDE_DIRS})
