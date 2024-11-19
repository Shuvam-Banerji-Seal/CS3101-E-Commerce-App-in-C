cmake_minimum_required(VERSION 3.16)
project(gtk4-auth-app C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compile options
add_compile_options(-Wall -Wextra -Wpedantic)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules(LIBADWAITA REQUIRED IMPORTED_TARGET libadwaita-1)

# Configure wayland support
option(ENABLE_WAYLAND "Enable Wayland support" ON)
if(ENABLE_WAYLAND)
    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
    add_definitions(-DUSE_WAYLAND)
endif()

# Generate resource file
configure_file(
    ${CMAKE_SOURCE_DIR}/resources/style.css
    ${CMAKE_BINARY_DIR}/style.css
    COPYONLY
)

# Add resource compilation
set(GRESOURCE_XML "${CMAKE_SOURCE_DIR}/resources/resources.xml")
set(GRESOURCE_C "${CMAKE_BINARY_DIR}/resources.c")

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    COMMAND glib-compile-resources
    ARGS --target=${GRESOURCE_C} --generate-source ${GRESOURCE_XML}
    DEPENDS ${GRESOURCE_XML} ${CMAKE_SOURCE_DIR}/resources/style.css
    VERBATIM
)

# Add source files
add_executable(${PROJECT_NAME}
    src/main.c
    src/app.c
    src/database.c
    src/ui.c
    src/auth.c
    ${GRESOURCE_C}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PkgConfig::GTK4
    PkgConfig::SQLITE3
    PkgConfig::LIBADWAITA
    -lcrypt
)

if(ENABLE_WAYLAND)
    target_link_libraries(${PROJECT_NAME} PkgConfig::WAYLAND)
endif()

# Install targets
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/resources/style.css
        DESTINATION share/${PROJECT_NAME})