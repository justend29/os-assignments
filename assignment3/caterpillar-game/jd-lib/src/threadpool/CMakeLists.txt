file(GLOB_RECURSE PRIVATE_HDRS LIST_DIRECTORIES false CONFIGURE_DEPENDS *.h)
set(${PUBLIC_HDRS} ${PROJECT_SOURCE_DIR}/include/jd/threadpool.h)
file(GLOB_RECURSE SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS *.c)

find_package(Threads REQUIRED)

add_library(${THREADPOOL_LIB} OBJECT ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})
target_link_libraries(${THREADPOOL_LIB} PUBLIC Threads::Threads)
target_include_directories(${THREADPOOL_LIB}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src)
