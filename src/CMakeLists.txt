

include_directories(".")
add_executable(kurve main.cpp GamepadController.cpp)
target_link_libraries(kurve ${CONAN_LIBS})