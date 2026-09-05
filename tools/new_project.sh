#!/usr/bin/env bash

# 51_Template 工程初始化/改名脚本
#
# 使用场景：先复制整个模板目录并改名，再在新工程目录中执行：
#   ./tools/new_project.sh 8051_Clion
#
# 脚本会：
#   1. 清理复制来的 build/；
#   2. 更新 CMake 工程名；
#   3. 重命名 keil/ 下的 .uvproj；
#   4. 更新 README 标题和工程名；
#   5. 保留 .idea/encodings.xml 的 GBK 配置。

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 ProjectName"
    echo "Example: $0 8051_Clion"
    exit 1
fi

PROJECT_NAME="$1"

# 工程名会写入 CMake 和文档，限制为安全的工程标识符。
if [[ ! "$PROJECT_NAME" =~ ^[A-Za-z0-9_][A-Za-z0-9_-]*$ ]]; then
    echo "Error: project name may contain only letters, numbers, '_' and '-'." >&2
    echo "       It must not start with '-' or contain spaces." >&2
    exit 1
fi

if [[ ! -f "$PROJECT_ROOT/CMakeLists.txt" || ! -d "$PROJECT_ROOT/keil" ]]; then
    echo "Error: this script must be run inside a 51_Template project." >&2
    exit 1
fi

PROJECT_FILE=""
PROJECT_FILE_COUNT=0
while IFS= read -r file; do
    PROJECT_FILE="$file"
    PROJECT_FILE_COUNT=$((PROJECT_FILE_COUNT + 1))
done < <(find "$PROJECT_ROOT/keil" -maxdepth 1 -type f -name '*.uvproj' -print)

if [[ "$PROJECT_FILE_COUNT" -gt 1 ]]; then
    echo "Error: more than one .uvproj file exists in $PROJECT_ROOT/keil" >&2
    exit 1
fi

if [[ "$PROJECT_FILE_COUNT" -eq 1 ]]; then
    TARGET_PROJECT_FILE="$PROJECT_ROOT/keil/${PROJECT_NAME}.uvproj"
    if [[ "$PROJECT_FILE" != "$TARGET_PROJECT_FILE" ]]; then
        if [[ -e "$TARGET_PROJECT_FILE" ]]; then
            echo "Error: target Keil project already exists: $TARGET_PROJECT_FILE" >&2
            exit 1
        fi
        mv "$PROJECT_FILE" "$TARGET_PROJECT_FILE"
    fi
fi

# 删除复制来的 CMake 缓存、Keil 中间文件和旧 HEX。
rm -rf "$PROJECT_ROOT/build"

# 更新 CMake 工程名。CMakeLists.txt 使用 UTF-8 编码。
PROJECT_NAME="$PROJECT_NAME" perl -0pi -e \
    's/project\(51_Template LANGUAGES C\)/project($ENV{PROJECT_NAME} LANGUAGES C)/' \
    "$PROJECT_ROOT/CMakeLists.txt"

# README 由模板生成，统一替换其中的旧工程名。
if [[ -f "$PROJECT_ROOT/README.md" ]]; then
    PROJECT_NAME="$PROJECT_NAME" perl -0pi -e \
        's/51_Template/$ENV{PROJECT_NAME}/g' "$PROJECT_ROOT/README.md"
fi

echo "Project initialized successfully: $PROJECT_NAME"
echo "Project root: $PROJECT_ROOT"
echo "Build directory was cleaned."
if [[ -f "$PROJECT_ROOT/keil/${PROJECT_NAME}.uvproj" ]]; then
    echo "Keil project: keil/${PROJECT_NAME}.uvproj"
fi
echo "Next step: Reload CMake Project in CLion, then run firmware_build."
