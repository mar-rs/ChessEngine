cmake_minimum_required(VERSION 3.12)
project(guhess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h" "src/*.hpp")

add_executable(guhess ${SOURCES} ${HEADERS})

file(GLOB_RECURSE ALL_FILES CONFIGURE_DEPENDS "src/*")
foreach(FILE ${ALL_FILES})
    if(FILE MATCHES "\\.(h|hpp)$")
        get_filename_component(DIR ${FILE} DIRECTORY)
        target_include_directories(guhess PRIVATE ${DIR})
    endif()
endforeach()

target_include_directories(guhess PRIVATE src)
