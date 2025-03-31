cmake_minimum_required(VERSION 3.30.0)
project(opencv_camera VERSION 0.1.0)
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.3/msvc2022_64" ${CMAKE_PREFIX_PATH})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Find required packages
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Enable automoc, autouic, and autorcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add executable
add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui # Añadimos el archivo de interfaz de usuario
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    ${OpenCV_LIBS} # Variable que contiene las bibliotecas de OpenCV encontradas por find_package
)