##add_definitions(-DJRTPLIB_COMPILING)

set (HEADERS
	../common/Task.h
	MergeTask.h
	../common/utils2.h
	../common/utils.h
	../common/logger.h
	../common/HttpParser.h
	)

set(SOURCES
	../common/Task.cpp
	../common/TaskQueue.cpp
	../common/utils.c
	../common/unicode.c
	mergefile.c
	MergeTask.cpp
	mergepipe.cpp
	MergeDelayTask.cpp
	../common/utils2.cpp
	checkMergeTask.cpp
	../common/logger.c
	)


if (MSVC)
	set(CMAKE_DEBUG_POSTFIX _d)
endif ()


#aux_source_directory(. DIR_LIB_SRCS)
add_library(merge ${SOURCES})





