file(GLOB PRIVATE_HDRS LIST_DIRECTORIES false CONFIGURE_DEPENDS *.h)
file(GLOB PUBLIC_HDRS LIST_DIRECTORIES false CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/include/myshell/commands/*.h
        ${PROJECT_SOURCE_DIR}/include/myshell/commands/internal/*.h)
set(SRCS commands.c internal/cd.c internal/exit.c internal/fg.c internal/bg.c internal/argument_validity.c)

add_library(${CMDS_LIB} OBJECT ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})
target_include_directories(${CMDS_LIB}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src ${JD_LIB_INCLUDE})
