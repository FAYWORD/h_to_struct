cmake_minimum_required(VERSION 2.8)

if("${CMAKE_VERSION}" VERSION_LESS "3.0")
    project(h_to_struct C)
else()
    project(h_to_struct LANGUAGES C)
endif()

set(SOURCES
    parse_hfile.c
    main.c
    options.c
    write.c
    func.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

