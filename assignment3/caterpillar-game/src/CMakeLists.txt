set(JD_LIB_INCLUDE ${CMAKE_SOURCE_DIR}/jd-lib/include)
set(JD_LIB jd_lib)
add_library(${JD_LIB} STATIC IMPORTED)
set_property(TARGET ${JD_LIB} PROPERTY
             IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/build/jd-lib/libjd.a)
file(GLOB JD_LIB_HDRS LIST_DIRECTORIES false CMAKE_CONFIGURE_DEPENDS ${JD_LIB_INCLUDE}/*.h)

set(CONSOLE_LIB console_lib)
set(CONSOLE_LIB_HDR ${CMAKE_SOURCE_DIR}/distribute/console.h)
set(CONSOLE_LIB_SRC ${CMAKE_SOURCE_DIR}/distribute/console.c)
add_library(console_lib STATIC ${CONSOLE_LIB_HDR} ${CONSOLE_LIB_SRC})

file(GLOB HDRS LIST_DIRECTORIES false CONFIGURE_DEPENDS
     ${PROJECT_SOURCE_DIR}/include/caterpillar/*.h)
set(SRCS
     main.c
     game/game.c
     game/game_console.c
     game/constants.c
     ${CONSOLE_LIB_HDR}
     user/player.c
     user/player_state.c
     user/user_input.c
     caterpillars/caterpillar.c
     caterpillars/spawn_caterpillars.c
     bullets/bullet_collisions.c
     bullets/collision_reactions.c
     bullets/bullet.c
     bullets/bullet_list.c
     bullets/registry.c
     bullets/bullet_list.h
     upkeep.c
     ${HDRS}
     ${JD_LIB_HDRS})

set(ASS3_BIN caterpillar)
add_executable(${ASS3_BIN}
               ${SRCS})


find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

target_include_directories(${ASS3_BIN} PRIVATE ${JD_LIB_INCLUDE}
                           ${CMAKE_SOURCE_DIR}/include
                           ${CMAKE_SOURCE_DIR}/src/bullets)
target_link_libraries(${ASS3_BIN} PRIVATE
     ${JD_LIB}
     Threads::Threads
     ${CURSES_LIBRARIES}
     ${CONSOLE_LIB})
