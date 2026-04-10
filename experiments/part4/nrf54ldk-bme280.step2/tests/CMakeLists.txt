cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(weatherstation_tests)

target_sources(app PRIVATE 
    test_weather_shared_mem.c
    test_weather_sensor.c
    test_weather_monitor.c
    test_console_interface.c
    ../src/Weather_SharedMem.c
    ../src/console_interface.c
)
