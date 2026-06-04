cmake_minimum_required(VERSION 3.16)

project(dexhand VERSION 1.0 LANGUAGES CXX)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
    MESSAGE(STATUS "Platform: ARM architecture")
    set(IS_ARM TRUE)
else()
    MESSAGE(STATUS "Platform: x86 architecture")
    set(IS_ARM FALSE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/interfaces
        ${PROJECT_SOURCE_DIR}/include
        #${PROJECT_SOURCE_DIR}/include/lys
        ${PROJECT_SOURCE_DIR}/include/utils
        ${PROJECT_SOURCE_DIR}/include/models
        ${PROJECT_SOURCE_DIR}/include/connectivity
        ${PROJECT_SOURCE_DIR}/python
        ${PROJECT_SOURCE_DIR}/src/connectivity)

include_directories(${INCLUDE_DIRS})

if(WIN32)
    include_directories(${PROJECT_SOURCE_DIR}/include/usb2daqsa/windows
            ${PROJECT_SOURCE_DIR}/include/zlg/windows)
else()
    include_directories(${PROJECT_SOURCE_DIR}/include/usb2daqsa/linux
            ${PROJECT_SOURCE_DIR}/include/zlg/linux)
endif()

file(GLOB SRC_FILES
        ./include/connectivity/CANWrapper.h
        ./include/connectivity/Protocols.h
        ./include/utils/SafeQueue.h
        ./src/interfaces/*.cpp
        ./src/models/*.cpp
        ./src/connectivity/*.cpp
        ./src/connectivity/usbadapters/*.cpp
        ./src/utils/*.cpp
        ./python/*.cpp)

file(GLOB NIX_SRC_FILES ./src/connectivity/linux/*.cpp)

file(GLOB TEST_SRC_FILES
        ./tests/*.cpp)

ADD_LIBRARY(dexhand SHARED ${SRC_FILES}
        include/utils/logger.h)
ADD_EXECUTABLE(UnitTest ${TEST_SRC_FILES})

if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Od -D_DEBUG /MD /D _CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 /MT /D _CRT_SECURE_NO_WARNINGS")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_definitions(
            /std:c++20 /GS /W3 /Gy /Ob2 /Zc:wchar_t /Zi /Gm- /Zc:inline /Zc:__cplusplus /fp:precise /D "WIN32"
            /D "_WINDOWS" /D "_USRDLL" /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /FC /Fa /GF
            /utf-8 /D "_AMD64_" /EHsc /nologo /diagnostics:classic /D "DEXLIB_EXPORTS")
    link_libraries(ws2_32)

    set(DEPEND_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/windows)
    set(TARGET_LIB_DIR ${PROJECT_SOURCE_DIR}/bin/windows)

    target_compile_definitions(dexhand PRIVATE BUILD_DLL)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -std=c++20 -O2 -g -ggdb")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -std=c++20 -O0 -g -ggdb")
    add_compile_options(-shared -fPIC -fno-strict-aliasing -Wstringop-overflow=0 -Wpessimizing-move)

    if(IS_ARM)
        set(DEPEND_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/linux/aarch64)
        set(TARGET_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/linux/aarch64)
    else()
        set(DEPEND_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/linux/x86_64)
        set(TARGET_LIB_DIR ${PROJECT_SOURCE_DIR}/libs/linux/x86_64)
    endif()
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(DEXHAND_LIB_PROJ_NAME ${PROJECT_NAME} PARENT_SCOPE)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set(${PROJECT_NAME}_TARGET_DIR ${TARGET_LIB_DIR} PARENT_SCOPE)
    else()
        set(${PROJECT_NAME}_TARGET_DIR ${TARGET_LIB_DIR}/${CMAKE_BUILD_TYPE} PARENT_SCOPE)
    endif()
endif()

target_include_directories(dexhand PUBLIC ${INCLUDE_DIRS})
target_include_directories(UnitTest PUBLIC ${INCLUDE_DIRS})

add_dependencies(UnitTest dexhand)

if(WIN32)
    target_link_libraries(dexhand PUBLIC ${DEPEND_LIB_DIR}/zlgcan/zlgcan.lib)
    set_target_properties(dexhand PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            RUNTIME_OUTPUT_DIRECTORY ${TARGET_LIB_DIR}/${CMAKE_BUILD_TYPE}
            ARCHIVE_OUTPUT_DIRECTORY ${TARGET_LIB_DIR}/${CMAKE_BUILD_TYPE})

    target_link_libraries(UnitTest PUBLIC dexhand)
    target_link_libraries(UnitTest PUBLIC ${DEPEND_LIB_DIR}/zlgcan/zlgcan.lib)
    set_target_properties(UnitTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/windows/${CMAKE_BUILD_TYPE}")

    add_custom_target(COPY_TARGET_FILES_WIN ALL
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy ${DEPEND_LIB_DIR}/zlgcan/zlgcan.lib ${TARGET_LIB_DIR}/${CMAKE_BUILD_TYPE}/zlgcan.lib
            COMMAND ${CMAKE_COMMAND} -E copy ${DEPEND_LIB_DIR}/zlgcan/zlgcan.dll ${TARGET_LIB_DIR}/${CMAKE_BUILD_TYPE}/zlgcan.dll
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPEND_LIB_DIR}/zlgcan/kerneldlls/ ${TARGET_LIB_DIR}/${CMAKE_BUILD_TYPE}/kerneldlls/
    )
else()
    MESSAGE(STATUS DEPEND_LIB_DIR=${DEPEND_LIB_DIR})
    set_property(TARGET dexhand PROPERTY INTERFACE_LINK_LIBRARIES -Wl,--whole-archive,$<TARGET_FILE:mpfr gmp>, --no-whole-archive)
    target_link_libraries(dexhand PUBLIC ${DEPEND_LIB_DIR}/libusb-1.0.so)
    target_link_libraries(dexhand PUBLIC ${DEPEND_LIB_DIR}/libusbcanfd.so)
    target_link_libraries(dexhand PUBLIC ${DEPEND_LIB_DIR}/libserial.so)
    #target_link_libraries(dexhand PUBLIC ${DEPEND_LIB_DIR}/libControlCAN.so)
    set_target_properties(dexhand PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TARGET_LIB_DIR}")

    target_link_libraries(UnitTest PUBLIC ${DEPEND_LIB_DIR}/libusb-1.0.so)
    target_link_libraries(UnitTest PUBLIC ${DEPEND_LIB_DIR}/libusbcanfd.so)
    target_link_libraries(dexhand PUBLIC ${DEPEND_LIB_DIR}/libserial.so)
    #target_link_libraries(UnitTest PUBLIC ${DEPEND_LIB_DIR}/libControlCAN.so)
    target_link_libraries(UnitTest PUBLIC ${TARGET_LIB_DIR}/libdexhand.so)
    set_target_properties(UnitTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/linux")
endif()
