#!/usr/bin/env bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0
#
# Batch-verify .abc samples by directly invoking static_abc_verifier binary.
# Intended for offline/intranet environments where Maven/JAR build is unavailable.
#
# Default expectation: every sample should be blocked (verifier exits non-zero).
#
# Blocked samples: parses verifier output lines "  [En] ..." (ErrorCode in verifier.h)
# and prints how often each rule fired (occurrences) and in how many distinct files.
#
# Usage:
#   ./batch_verify_samples.sh <abc_dir> [--recursive] [--verbose]
#                            [--verifier /path/to/static_abc_verifier]
#                            [--allow-pass]
#                            [--tsv-report <path>]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERIFIER_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# One error code per line (same code may repeat if the verifier reported it multiple times).
extract_verifier_error_codes() (
  set +o pipefail
  echo "$1" | grep -oE '\[E[0-9]+\]' | sed 's/\[E//;s/\]//'
)

# ErrorCode enum names must match verifier.h (numeric values only).
error_code_name() {
  case "$1" in
    1) echo "FILE_OPEN_FAILED" ;;
    2) echo "INVALID_MAGIC" ;;
    3) echo "INVALID_VERSION" ;;
    4) echo "CHECKSUM_MISMATCH" ;;
    5) echo "FILE_SIZE_MISMATCH" ;;
    6) echo "INVALID_HEADER" ;;
    7) echo "INVALID_CLASS_INDEX" ;;
    8) echo "INVALID_LITERAL_ARRAY_INDEX" ;;
    9) echo "INVALID_REGION_HEADER" ;;
    10) echo "INVALID_FOREIGN_SECTION" ;;
    11) echo "INVALID_SOURCE_LANG" ;;
    12) echo "INVALID_CLASS_DATA" ;;
    13) echo "INVALID_METHOD_DATA" ;;
    14) echo "INVALID_CODE_DATA" ;;
    15) echo "INVALID_LITERAL_ARRAY_DATA" ;;
    16) echo "INVALID_BYTECODE" ;;
    17) echo "INVALID_REGISTER_INDEX" ;;
    18) echo "INVALID_JUMP_TARGET" ;;
    19) echo "INVALID_TRY_CATCH" ;;
    *) echo "UNKNOWN_$1" ;;
  esac
}

usage() {
  cat <<EOF
Usage: $(basename "$0") <abc_dir> [options]

Options:
  -r, --recursive          Scan subdirectories
  -v, --verbose            Print each file result
      --verifier <path>    Path to static_abc_verifier binary
      --allow-pass         Return 0 even if some files are not blocked
      --tsv-report <path>  Write per-file tab-separated report (path, exit, rule codes)
  -h, --help               Show this help

Env:
  STATIC_ABC_VERIFIER_BIN  Alternative way to provide verifier binary path

Default verifier lookup order:
  1) --verifier
  2) STATIC_ABC_VERIFIER_BIN
  3) $VERIFIER_ROOT/build/\$(uname -m)/static_abc_verifier
  4) static_abc_verifier in PATH

Exit code:
  0 -> all samples blocked (or --allow-pass)
  2 -> some samples were not blocked
  1 -> usage/runtime error
EOF
}

ROOT_DIR=""
RECURSIVE=0
VERBOSE=0
ALLOW_PASS=0
TSV_REPORT=""
VERIFIER_BIN="${STATIC_ABC_VERIFIER_BIN:-}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    -r|--recursive)
      RECURSIVE=1
      shift
      ;;
    -v|--verbose)
      VERBOSE=1
      shift
      ;;
    --allow-pass)
      ALLOW_PASS=1
      shift
      ;;
    --tsv-report)
      if [[ $# -lt 2 ]]; then
        echo "Missing value for --tsv-report" >&2
        usage
        exit 1
      fi
      TSV_REPORT="$2"
      shift 2
      ;;
    --verifier)
      if [[ $# -lt 2 ]]; then
        echo "Missing value for --verifier" >&2
        usage
        exit 1
      fi
      VERIFIER_BIN="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage
      exit 1
      ;;
    *)
      if [[ -z "$ROOT_DIR" ]]; then
        ROOT_DIR="$1"
      else
        echo "Unexpected extra argument: $1" >&2
        usage
        exit 1
      fi
      shift
      ;;
  esac
done

if [[ -z "$ROOT_DIR" ]]; then
  usage
  exit 1
fi

ROOT_DIR="$(realpath "$ROOT_DIR")"
if [[ ! -d "$ROOT_DIR" ]]; then
  echo "Not a directory: $ROOT_DIR" >&2
  exit 1
fi

if [[ -z "$VERIFIER_BIN" ]]; then
  ARCH="$(uname -m)"
  CANDIDATE="$VERIFIER_ROOT/build/$ARCH/static_abc_verifier"
  if [[ -x "$CANDIDATE" ]]; then
    VERIFIER_BIN="$CANDIDATE"
  elif command -v static_abc_verifier >/dev/null 2>&1; then
    VERIFIER_BIN="$(command -v static_abc_verifier)"
  fi
fi

if [[ -z "$VERIFIER_BIN" ]] || [[ ! -x "$VERIFIER_BIN" ]]; then
  echo "Cannot find executable verifier binary." >&2
  echo "Set --verifier or STATIC_ABC_VERIFIER_BIN, or build via ./build.sh native." >&2
  exit 1
fi

FILES=()
if [[ "$RECURSIVE" == "1" ]]; then
  while IFS= read -r -d '' f; do
    FILES+=("$f")
  done < <(find "$ROOT_DIR" -type f -name '*.abc' -print0)
else
  while IFS= read -r -d '' f; do
    FILES+=("$f")
  done < <(find "$ROOT_DIR" -maxdepth 1 -type f -name '*.abc' -print0)
fi

if [[ "${#FILES[@]}" == "0" ]]; then
  echo "No .abc files found under: $ROOT_DIR"
  exit 0
fi

IFS=$'\n' FILES=($(printf '%s\n' "${FILES[@]}" | sort))
unset IFS

blocked=0
not_blocked=0
tool_failures=0
blocked_without_parsed_rules=0
declare -A ERR_OCCURRENCES
declare -A ERR_FILE_HITS
total="${#FILES[@]}"

if [[ -n "$TSV_REPORT" ]]; then
  printf '%s\t%s\t%s\n' "path" "exit" "error_codes" >"$TSV_REPORT"
fi

echo "Verifier:   $VERIFIER_BIN"
echo "Directory:  $ROOT_DIR"
echo "Recursive:  $RECURSIVE"
echo ".abc files: $total"

for i in "${!FILES[@]}"; do
  f="${FILES[$i]}"
  if ((( (i + 1) % 100 == 0 ) || ( i + 1 == total ) )); then
    echo "Progress: $((i + 1))/$total" >&2
  fi

  set +e
  output="$("$VERIFIER_BIN" "$f" 2>&1)"
  code=$?
  set -e

  if [[ "$code" -eq 0 ]]; then
    ((not_blocked+=1))
    if [[ -n "$TSV_REPORT" ]]; then
      printf '%s\t%d\t%s\n' "$f" "$code" "" >>"$TSV_REPORT"
    fi
    if [[ "$VERBOSE" == "1" ]]; then
      echo -e "NOT_BLOCKED\t$f"
      echo "$output"
    fi
  elif [[ "$code" -eq 1 ]]; then
    ((blocked+=1))
    mapfile -t _codes < <(extract_verifier_error_codes "$output")
    if [[ "${#_codes[@]}" -eq 0 ]]; then
      ((blocked_without_parsed_rules+=1))
    fi
    declare -A _seen_this_file=()
    for c in "${_codes[@]}"; do
      [[ -z "$c" ]] && continue
      ERR_OCCURRENCES[$c]=$((${ERR_OCCURRENCES[$c]:-0} + 1))
      if [[ -z "${_seen_this_file[$c]:-}" ]]; then
        _seen_this_file[$c]=1
        ERR_FILE_HITS[$c]=$((${ERR_FILE_HITS[$c]:-0} + 1))
      fi
    done
    if [[ -n "$TSV_REPORT" ]]; then
      _joined="$(printf '%s\n' "${_codes[@]}" | sort -u | paste -sd, -)"
      printf '%s\t%d\t%s\n' "$f" "$code" "${_joined}" >>"$TSV_REPORT"
    fi
    if [[ "$VERBOSE" == "1" ]]; then
      echo -e "BLOCKED\t$f"
      echo "$output"
      _vjoined="$(printf '%s\n' "${_codes[@]}" | sort -u | paste -sd, -)"
      echo "  (rules: ${_vjoined:-<none parsed>})"
    fi
    unset _seen_this_file
  else
    ((tool_failures+=1))
    echo -e "TOOL_FAILURE\t$f\texit=$code" >&2
    if [[ -n "$TSV_REPORT" ]]; then
      _joined="$(extract_verifier_error_codes "$output" | sort -u | paste -sd, -)"
      printf '%s\t%d\t%s\n' "$f" "$code" "${_joined}" >>"$TSV_REPORT"
    fi
    if [[ "$VERBOSE" == "1" ]]; then
      echo "$output" >&2
    fi
  fi
done

echo "---"
echo "Blocked:      $blocked"
echo "Not blocked:  $not_blocked"
echo "Tool failures:$tool_failures"

if [[ "$blocked" -gt 0 ]]; then
  echo "---"
  echo "Verification rule statistics (blocked samples; codes match verifier.h ErrorCode)"
  if [[ "${#ERR_OCCURRENCES[@]}" -eq 0 ]]; then
    echo "  (no [En] lines parsed; check verifier output format)" >&2
  else
    printf '%s\t%s\t%s\t%s\n' "occurrences" "files" "code" "ErrorCode_name"
    for c in "${!ERR_OCCURRENCES[@]}"; do
      fh="${ERR_FILE_HITS[$c]:-0}"
      printf '%d\t%d\t%s\t%s\n' "${ERR_OCCURRENCES[$c]}" "$fh" "$c" "$(error_code_name "$c")"
    done | sort -t $'\t' -k1nr -k3n
  fi
  if [[ "$blocked_without_parsed_rules" -gt 0 ]]; then
    echo "Note: $blocked_without_parsed_rules blocked file(s) had exit 1 but no parseable [En] lines." >&2
  fi
fi

if [[ "$ALLOW_PASS" == "1" ]]; then
  exit 0
fi

if [[ "$not_blocked" -eq 0 ]] && [[ "$tool_failures" -eq 0 ]]; then
  exit 0
fi
exit 2
