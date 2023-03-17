cmake_minimum_required(VERSION 3.24)

set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)

project(rusokucan
        DESCRIPTION "Qt can bus plugin for Rusoku TouCAN adapters"
        HOMEPAGE_URL "https://www.rusoku.com"
        LANGUAGES CXX C
        )

set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)
#set(CMAKE_MAKE_PROGRAM "ninja")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_OSX_ARCHITECTURES x86_64)

set(CMAKE_PREFIX_PATH "~/Qt/5.15.2/clang_64"
                        "C:/Qt/5.15.2/msvc2019_64")

find_package(Qt5 COMPONENTS
        Core
        SerialBus
        REQUIRED)

set(SOURCE_FILES
        main.cpp
        rusokucanbackend.cpp
        rusokucanbackend_p.cpp)

set(HEADERS_FILES
        rusokucanbackend.h
        rusokucanbackend_p.h
        rusoku_symbols_p.h)

set(OTHER_FILES
        plugin.json)

add_library(rusokucan SHARED ${SOURCE_FILES} ${HEADERS_FILES} ${OTHER_FILES})

target_link_libraries(rusokucan
        Qt5::Core
        Qt5::SerialBus)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(rusokucan QT_DEBUG)
else()
    add_compile_definitions(rusokucan QT_NO_DEBUG)
    add_compile_definitions(rusokucan QT_NO_DEBUG_OUTPUT)
endif()

set_property(TARGET rusokucan PROPERTY MACHO_CURRENT_VERSION 1.0.0)
#set_property(TARGET rusokucan PROPERTY MACHO_COMPATABILITY_VERSION 1.0.0)
set_property(TARGET rusokucan PROPERTY VERSION 1.0.0)
