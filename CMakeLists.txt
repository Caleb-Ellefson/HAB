cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_lora_app)

# headers
target_include_directories(app PRIVATE src/include)

# source files
target_sources(app PRIVATE
    src/main.c
    src/lora.c
)