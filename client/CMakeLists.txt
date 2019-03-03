##add_definitions(-DJRTPLIB_COMPILING)

set (HEADERS
	DbHelper.h
	DbTask.h
	../common/Task.h
	../common/utils.h
	watch.h
	../common/logger.h
	../common/HttpParser.h
	)

set(SOURCES
	DbHelper.cpp
	DbTask.cpp
	multiUpload.cpp
	UploadTask.cpp
	../common/Task.cpp
	../common/TaskQueue.cpp
	watch.cpp
	../common/utils.c
	../common/unicode.c
	../common/logger.c
	../common/HttpParser.cpp
	../common/crc32.c
	../common/utils2.cpp
	)

set(CLISOURCES
	client.cpp
	)

if (MSVC)
	set(CMAKE_DEBUG_POSTFIX _d)
endif ()


#aux_source_directory(. DIR_LIB_SRCS)
add_library(fileclient ${SOURCES})





