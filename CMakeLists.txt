cmake_minimum_required(VERSION 3.16)
project(MandelbrotFractal)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(main src/main.cpp)

target_link_libraries(main
    SFML::Graphics
    SFML::Window
    SFML::System
)

foreach(target_name main)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_SOURCE_DIR}/assets/mandelbrot.frag
                $<TARGET_FILE_DIR:${target_name}>/mandelbrot.frag
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_SOURCE_DIR}/assets/font.ttf
                $<TARGET_FILE_DIR:${target_name}>/font.ttf
    )
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/assets/mandelbrot.frag ${CMAKE_BINARY_DIR}/mandelbrot.frag COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/assets/font.ttf ${CMAKE_BINARY_DIR}/font.ttf COPYONLY)
