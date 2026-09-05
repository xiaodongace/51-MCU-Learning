#!/usr/bin/env bash

# 将 src/Labs/ 下的实验目录同步到 Keil 工程。
#
# 用法：
#   tools/sync_labs_to_keil.sh
#       同步 Labs 下全部实验目录。
#
#   tools/sync_labs_to_keil.sh TimerTickLab
#       只同步 src/Labs/TimerTickLab/。
#
#   tools/sync_labs_to_keil.sh --dry-run
#       预览会发生的修改，不写入 Keil 工程。
#
# 脚本只做“添加缺失项”：
#   1. 把 Labs 目录及其子目录加入 Keil Include Path；
#   2. 把 Labs 中缺失的 .c 文件加入 Keil 的 Labs 分组；
# 不会删除现有条目，也不会重复添加同一个文件。

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
LABS_ROOT="$PROJECT_ROOT/src/Labs"

usage() {
    cat <<'EOF'
Usage:
  tools/sync_labs_to_keil.sh [LabName] [--dry-run]

Examples:
  tools/sync_labs_to_keil.sh --dry-run
  tools/sync_labs_to_keil.sh
  tools/sync_labs_to_keil.sh LedBasicLab
EOF
}

die() {
    echo "Error: $*" >&2
    exit 1
}

DRY_RUN=0
LAB_NAME=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dry-run)
            DRY_RUN=1
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            [[ -z "$LAB_NAME" ]] || die "only one LabName may be specified"
            [[ "$1" =~ ^[A-Za-z_][A-Za-z0-9_]*$ ]] || \
                die "LabName must contain only letters, numbers and '_'"
            LAB_NAME="$1"
            ;;
    esac
    shift
done

[[ -d "$LABS_ROOT" ]] || die "Labs directory was not found: $LABS_ROOT"

PROJECT_FILE=""
PROJECT_COUNT=0
while IFS= read -r file; do
    PROJECT_FILE="$file"
    PROJECT_COUNT=$((PROJECT_COUNT + 1))
done < <(find "$PROJECT_ROOT/keil" -maxdepth 1 -type f -name '*.uvproj' -print)

[[ "$PROJECT_COUNT" -eq 1 ]] || \
    die "expected exactly one .uvproj file under $PROJECT_ROOT/keil"

SCAN_ROOT="$LABS_ROOT"
if [[ -n "$LAB_NAME" ]]; then
    SCAN_ROOT="$LABS_ROOT/$LAB_NAME"
    [[ -d "$SCAN_ROOT" ]] || die "Lab directory was not found: $SCAN_ROOT"
fi

add_unique() {
    local candidate="$1"
    local existing

    # macOS 自带的 Bash 3 在 set -u 下展开空数组会报“未绑定变量”，
    # 因此这里使用 :- 兼容第一次向数组添加路径的场景。
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
add_unique "..\\src\\Labs"

while IFS= read -r directory; do
    add_unique "$(to_keil_path "$directory")"
done < <(find "$SCAN_ROOT" -type d -print | LC_ALL=C sort)

SOURCE_FILES=()
while IFS= read -r source_file; do
    SOURCE_FILES+=("$source_file")
done < <(find "$SCAN_ROOT" -type f -name '*.c' -print | LC_ALL=C sort)

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

TEMP_PROJECT="$(mktemp "$PROJECT_ROOT/keil/.labs-sync.XXXXXX")"
TEMP_UPDATED="${TEMP_PROJECT}.updated"
trap 'rm -f "$TEMP_PROJECT" "$TEMP_UPDATED"' EXIT
cp -p "$PROJECT_FILE" "$TEMP_PROJECT"

# 合并而不是覆盖 Include Path，保留用户原本在 Keil 中配置的路径。
# 使用 -0777 明确按整个文件处理。不能写成 -0pi：Perl 会把 p 当作
# -0 的参数，导致文件按错误的记录分隔符拆分。
LAB_INCLUDE_PATHS="$(IFS=';'; printf '%s' "${INCLUDE_PATHS[*]}")" \
perl -0777 -e '
    my $project = do { local $/; <> };
    my @needed = grep { length } split /;/, $ENV{LAB_INCLUDE_PATHS};
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
    LAB_SOURCE_ENTRIES="$NEW_SOURCE_ENTRIES" \
    perl -0777 -e '
        my $project = do { local $/; <> };
        my $entries = $ENV{LAB_SOURCE_ENTRIES};
        my $updated = $project =~ s{
            (<Group>\s*<GroupName>Labs</GroupName>\s*<Files>)
            (.*?)
            (</Files>\s*</Group>)
        }{$1 . $2 . $entries . $3}sex;

        die "Labs group was not found in Keil project\n" unless $updated;
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
    echo "Keil Labs synchronization is already up to date."
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
        echo "New .c source entries:"
        for source_path in "${NEW_SOURCE_PATHS[@]}"; do
            echo "  $source_path"
        done
    fi
    exit 0
fi

BACKUP_FILE="$PROJECT_FILE.before-labs-sync.bak"
cp -p "$PROJECT_FILE" "$BACKUP_FILE"
mv "$TEMP_PROJECT" "$PROJECT_FILE"
trap - EXIT

echo "Keil Labs synchronization completed."
echo "Project: $PROJECT_FILE"
echo "Backup:  $BACKUP_FILE"
echo "Include paths were merged without removing existing entries."

if [[ -z "$NEW_SOURCE_ENTRIES" ]]; then
    echo "No new .c source entries were needed."
else
    echo "Added .c files to the Labs group:"
    for source_path in "${NEW_SOURCE_PATHS[@]}"; do
        echo "  $source_path"
    done
fi
