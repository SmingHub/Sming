cmake_minimum_required(VERSION 3.8)

project(lwip C)

set (BUILD_SHARED_LIBS OFF)
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

include (ExternalProject)

set (PCAP_SRC npcap-sdk-1.03.zip)
set (PCAP_DIR src/npcap)
ExternalProject_Add(npcap
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
	URL https://nmap.org/npcap/dist/${PCAP_SRC}
	URL_HASH MD5=b9bfe28d9c9dc1110d6d635d7e7f9e23
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DLWIP_DEBUG")

set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lwip)
include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_INCLUDE_DIRS
	"${LWIP_DIR}/.."
	"${LWIP_DIR}/src/include"
	"${LWIP_CONTRIB_DIR}/ports/win32/include"
	"${PCAP_DIR}/Include"
	"${CMAKE_CURRENT_SOURCE_DIR}/"
)

include(${LWIP_DIR}/src/Filelists.cmake)

add_library(lwip
	host_lwip.c
	npcap.c
	${LWIP_CONTRIB_DIR}/ports/win32/sys_arch.c
	${LWIP_CONTRIB_DIR}/ports/win32/pcapif.c
	${LWIP_CONTRIB_DIR}/ports/win32/pcapif_helper.c
    ${LWIP_DIR}/src/api/err.c
	${lwipcore_SRCS}
	${lwipcore4_SRCS}
	${lwipcore6_SRCS}
	${lwipnetif_SRCS}
)

add_dependencies(lwip npcap)

set_target_properties(lwip
	PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${USER_LIBDIR} PREFIX "clib-" OUTPUT_NAME "${LWIP_LIBNAME}"
)

target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS} -m32 -Wno-strict-aliasing)
target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_compile_definitions(lwip PUBLIC ${CFLAGS_EXTRA})
target_include_directories(lwip PRIVATE ${LWIP_INCLUDE_DIRS} ${LWIP_MBEDTLS_INCLUDE_DIRS})
