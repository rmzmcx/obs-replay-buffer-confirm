cmake_minimum_required(VERSION 3.28)
project(obs-replay-buffer-confirm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Local OBS 32.2.2 SDK snapshot: headers exported from the obs-studio tag 32.2.2,
# import libraries generated from the binaries shipped with the OBS install.
set(OBS_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.deps" CACHE PATH "Local OBS SDK snapshot")

add_library(${PROJECT_NAME} SHARED
  src/plugin-main.cpp
  src/guard/replay_buffer_guard.cpp
  src/guard/replay_buffer_guard.hpp
  src/guard/settings.cpp
  src/guard/settings.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  "${OBS_DEPS_DIR}/obs-sdk/libobs"
  "${OBS_DEPS_DIR}/obs-sdk/frontend/api"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  "${OBS_DEPS_DIR}/lib/obs.lib"
  "${OBS_DEPS_DIR}/lib/obs-frontend-api.lib"
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /W3)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${PROJECT_NAME}/bin/64bit")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/" DESTINATION "${PROJECT_NAME}/data")