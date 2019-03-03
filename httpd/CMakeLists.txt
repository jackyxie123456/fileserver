##add_definitions(-DJRTPLIB_COMPILING)

set (HEADERS
	event.h
	http.h
	httpd.h
	../common/logger.h
	network.h
	tree.h
	types.h
	../common/utils.h
	dump.h
	)

set(SOURCES
	network.c
	../common/logger.c
	event.c
	http.c
	../common/utils.c
	unicode.c
	dump.c
	mergepipe.c
	../common/crc32.c
	)


if (MSVC)
	set(CMAKE_DEBUG_POSTFIX _d)
endif ()


#aux_source_directory(. DIR_LIB_SRCS)
add_library(http ${SOURCES})


