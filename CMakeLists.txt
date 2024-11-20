cmake_minimum_required(VERSION 3.10)
project(ChatApplication C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(LIBADWAITA REQUIRED IMPORTED_TARGET libadwaita-1)
find_package(SQLite3 REQUIRED)


message(STATUS "SQLite3_INCLUDE_DIRS: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "SQLite3_LIBRARIES: ${SQLite3_LIBRARIES}")


add_executable(masterchatbot
    chatbot/chatbot-gtk-final.c
)

target_link_libraries(masterchatbot
    PRIVATE
    PkgConfig::GTK4
    PkgConfig::LIBADWAITA
    m
)


add_executable(main
    main.c
    menu2.c
    ascii_art.c
    database.c
    auth.c
    db_init.c
    cart.c

)

target_include_directories(main
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(main
    PRIVATE
    ${SQLite3_LIBRARIES}
    m
)
