set(JD_LIB_INCLUDE ${CMAKE_SOURCE_DIR}/jd-lib/include)
set(JD_LIB jd_lib)
add_library(${JD_LIB} STATIC IMPORTED)
set_property(TARGET ${JD_LIB} PROPERTY
             IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/build/jd-lib/libjd.a)

set(CMDS_LIB commands_lib)
add_subdirectory(commands)

set(SRCS prompt.c job_states.c signal_handlers.c main.c)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ASS2_BIN myshell)
add_executable(${ASS2_BIN}
               $<TARGET_OBJECTS:${CMDS_LIB}>
               ${SRCS})

target_include_directories(${ASS2_BIN} PRIVATE ${JD_LIB_INCLUDE} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${ASS2_BIN} PRIVATE ${JD_LIB})
