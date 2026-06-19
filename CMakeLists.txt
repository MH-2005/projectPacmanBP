cmake_minimum_required(VERSION 3.16)
project(Project_Pacman C)

set(CMAKE_C_STANDARD 23)

include(FetchContent)

function(include_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(${libName})
endfunction()

set(LIB1 raylib)
find_package(${LIB1} QUIET)
if (NOT ${LIB1}_FOUND)
    message(STATUS "Getting ${LIB1} from Github...")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    include_dependency(${LIB1} https://github.com/raysan5/raylib.git 5.5)
else()
    message(STATUS "Using local ${LIB1}")
endif()

add_executable(Project_Pacman main.c
        menu.h
        mainGame.h
        points.h
        ghosts.h
        GetNamesAndScores.h
        scores.h
        level.h
)

if (NOT ${LIB1}_FOUND)
    target_include_directories(Project_Pacman PRIVATE ${raylib_SOURCE_DIR}/src)
endif()

target_link_libraries(Project_Pacman PRIVATE ${LIB1})