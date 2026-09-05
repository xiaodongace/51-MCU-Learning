# Keil C51 虚拟机接入配置说明。
#
# macOS:  /Users/mac/CLionProjects/51_Template
# Windows: Z:\CLionProjects\51_Template
# Keil:   C:\keil
#
# 注意：macOS 不能直接执行 Windows 的 UV4.exe。实际的编译命令将在
# Windows 侧由 tools/build_keil.bat 执行；CLion 侧后续只负责触发和读取结果。

set(KEIL_VM_PROJECT_WINDOWS "Z:/CLionProjects/51_Template")
set(KEIL_ROOT_WINDOWS "C:/keil")
