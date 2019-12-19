cmake_minimum_required(VERSION 3.2)
project(voip_patrol VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

message("CMAKE_SYSTEM_PROCESSOR:${CMAKE_SYSTEM_PROCESSOR} CMAKE_SYSTEM:${CMAKE_SYSTEM}  CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE} OV:${OV}")

set(ROOT_DIR ".")
set(SRC_DIR "${ROOT_DIR}/src")
set(CURL_SRC_DIR "${SRC_DIR}/curl")
set(EZXML_SRC_DIR "${SRC_DIR}/ezxml")
set(VOIP_PATROL_SRC_DIR "${SRC_DIR}/voip_patrol")

set(EZXML_SRCS
	${EZXML_SRC_DIR}/ezxml.c
)
include_directories("${SRC_DIR}")

set(VOIP_PATROL_SRCS_CPP
	${VOIP_PATROL_SRC_DIR}/mod_voip_patrol.cc
	${VOIP_PATROL_SRC_DIR}/voip_patrol.cc
	${VOIP_PATROL_SRC_DIR}/action.cc
	${VOIP_PATROL_SRC_DIR}/check.cc
)

set(VOIP_PATROL_SRCS_C

)

include_directories(${ROOT_DIR}/pjproject/pjmedia/include)
include_directories(${ROOT_DIR}/pjproject/pjsip/include)
include_directories(${ROOT_DIR}/pjproject/pjlib/include)
include_directories(${ROOT_DIR}/pjproject/pjnath/include)
include_directories(${ROOT_DIR}/pjproject/pjlib-util/include)

include_directories(${ROOT_DIR}/include)

set(SOURCE_FILES ${VOIP_PATROL_SRCS_CPP} ${VOIP_PATROL_SRCS_C} ${CURL_SRCS} ${EZXML_SRCS})

add_executable(voip_patrol ${SOURCE_FILES})

set(CMAKE_LIBRARY_PATH
	"${ROOT_DIR}/pjproject/pjsip/lib"
	"${ROOT_DIR}/pjproject/pjnath/lib"
	"${ROOT_DIR}/pjproject/pjlib/lib"
	"${ROOT_DIR}/pjproject/media/lib"
	"${ROOT_DIR}/pjproject/pjlib-util/lib"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

execute_process(COMMAND "./pjproject/config.guess" OUTPUT_VARIABLE AC_SYSTEM)
string(STRIP ${AC_SYSTEM} AC_SYSTEM)
target_link_libraries(voip_patrol
	pjsua2-${AC_SYSTEM}
	stdc++
	pjsua-${AC_SYSTEM}
	pjsip-ua-${AC_SYSTEM}
	pjsip-simple-${AC_SYSTEM}
	pjsip-${AC_SYSTEM}
	pjmedia-codec-${AC_SYSTEM}
	pjmedia-${AC_SYSTEM}
	pjmedia-videodev-${AC_SYSTEM}
	pjmedia-audiodev-${AC_SYSTEM}
	pjmedia-${AC_SYSTEM}
	pjnath-${AC_SYSTEM}
	pjlib-util-${AC_SYSTEM}
	srtp-${AC_SYSTEM}
	resample-${AC_SYSTEM}
	gsmcodec-${AC_SYSTEM}
	speex-${AC_SYSTEM}
	ilbccodec-${AC_SYSTEM}
	g7221codec-${AC_SYSTEM}
	pj-${AC_SYSTEM}
	pthread
	curl
	m
	asound
	ssl
	-std=c++11
)

find_package(PkgConfig REQUIRED)
pkg_search_module(OPENSSL openssl)
if( OPENSSL_FOUND )
	message(">> openssl found")
	target_link_libraries(voip_patrol ssl crypto)
else()
	message(">> openssl not found")
endif()

pkg_search_module(OPUS opus)
if( OPUS_FOUND )
	message(">> opus found")
	target_link_libraries(voip_patrol opus)
else()
	message(">> opus not found")
endif()

pkg_search_module(UUID uuid)
if( UUID_FOUND )
	 message(">> uuid found")
	target_link_libraries(voip_patrol uuid)
else()
	message(">> uuid not found")
endif()
