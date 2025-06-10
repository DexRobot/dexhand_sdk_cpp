cmake_minimum_required(VERSION 3.16)

project(dexhand_example VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../include)

include_directories(${INCLUDE_DIRS})

file(GLOB DX21_SOURCES ./dexhand021.cpp)
file(GLOB DX21S_SOURCES ./dexhand021s.cpp)

set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -g -ggdb -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g -ggdb -pthread")
add_compile_options(-m64 -shared -fPIC -fno-strict-aliasing -Wstringop-overflow=0 -Wpessimizing-move)

set(DEPEND_LIB_DIR ${PROJECT_SOURCE_DIR}/../lib/linux)
ADD_EXECUTABLE(dexhand021  ${DX21_SOURCES})
ADD_EXECUTABLE(dexhand021s ${DX21S_SOURCES})

target_include_directories(dexhand021  PUBLIC ${INCLUDE_DIRS})
target_include_directories(dexhand021s PUBLIC ${INCLUDE_DIRS})

target_link_libraries(dexhand021 PUBLIC ${DEPEND_LIB_DIR}/libusb-1.0.so)
target_link_libraries(dexhand021 PUBLIC ${DEPEND_LIB_DIR}/libusbcanfd.so.1.0.8)
target_link_libraries(dexhand021 PUBLIC ${DEPEND_LIB_DIR}/libControlCAN.so)
target_link_libraries(dexhand021 PUBLIC ${DEPEND_LIB_DIR}/libdexhand.so)

target_link_libraries(dexhand021s PUBLIC ${DEPEND_LIB_DIR}/libusb-1.0.so)
target_link_libraries(dexhand021s PUBLIC ${DEPEND_LIB_DIR}/libusbcanfd.so.1.0.8)
target_link_libraries(dexhand021s PUBLIC ${DEPEND_LIB_DIR}/libControlCAN.so)
target_link_libraries(dexhand021s PUBLIC ${DEPEND_LIB_DIR}/libdexhand.so)

