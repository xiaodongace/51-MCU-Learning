#!/usr/bin/env bash

# 用法：
#   tools/sync_src_to_keil.sh ModuleName
#   tools/sync_src_to_keil.sh ModuleName --dry-run
#   tools/sync_src_to_keil.sh --all [--dry-run]

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
SRC_ROOT="$PROJECT_ROOT/src"
LABS_ROOT="$SRC_ROOT/Labs"

usage() {
    cat <<'EOF'
Usage:
  tools/sync_src_to_keil.sh ModuleName [--dry-run]
  tools/sync_src_to_keil.sh --all [--dry-run]

Examples:
  tools/sync_src_to_keil.sh Relay
  tools/sync_src_to_keil.sh Light
  tools/sync_src_to_keil.sh Car
  tools/sync_src_to_keil.sh Sensor/SHT30 --dry-run
  tools/sync_src_to_keil.sh --all

Note:
  可以直接输入目录名，脚本会在 src/ 下自动查找对应目录。
  也可以输入相对于 src/ 的目录路径，或者 src/ 下的绝对路径。
  如果找到多个同名目录，脚本会列出候选路径并要求使用相对路径。
  src/Labs is handled by tools/sync_labs_to_keil.sh.
EOF
}

die() {
    echo "Error: $*" >&2
    exit 1
}

DRY_RUN=0
SYNC_ALL=0
MODULE_PATH=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dry-run)
            DRY_RUN=1
            ;;
        --all)
            SYNC_ALL=1
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            [[ -z "$MODULE_PATH" ]] || die "only one ModuleName may be specified"
            if [[ "$1" != /* ]]; then
                [[ "$1" =~ ^[A-Za-z_][A-Za-z0-9_/-]*$ ]] || \
                    die "ModuleName may contain only letters, numbers, '_', '-' and '/'"
            fi
            [[ "$1" != *".."* ]] || die "ModuleName must not contain '..'"
            MODULE_PATH="$1"
            ;;
    esac
    shift
done

[[ "$SYNC_ALL" -eq 0 || -z "$MODULE_PATH" ]] || \
    die "use either ModuleName or --all, not both"
[[ "$SYNC_ALL" -eq 1 || -n "$MODULE_PATH" ]] || \
    die "specify ModuleName, or use --all to synchronize all non-Labs modules"

[[ -d "$SRC_ROOT" ]] || die "src directory was not found: $SRC_ROOT"

PROJECT_FILE=""
PROJECT_COUNT=0
while IFS= read -r file; do
    PROJECT_FILE="$file"
    PROJECT_COUNT=$((PROJECT_COUNT + 1))
done < <(find "$PROJECT_ROOT/keil" -maxdepth 1 -type f -name '*.uvproj' -print)

[[ "$PROJECT_COUNT" -eq 1 ]] || \
    die "expected exactly one .uvproj file under $PROJECT_ROOT/keil"

SCAN_ROOT="$SRC_ROOT"
if [[ "$SYNC_ALL" -eq 0 ]]; then
    [[ "$MODULE_PATH" != "Labs" && "$MODULE_PATH" != Labs/* ]] || \
        die "src/Labs must use tools/sync_labs_to_keil.sh"

    # 允许输入src目录下的绝对路径，但禁止同步工程目录之外的内容。
    if [[ "$MODULE_PATH" == /* ]]; then
        [[ "$MODULE_PATH" == "$SRC_ROOT"/* ]] || \
            die "absolute ModuleName must be inside $SRC_ROOT"
        SCAN_ROOT="$MODULE_PATH"
    elif [[ "$MODULE_PATH" == */* ]]; then
        # 包含斜杠时，按照相对于src的路径处理，例如 projects/Car/Light。
        SCAN_ROOT="$SRC_ROOT/$MODULE_PATH"
    elif [[ -d "$SRC_ROOT/$MODULE_PATH" ]]; then
        # 优先兼容原来的顶层目录用法，例如 LED、Motor、UART。
        SCAN_ROOT="$SRC_ROOT/$MODULE_PATH"
    else
        # 顶层目录不存在时，在src下递归查找同名目录。
        # 这样可以直接输入 Light 或 Car，不需要输入完整路径。
        MATCHED_DIRS=()
        while IFS= read -r matched_dir; do
            MATCHED_DIRS+=("$matched_dir")
        done < <(find "$SRC_ROOT" -path "$LABS_ROOT" -prune -o -type d -name "$MODULE_PATH" -print | LC_ALL=C sort)

        if [[ "${#MATCHED_DIRS[@]}" -eq 0 ]]; then
            die "module directory was not found under $SRC_ROOT: $MODULE_PATH"
        fi

        if [[ "${#MATCHED_DIRS[@]}" -gt 1 ]]; then
            echo "multiple directories named '$MODULE_PATH' were found:" >&2
            for matched_dir in "${MATCHED_DIRS[@]}"; do
                echo "  ${matched_dir#$PROJECT_ROOT/}" >&2
            done
            die "please use a relative path such as projects/Car/$MODULE_PATH"
        fi

        SCAN_ROOT="${MATCHED_DIRS[0]}"
    fi

    [[ -d "$SCAN_ROOT" ]] || die "module directory was not found: $SCAN_ROOT"
fi

add_unique() {
    local candidate="$1"
    local existing

    # 兼容 macOS 自带 Bash 3 在 set -u 下对空数组的处理。
    for existing in "${INCLUDE_PATHS[@]:-}"; do
        [[ "$existing" == "$candidate" ]] && return
    done
    INCLUDE_PATHS+=("$candidate")
}

to_keil_path() {
    local absolute_path="$1"
    local relative_path

    relative_path="${absolute_path#$PROJECT_ROOT/}"
    relative_path="${relative_path//\//\\}"
    printf '..\\%s' "$relative_path"
}

INCLUDE_PATHS=()
add_unique "..\\src"

if [[ "$SYNC_ALL" -eq 1 ]]; then
    while IFS= read -r directory; do
        add_unique "$(to_keil_path "$directory")"
    done < <(find "$SCAN_ROOT" -path "$LABS_ROOT" -prune -o -type d -print | LC_ALL=C sort)
else
    while IFS= read -r directory; do
        add_unique "$(to_keil_path "$directory")"
    done < <(find "$SCAN_ROOT" -type d -print | LC_ALL=C sort)
fi

SOURCE_FILES=()
if [[ "$SYNC_ALL" -eq 1 ]]; then
    while IFS= read -r source_file; do
        SOURCE_FILES+=("$source_file")
    done < <(find "$SCAN_ROOT" -path "$LABS_ROOT" -prune -o -type f -name '*.c' -print | LC_ALL=C sort)
else
    while IFS= read -r source_file; do
        SOURCE_FILES+=("$source_file")
    done < <(find "$SCAN_ROOT" -type f -name '*.c' -print | LC_ALL=C sort)
fi

[[ "${#SOURCE_FILES[@]}" -gt 0 ]] || \
    die "no .c files were found under $SCAN_ROOT"

NEW_SOURCE_ENTRIES=""
NEW_SOURCE_PATHS=()

for source_file in "${SOURCE_FILES[@]}"; do
    source_name="$(basename "$source_file")"
    source_path="$(to_keil_path "$source_file")"

    # FilePath 是工程中最稳定的唯一标识；FileName 可以在不同目录中重复。
    if grep -Fq "<FilePath>$source_path</FilePath>" "$PROJECT_FILE"; then
        continue
    fi

    NEW_SOURCE_ENTRIES+=$'            <File>\n'
    NEW_SOURCE_ENTRIES+="              <FileName>$source_name</FileName>"$'\n'
    NEW_SOURCE_ENTRIES+=$'              <FileType>1</FileType>\n'
    NEW_SOURCE_ENTRIES+="              <FilePath>$source_path</FilePath>"$'\n'
    NEW_SOURCE_ENTRIES+=$'            </File>\n'
    NEW_SOURCE_PATHS+=("$source_path")
done

TEMP_PROJECT="$(mktemp "$PROJECT_ROOT/keil/.src-sync.XXXXXX")"
TEMP_UPDATED="${TEMP_PROJECT}.updated"
trap 'rm -f "$TEMP_PROJECT" "$TEMP_UPDATED"' EXIT
cp -p "$PROJECT_FILE" "$TEMP_PROJECT"

# 合并而不是覆盖 Include Path，保留用户原本在 Keil 中配置的路径。
SRC_INCLUDE_PATHS="$(IFS=';'; printf '%s' "${INCLUDE_PATHS[*]}")" \
perl -0777 -e '
    my $project = do { local $/; <> };
    my @needed = grep { length } split /;/, $ENV{SRC_INCLUDE_PATHS};
    my $updated = 0;

    $updated = $project =~ s{<IncludePath>(.*?)</IncludePath>}{
        my @paths = grep { length } split /;/, $1;
        my %seen = map { lc($_) => 1 } @paths;
        for my $path (@needed) {
            next if $seen{lc($path)}++;
            push @paths, $path;
        }
        "<IncludePath>" . join(";", @paths) . "</IncludePath>";
    }gse;

    die "No IncludePath node found in Keil project\n" unless $updated;
    print $project;
' "$TEMP_PROJECT" > "$TEMP_UPDATED"
mv "$TEMP_UPDATED" "$TEMP_PROJECT"

if [[ -n "$NEW_SOURCE_ENTRIES" ]]; then
    SRC_SOURCE_ENTRIES="$NEW_SOURCE_ENTRIES" \
    perl -0777 -e '
        my $project = do { local $/; <> };
        my $entries = $ENV{SRC_SOURCE_ENTRIES};
        my $updated = $project =~ s{
            (<Group>\s*<GroupName>BSP</GroupName>\s*<Files>)
            (.*?)
            (</Files>\s*</Group>)
        }{$1 . $2 . $entries . $3}sex;

        die "BSP group was not found in Keil project\n" unless $updated;
        print $project;
    ' "$TEMP_PROJECT" > "$TEMP_UPDATED"
    mv "$TEMP_UPDATED" "$TEMP_PROJECT"
fi

if [[ -n "$NEW_SOURCE_ENTRIES" ]]; then
    for source_path in "${NEW_SOURCE_PATHS[@]}"; do
        grep -Fq "<FilePath>$source_path</FilePath>" "$TEMP_PROJECT" || \
            die "failed to add source entry: $source_path"
    done
fi

if cmp -s "$PROJECT_FILE" "$TEMP_PROJECT"; then
    echo "Keil source synchronization is already up to date."
    exit 0
fi

if [[ "$DRY_RUN" -ne 0 ]]; then
    echo "Dry run: the Keil project would be updated."
    echo "Include directories to ensure:"
    for include_path in "${INCLUDE_PATHS[@]}"; do
        echo "  $include_path"
    done

    if [[ -z "$NEW_SOURCE_ENTRIES" ]]; then
        echo "No new .c source entries are needed."
    else
        echo "New .c source entries for BSP:"
        for source_path in "${NEW_SOURCE_PATHS[@]}"; do
            echo "  $source_path"
        done
    fi
    exit 0
fi

BACKUP_FILE="$PROJECT_FILE.before-src-sync.bak"
cp -p "$PROJECT_FILE" "$BACKUP_FILE"
mv "$TEMP_PROJECT" "$PROJECT_FILE"
trap - EXIT

echo "Keil source synchronization completed."
echo "Project: $PROJECT_FILE"
echo "Backup:  $BACKUP_FILE"
echo "Include paths were merged without removing existing entries."

if [[ -z "$NEW_SOURCE_ENTRIES" ]]; then
    echo "No new .c source entries were needed."
else
    echo "Added .c files to BSP:"
    for source_path in "${NEW_SOURCE_PATHS[@]}"; do
        echo "  $source_path"
    done
fi
