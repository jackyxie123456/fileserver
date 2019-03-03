# CMakeList.txt: 
# 
#
cmake_minimum_required (VERSION 2.8)


project(httpserver)


SET(CMAKE_BUILD_TYPE  DEBUG)   #
															 #debug 
															 #release
							
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")															 
															 
SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")  				
	
add_definitions(-std=c++11)
							 
aux_source_directory(. DIR_SRCS)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)  ##

# 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# Add header file include directories
include_directories(${PROJECT_SOURCE_DIR}/./ 
									  ${PROJECT_SOURCE_DIR}/./include   
										${PROJECT_SOURCE_DIR}/./common
									  )#????????



add_subdirectory(${PROJECT_SOURCE_DIR}/httpd  httpd)                  # 
add_subdirectory(${PROJECT_SOURCE_DIR}/client  client)   
add_subdirectory(${PROJECT_SOURCE_DIR}/mergedaemon  mergedaemon)  
add_subdirectory(${PROJECT_SOURCE_DIR}/test  test)
#add_subdirectory(${PROJECT_SOURCE_DIR}/sdlPlayer/rtsp2rtmp  rtsp2rtmp)                  #


#add_library(librtmp SHARED IMPORTED)
#set_target_properties(librtmp PROPERTIES IMPORTED_LOCATION
#${PROJECT_NAME}/../rtmpdump/lib/librtmp.a)

#add_executable(sipclient)   #
#add_executable(rtsp2rtmp) 

link_directories(
			    ${PROJECT_SOURCE_DIR}/httpd
				${PROJECT_NAME}/lib)
										
#link_libraries("../rtmpdump/lib/librtmp.a")
#link_libraries("../jrtplib-3.11.1/lib/jrtp.a")


set (HEADERS
	
	)

set(SOURCES
	${PROJECT_SOURCE_DIR}/httpd/main.c
	)

set(CLISOURCES
	${PROJECT_SOURCE_DIR}/client/client.cpp
	)


set(DAEMONSOURCES
	${PROJECT_SOURCE_DIR}/mergedaemon/mergedaemon.cpp
	)

if (MSVC)
	set(CMAKE_DEBUG_POSTFIX _d)
endif ()

add_executable(httpserver ${SOURCES})   #上述方法有问题，这种方法才有效

target_link_libraries(httpserver  
						${CMAKE_SOURCE_DIR}/lib/libhttp.a 
						rt z ssl crypto   pthread) 

add_executable(httpclient ${CLISOURCES})   #

target_link_libraries(httpclient  
						${CMAKE_SOURCE_DIR}/lib/libfileclient.a
						libgumbo.a
						curl sqlite3 rt z ssl crypto   pthread) 

add_executable(daemon1 ${DAEMONSOURCES})   #

target_link_libraries(daemon1  
						${CMAKE_SOURCE_DIR}/lib/libmerge.a
					  pthread) 