cmake_minimum_required(VERSION 3.10)

project(lab-4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

set_target_properties(
  ${PROJECT_NAME} 
  PROPERTIES CXX_STANDARD 17
)
    
add_subdirectory(libs)

target_link_libraries(
  ${PROJECT_NAME}
  sfml-graphics    
  sfml-window
  CLI11::CLI11
  jsoncpp_static
  config_loader
)

target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_JSONCPP)

