cmake_minimum_required(VERSION 3.15)
project(CanInfotainment)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Fix for MSVC warnings
if(MSVC)
    add_compile_options(/W3 /wd4068 /wd4244 /wd4267)
endif()

include(FetchContent)

# Fetch Asio (required by Crow)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG        asio-1-30-2
)
FetchContent_Populate(asio)
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE PATH "" FORCE)

# Fetch Crow which handles websockets
set(CROW_FEATURES "websockets" CACHE STRING "" FORCE)
FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG        v1.2.0
)
FetchContent_MakeAvailable(Crow)

add_executable(CanServer src/main.cpp src/CanSimulator.cpp)
target_include_directories(CanServer PRIVATE src)
target_link_libraries(CanServer PRIVATE Crow::Crow)

if(WIN32)
    target_link_libraries(CanServer PRIVATE wsock32 ws2_32)
endif()
