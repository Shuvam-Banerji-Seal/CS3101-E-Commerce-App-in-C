cmake_minimum_required(VERSION 3.15)

# Set the project name
project(LibrarySystem)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Enable Qt's Meta-Object Compiler (moc)
set(CMAKE_AUTOMOC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Set the source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
)

# Set the header files
set(HEADERS
    include/MainWindow.h
)

# Add the executable
add_executable(cstest ${SOURCES} ${HEADERS})

# Link Qt6 libraries
target_link_libraries(cstest Qt6::Widgets)

# Include directories
target_include_directories(cstest PUBLIC include)

