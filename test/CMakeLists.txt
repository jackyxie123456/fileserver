##add_definitions(-DJRTPLIB_COMPILING)

set (HEADERS
	../common/crc32.h
	)

set(SOURCES
	../common/crc32.c
	main.cpp
	)


if (MSVC)
	set(CMAKE_DEBUG_POSTFIX _d)
endif ()


#aux_source_directory(. DIR_LIB_SRCS)
##add_library(fileclient ${SOURCES})

add_executable(test ${SOURCES})   #





