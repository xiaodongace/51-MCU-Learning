#!/usr/bin/env bash

# 51_Template 外设/通信模块生成器
#
# 用法：
#   tools/new_bsp.sh LED
#
# 生成：
#   src/LED/LED.c
#   src/LED/LED.h

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$PROJECT_ROOT/src"

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 ModuleName"
    echo "Example: $0 LED"
    exit 1
fi

NAME="$1"

# 模块名同时用于目录名、文件名、函数前缀和头文件保护宏，必须是合法 C 标识符。
if [[ ! "$NAME" =~ ^[A-Za-z_][A-Za-z0-9_]*$ ]]; then
    echo "Error: module name must be a valid C identifier: $NAME" >&2
    exit 1
fi

MODULE_DIR="$SRC_DIR/$NAME"
HEADER_FILE="$MODULE_DIR/$NAME.h"
SOURCE_FILE="$MODULE_DIR/$NAME.c"
GUARD="__$(printf '%s' "$NAME" | tr '[:lower:]' '[:upper:]')_H"

if [[ -e "$MODULE_DIR" ]]; then
    echo "Error: module '$NAME' already exists: $MODULE_DIR" >&2
    exit 1
fi

mkdir -p "$MODULE_DIR"

cat > "$HEADER_FILE" <<EOF
#ifndef ${GUARD}
#define ${GUARD}

#include "config.h"

void ${NAME}_Init(void);

#endif /* ${GUARD} */
EOF

cat > "$SOURCE_FILE" <<EOF
#include "${NAME}.h"

void ${NAME}_Init(void)
{
    /* 在这里完成 ${NAME} 模块的初始化。 */
}
EOF

echo "Module '$NAME' created successfully."
echo "  $HEADER_FILE"
echo "  $SOURCE_FILE"
echo "Run 'Reload CMake Project' in CLion if the new files are not indexed immediately."
