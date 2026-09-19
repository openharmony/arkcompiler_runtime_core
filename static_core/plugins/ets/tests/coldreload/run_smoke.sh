#!/usr/bin/env bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Coldreload end-to-end smoke test.
#
#   Usage: run_smoke.sh [BUILD_DIR] [OUT_DIR]
#     BUILD_DIR  panda build directory (default: <repo>/build) providing bin/ark
#                and plugins/ets/etsstdlib.abc
#     OUT_DIR    scratch directory for the compiled abc files (default: BUILD_DIR/coldreload-smoke)
#
# Exit code 0 means every case passed.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PANDA_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"
BUILD_DIR="${1:-${PANDA_ROOT}/build}"
OUT_DIR="${2:-${BUILD_DIR}/coldreload-smoke}"

mkdir -p "${OUT_DIR}"
BUILD_DIR="$(cd "${BUILD_DIR}" && pwd)"
OUT_DIR="$(cd "${OUT_DIR}" && pwd)"

# Native driver that binds `coldReloadNative` in the test module and drives the production
# transaction (tests/ani/coldreload_app).
COLDRELOAD_APP="${COLDRELOAD_APP:-${BUILD_DIR}/bin/ani_coldreload_app}"
# Any single case must finish well within this; exceeding it means a hang.
MT_TIMEOUT_SECONDS=120
ETSSTDLIB="${ETSSTDLIB:-${BUILD_DIR}/plugins/ets/etsstdlib.abc}"
# A Debug build does not ship es2panda; override with ES2PANDA=<path> if needed.
ES2PANDA="${ES2PANDA:-${BUILD_DIR}/bin/es2panda}"
if [[ ! -f "${ES2PANDA}" ]]; then
    ES2PANDA="${PANDA_ROOT}/build/bin/es2panda"
fi

ARK_AOT="${ARK_AOT:-${BUILD_DIR}/bin/ark_aot}"

for tool in "${ES2PANDA}" "${ARK_AOT}" "${COLDRELOAD_APP}" "${ETSSTDLIB}"; do
    if [[ ! -f "${tool}" ]]; then
        echo "FATAL: not found: ${tool}" >&2
        exit 2
    fi
done

# `coldreload::Error` ordinals, keep in sync with plugins/ets/runtime/coldreload/coldreload.h
readonly ERR_NONE=0
readonly ERR_INVALID_INPUT_ARG=1
readonly ERR_PATCH_OPEN_FAILED=2
readonly ERR_AOT_LOADED=3
readonly ERR_CLASSES_ALREADY_LOADED=4

# The unpatched report line, shared by every case that must observe the original file.
readonly WANT_V1_RUN="run tag=V1 sub=V1 n=2 calc=1 base=100"

FAILURES=0

function compile_case() {
    # compile_case <variant> -> ${OUT_DIR}/<variant>/t.abc
    local variant="$1"
    mkdir -p "${OUT_DIR}/${variant}"
    if ! "${ES2PANDA}" --extension=ets \
            --output="${OUT_DIR}/${variant}/t.abc" \
            "${SCRIPT_DIR}/${variant}/t.ets" >"${OUT_DIR}/${variant}/es2panda.log" 2>&1; then
        echo "FATAL: es2panda failed for ${variant}, see ${OUT_DIR}/${variant}/es2panda.log" >&2
        exit 2
    fi
}

function run_case() {
    # run_case <name> <patch> <patch2> <pre> <want_rc> <want_rc2|-> <want_run> <want_lazy> [want_pre]
    #
    #   <patch>    '' for the INVALID_INPUT_ARG case; a nonexistent path for PATCH_OPEN_FAILED
    #   <patch2>   second patch ('' -> no second coldReload call, no rc2 line)
    #   <pre>      'pre' -> Driver runs once BEFORE coldReload (the startup-contract case)
    #   <want_rc2> '-' when no second call is expected
    local name="$1" patch="$2" patch2="$3" pre="$4"
    local want_rc="$5" want_rc2="$6" want_run="$7" want_lazy="$8"
    local want_pre="${9-}"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${COLDRELOAD_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/base/t.abc" "${patch}" "${patch2}" "${pre}" >"${log}" 2>&1
    local exitcode=$?

    local rc rc2 run lazy pre
    rc="$(sed -n 's/^rc=\([0-9-]*\)$/\1/p' "${log}" | head -1)"
    rc2="$(sed -n 's/^rc2=\([0-9-]*\)$/\1/p' "${log}" | head -1)"
    run="$(grep -m1 '^run ' "${log}")"
    lazy="$(sed -n 's/^lazy=\(.*\)$/\1/p' "${log}" | head -1)"
    pre="$(grep -m1 '^pre ' "${log}")"

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    [[ "${rc}" == "${want_rc}" ]] || { echo "  rc: got '${rc}' want '${want_rc}'"; ok=0; }
    if [[ "${want_rc2}" != "-" ]]; then
        [[ "${rc2}" == "${want_rc2}" ]] || { echo "  rc2: got '${rc2}' want '${want_rc2}'"; ok=0; }
    else
        [[ -z "${rc2}" ]] || { echo "  unexpected rc2='${rc2}'"; ok=0; }
    fi
    [[ "${run}" == "${want_run}" ]] || { echo "  run: got '${run}' want '${want_run}'"; ok=0; }
    [[ "${lazy}" == "${want_lazy}" ]] || { echo "  lazy: got '${lazy}' want '${want_lazy}'"; ok=0; }
    if [[ -n "${want_pre}" ]]; then
        [[ "${pre}" == "${want_pre}" ]] || { echo "  pre: got '${pre}' want '${want_pre}'"; ok=0; }
    else
        [[ -z "${pre}" ]] || { echo "  unexpected pre='${pre}'"; ok=0; }
    fi
    # A successful reload must announce the prepend; refusals carry their own diagnostics,
    # asserted per case below.
    if [[ "${want_rc}" == "${ERR_NONE}" ]]; then
        grep -q "prepended to abcFiles" "${log}" || { echo "  no 'prepended to abcFiles' line"; ok=0; }
    fi

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

echo "== compiling =="
compile_case base
compile_case full_v2
compile_case partial
compile_case addfield
compile_case addmethod
compile_case rebase
compile_case twice_a
compile_case twice_b

# Build the hqf-shaped patch packages: plain zips whose `ets/modules_static.abc` entry
# holds the abc, entries STORED like real packages because the runtime mmaps them.
function package() {
    # package <abc-path> <package-path>
    local abc="$1" pkg="$2"
    local stage="${OUT_DIR}/stage"
    rm -rf "${stage}" "${pkg}"
    mkdir -p "${stage}/ets"
    cp "${abc}" "${stage}/ets/modules_static.abc"
    (cd "${stage}" && zip -0 -X -q -r "${pkg}" ets)
    rm -rf "${stage}"
}
mkdir -p "${OUT_DIR}/pkg"
package "${OUT_DIR}/full_v2/t.abc" "${OUT_DIR}/pkg/patch.hqf"
: > "${OUT_DIR}/pkg/does-not-exist.hqf"
# A native-only patch package: no abc entry, only the .so a cpp change produces.
native_stage="${OUT_DIR}/stage_native"
rm -rf "${native_stage}" "${OUT_DIR}/pkg/native-only.hqf"
mkdir -p "${native_stage}/libs/arm64-v8a"
: > "${native_stage}/libs/arm64-v8a/libdummy.so"
(cd "${native_stage}" && zip -0 -X -q -r "${OUT_DIR}/pkg/native-only.hqf" libs)
rm -rf "${native_stage}"

# A `.hap` patch package: same integrity-checked entry branch as `.hqf`, pins the suffix routing.
package "${OUT_DIR}/full_v2/t.abc" "${OUT_DIR}/pkg/patch.hap"

# A `.hsp` patch package. The hsp entry lookup works on the DERIVED path of the package
# (`GetRelativePathForHsp` returns a non-absolute derived path unchanged), so the case passes a
# patch path relative to the driver's working directory and the entry is named after that
# derived path.
hsp_rel="coldreload-smoke/pkg/patch-hsp.hsp"
rm -rf "${OUT_DIR}/hsp_stage" "${OUT_DIR}/pkg/patch-hsp.hsp"
mkdir -p "${OUT_DIR}/hsp_stage/coldreload-smoke/pkg/patch-hsp/ets"
cp "${OUT_DIR}/full_v2/t.abc" "${OUT_DIR}/hsp_stage/coldreload-smoke/pkg/patch-hsp/ets/modules_static.abc"
(cd "${OUT_DIR}/hsp_stage" && zip -0 -X -q -r "${OUT_DIR}/pkg/patch-hsp.hsp" coldreload-smoke)
rm -rf "${OUT_DIR}/hsp_stage"

echo "== running =="
# Every case runs the SAME program (base/t.abc); only the patch differs.
run_case full "${OUT_DIR}/full_v2/t.abc" '' '' \
    "${ERR_NONE}" - "run tag=V2 sub=V2 n=2 calc=42 base=100" "L2"

# The device shape: the framework hands over a `.hqf` patch package whose
# `ets/modules_static.abc` entry holds the abc. Same expectations as `full`.
run_case hqf "${OUT_DIR}/pkg/patch.hqf" '' '' \
    "${ERR_NONE}" - "run tag=V2 sub=V2 n=2 calc=42 base=100" "L2"
# A package without a readable entry is refused, not opened another way.
run_case hap_patch "${OUT_DIR}/pkg/patch.hap" '' '' \
    "${ERR_NONE}" - "run tag=V2 sub=V2 n=2 calc=42 base=100" "L2"
(cd "${BUILD_DIR}" && run_case hsp_patch "${hsp_rel}" '' '' \
    "${ERR_NONE}" - "run tag=V2 sub=V2 n=2 calc=42 base=100" "L2")
run_case neg_badpkg "${OUT_DIR}/pkg/does-not-exist.hqf" '' '' \
    "${ERR_PATCH_OPEN_FAILED}" - "${WANT_V1_RUN}" "L1"

run_case partial "${OUT_DIR}/partial/t.abc" '' '' \
    "${ERR_NONE}" - "run tag=P2 sub=P2 n=2 calc=1 base=100" "L1"
run_case addfield "${OUT_DIR}/addfield/t.abc" '' '' \
    "${ERR_NONE}" - "run tag=F2-7 sub=F2-7 n=2 calc=1 base=100" "L1"
run_case addmethod "${OUT_DIR}/addmethod/t.abc" '' '' \
    "${ERR_NONE}" - "run tag=V1 sub=M2 n=2 calc=1 base=100" "L1"
run_case rebase "${OUT_DIR}/rebase/t.abc" '' '' \
    "${ERR_NONE}" - "run tag=B2 sub=B2 n=2 calc=1 base=100" "L1"
# The second prepend lands in FRONT of the first, so twice_b must win the Greeter lookup.
run_case twice "${OUT_DIR}/twice_a/t.abc" "${OUT_DIR}/twice_b/t.abc" '' \
    "${ERR_NONE}" "${ERR_NONE}" "run tag=PB sub=PB n=2 calc=1 base=100" "L1"
run_case emptypath '' '' '' \
    "${ERR_INVALID_INPUT_ARG}" - "${WANT_V1_RUN}" "L1"
run_case badpath "${OUT_DIR}/nonexistent.abc" '' '' \
    "${ERR_PATCH_OPEN_FAILED}" - "${WANT_V1_RUN}" "L1"

# A native-only patch (no abc entry in the package) is a no-op success: cold reload only
# reorders class lookup, the .so files are the framework's business.
function run_native_only_case() {
    local name="native_only"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${COLDRELOAD_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/base/t.abc" "${OUT_DIR}/pkg/native-only.hqf" '' '' >"${log}" 2>&1
    local exitcode=$?

    local rc run
    rc="$(sed -n 's/^rc=\([0-9-]*\)$/\1/p' "${log}" | head -1)"
    run="$(grep -m1 '^run ' "${log}")"

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    [[ "${rc}" == "${ERR_NONE}" ]] ||
        { echo "  rc: got '${rc}' want '${ERR_NONE}' (native-only patch must be a no-op success)"; ok=0; }
    [[ "${run}" == "${WANT_V1_RUN}" ]] ||
        { echo "  run: got '${run}' want '${WANT_V1_RUN}' (a no-op must change nothing)"; ok=0; }
    grep -q "nothing to prepend" "${log}" ||
        { echo "  missing the native-only diagnostic"; ok=0; }
    ! grep -q "prepended to abcFiles" "${log}" ||
        { echo "  a package without an abc entry was prepended"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_native_only_case
# Startup contract, enforced: once the module has loaded classes the call is refused,
# and nothing already loaded changed.
run_case preloaded "${OUT_DIR}/full_v2/t.abc" '' 'pre' \
    "${ERR_CLASSES_ALREADY_LOADED}" - "${WANT_V1_RUN}" "L1" \
    "pre tag=V1 sub=V1 n=2 calc=1 base=100 lazy=L1"

# AOT-compiled base + partial patch must be REFUSED. AOT code may inline a method of a
# patched class into a caller the patch does not carry (here: the base was compiled with
# the old `partial` targets); prepending the patch changes class lookup but does not
# invalidate that compiled code, so the fallback caller would keep the old inlined body.
# Without the refusal this case is a silent wrong-answer: rc=0 and `run` reporting
# whatever the interpreter picked up while AOT methods still ran the old bodies.
function run_aot_case() {
    local name="aot_partial"
    local log="${OUT_DIR}/${name}.log"
    local an="${OUT_DIR}/base/t.an"

    if ! "${ARK_AOT}" --paoc-panda-files="${OUT_DIR}/base/t.abc" \
            --paoc-output="${an}" \
            --paoc-location="${OUT_DIR}/base" \
            --boot-panda-files="${ETSSTDLIB}" \
            --load-runtimes=ets \
            --compiler-enable-jit=false >"${OUT_DIR}/${name}.aot.log" 2>&1; then
        echo "  ark_aot failed, see ${OUT_DIR}/${name}.aot.log"
        echo "FAIL  ${name}"
        FAILURES=$((FAILURES + 1))
        return
    fi

    COLDRELOAD_AOT_FILE="${an}" \
    timeout "${MT_TIMEOUT_SECONDS}" "${COLDRELOAD_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/base/t.abc" "${OUT_DIR}/partial/t.abc" '' '' >"${log}" 2>&1
    local exitcode=$?

    local rc run
    rc="$(sed -n 's/^rc=\([0-9-]*\)$/\1/p' "${log}" | head -1)"
    run="$(grep -m1 '^run ' "${log}")"

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    [[ "${rc}" == "${ERR_AOT_LOADED}" ]] ||
        { echo "  rc: got '${rc}' want '${ERR_AOT_LOADED}' (AOT must be refused)"; ok=0; }
    [[ "${run}" == "${WANT_V1_RUN}" ]] ||
        { echo "  run: got '${run}' want '${WANT_V1_RUN}' (refused reload must change nothing)"; ok=0; }
    grep -q "AOT files are loaded, cold reload is not available" "${log}" ||
        { echo "  missing AOT refusal diagnostic"; ok=0; }
    ! grep -q "prepended to abcFiles" "${log}" || { echo "  refusal reported success"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_aot_case

if [[ ${FAILURES} -eq 0 ]]; then
    echo "ALL PASS"
else
    echo "${FAILURES} FAILURE(S)"
fi
exit "${FAILURES}"
