set(STRING_LIB string_lib)
add_subdirectory(string)

set(VECTOR_LIB vector_lib)
add_subdirectory(vector)

set(ERROR_LIB error_lib)
add_subdirectory(error)

set(LIST_LIB list_lib)
add_subdirectory(list)

set(QUEUE_LIB queue_lib)
add_subdirectory(queue)

set(THREADPOOL_LIB threadpool_lib)
add_subdirectory(threadpool)

set(TASK_LIB task_lib)
add_subdirectory(task)

set(JD_LIB jd)
add_library(${JD_LIB}
            STATIC $<TARGET_OBJECTS:${STRING_LIB}>
                   $<TARGET_OBJECTS:${VECTOR_LIB}>
                   $<TARGET_OBJECTS:${ERROR_LIB}>
                   $<TARGET_OBJECTS:${LIST_LIB}>
                   $<TARGET_OBJECTS:${QUEUE_LIB}>
                   $<TARGET_OBJECTS:${THREADPOOL_LIB}>
                   $<TARGET_OBJECTS:${TASK_LIB}>)
