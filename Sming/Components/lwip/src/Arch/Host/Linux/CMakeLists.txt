cmake_minimum_required(VERSION 3.8)

project(lwip C)

set (BUILD_SHARED_LIBS OFF)
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DLWIP_DEBUG")
include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/.."
    "${LWIP_DIR}/src/include"
    "${LWIP_CONTRIB_DIR}/ports/unix/port/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/"
)

include(${LWIP_CONTRIB_DIR}/ports/unix/Filelists.cmake)
include(${LWIP_DIR}/src/Filelists.cmake)

add_library(lwip
    ${LWIP_DIR}/src/api/err.c
    ${lwipcore_SRCS}
    ${lwipcore4_SRCS}
    ${lwipcore6_SRCS}
    ${lwipnetif_SRCS}
    ${lwipcontribportunix_SRCS}
    ${lwipcontribportunixnetifs_SRCS}
)

set_target_properties(lwip
	PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${USER_LIBDIR} OUTPUT_NAME "${LWIP_LIBNAME}"
)

target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS} ${BUILD_MODE})
target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_include_directories(lwip PRIVATE ${LWIP_INCLUDE_DIRS} ${LWIP_MBEDTLS_INCLUDE_DIRS})
