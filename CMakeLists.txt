# CMake 最低版本要求。CLion 会使用它加载本工程并显示自定义构建目标。
cmake_minimum_required(VERSION 3.20)

# 本工程不是由 CMake 直接编译 51 单片机代码。
# STC8H 库使用 Keil C51 专用语法，真正的编译工作在 Parallels 虚拟机
# 内由 Keil UV4.exe 完成；CMake 只负责为 CLion 提供统一的构建入口。
project(51_Project LANGUAGES C)

# 让 CLion 按 C 工程处理源文件，并生成完整的代码索引信息。
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 以下变量用于记录当前工程的目标芯片和主频，后续可以传给其他脚本。
# 例如：cmake -DMCU=STC8H8K64U -DMAIN_FOSC=22118400L ...
set(MCU "STC8H8K64U" CACHE STRING "目标芯片")
set(MAIN_FOSC "22118400L" CACHE STRING "系统主频")

# 这是 CLion 专用的索引目标，不参与固件生成。
# 它的作用是把用户代码和库函数头文件注册到 CLion，使以下功能正常工作：
# - #include 文件跳转；
# - Ctrl/Cmd+B 跳转到函数、宏和类型定义；
# - GPIO、UART、Timer 等库函数的代码补全。
#
# 真正的 51 编译仍然只通过 firmware_build/generate_hex 调用 Keil。
# 递归扫描用户代码和库函数。
#
# 例如新增：
#   src/LED/LED.c
#   src/LED/LED.h
# 或：
#   src/Protocol/I2C_Device.c
#   src/Protocol/I2C_Device.h
#
# 只要文件放在 src/ 或 drivers/ 下，CMake 重新加载后就会自动发现。
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/drivers/*.c"
)

file(GLOB_RECURSE MCU_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/device/*.h"
    "${CMAKE_SOURCE_DIR}/drivers/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(ide_indexing EXCLUDE_FROM_ALL
    ${PROJECT_SOURCES}
    ${MCU_HEADERS}
)

# 默认头文件目录。
set(PROJECT_INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/cmake/ide_include"
    "${CMAKE_SOURCE_DIR}/device"
    "${CMAKE_SOURCE_DIR}/drivers"
    "${CMAKE_SOURCE_DIR}/src"
)

# 自动收集 src 下的所有子目录。
# 这样即使新建的外设目录暂时还没有头文件，也会立即成为头文件搜索路径。
file(GLOB_RECURSE SRC_SUBDIRECTORIES LIST_DIRECTORIES true
    "${CMAKE_SOURCE_DIR}/src/*"
)

foreach(src_item IN LISTS SRC_SUBDIRECTORIES)
    if(IS_DIRECTORY "${src_item}")
        list(APPEND PROJECT_INCLUDE_DIRS "${src_item}")
    endif()
endforeach()

# 同时根据头文件再次补充目录，兼容 CMake 对新建目录的识别差异。
foreach(header_file IN LISTS MCU_HEADERS)
    get_filename_component(header_dir "${header_file}" DIRECTORY)
    list(APPEND PROJECT_INCLUDE_DIRS "${header_dir}")
endforeach()
list(REMOVE_DUPLICATES PROJECT_INCLUDE_DIRS)

target_include_directories(ide_indexing PRIVATE ${PROJECT_INCLUDE_DIRS})

# 在解析 Keil 专用头文件前预加载兼容宏。
target_compile_options(ide_indexing PRIVATE
    "-include${CMAKE_SOURCE_DIR}/cmake/keil_ide_compat.h"
)

# 便于 CLion 识别当前目标芯片和主频相关宏。
target_compile_definitions(ide_indexing PRIVATE
    PROJECT_MCU_STC8H8K64U=1
)

# 通过 Parallels Guest 执行 Windows 侧的 build_keil.bat。
# 该批处理文件会调用 C:\keil\UV4\UV4.exe，并生成 Project.hex。
add_custom_target(firmware_build
    COMMAND sh "${CMAKE_SOURCE_DIR}/tools/build_keil_vm.sh"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Build firmware through Keil in Parallels VM"
)

# 与 firmware_build 使用同一个 Keil 后端。
# 单独保留这个目标，便于在 CLion 中使用“生成 HEX”的语义执行。
add_custom_target(generate_hex
    COMMAND sh "${CMAKE_SOURCE_DIR}/tools/build_keil_vm.sh"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generate Intel HEX through Keil in Parallels VM"
)

# 删除 Keil 生成的中间文件和固件输出文件。
add_custom_target(firmware_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_SOURCE_DIR}/build/output
    COMMENT "Remove firmware output files"
)
