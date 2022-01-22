file(GLOB PRIVATE_HDRS LIST_DIRECTORIES false CONFIGURE_DEPENDS *.h)
set(${PUBLIC_HDRS} ${PROJECT_SOURCE_DIR}/include/jd/task.h)
file(GLOB SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS *.c)

find_package(Threads REQUIRED)

add_library(${TASK_LIB} OBJECT ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})
target_link_libraries(${TASK_LIB} PUBLIC Threads::Threads)
target_include_directories(${TASK_LIB}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src)
