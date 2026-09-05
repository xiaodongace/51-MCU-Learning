#!/bin/sh

# macOS 侧 Parallels Guest 编译入口。
# 通过共享目录调用 Windows 侧 Keil 构建脚本。

set -eu

PRLCTL_BIN="${PRLCTL_BIN:-/usr/local/bin/prlctl}"
PARALLELS_VM_NAME="${PARALLELS_VM_NAME:-Windows 11}"

# 默认约定：工程位于 /Users/mac/CLionProjects 下，并映射到 Windows 的
# Parallels 共享目录。使用 --resolve-paths 让 Parallels 自动把宿主机路径
# 转换为 Guest 可访问的 \\Mac\Home\... 路径，避免手工拼接 Z:\\ 路径时的转义问题。
SCRIPT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
PROJECT_NAME="$(basename -- "$PROJECT_ROOT")"
GUEST_BUILD_SCRIPT="${PROJECT_ROOT}/tools/build_keil.bat"

if [ ! -x "$PRLCTL_BIN" ]; then
    echo "ERROR: prlctl was not found at $PRLCTL_BIN" >&2
    exit 2
fi

echo "Parallels VM: $PARALLELS_VM_NAME"
echo "Project: $PROJECT_NAME"
echo "Host project path: $PROJECT_ROOT"
echo "Guest build script (resolved by Parallels): $GUEST_BUILD_SCRIPT"

exec "$PRLCTL_BIN" exec "$PARALLELS_VM_NAME" --current-user --resolve-paths cmd.exe /d /c call "$GUEST_BUILD_SCRIPT"
