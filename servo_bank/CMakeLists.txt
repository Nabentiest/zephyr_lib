add_library(servo_bank STATIC src/servo.c)

target_include_directories(servo_bank PUBLIC include)
target_link_libraries(servo_bank PUBLIC zephyr_interface)
