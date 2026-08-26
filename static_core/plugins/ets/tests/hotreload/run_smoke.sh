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
# Hotreload MVP end-to-end smoke test.
#
#   Usage: run_smoke.sh [BUILD_DIR] [OUT_DIR]
#     BUILD_DIR  panda build directory (default: <repo>/build)
#     OUT_DIR    scratch directory for the compiled abc files (default: BUILD_DIR/hotreload-smoke)
#
# Exit code 0 means every case passed.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PANDA_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"
BUILD_DIR="${1:-${PANDA_ROOT}/build}"
OUT_DIR="${2:-${BUILD_DIR}/hotreload-smoke}"

# Absolute paths everywhere. Two reasons: the target abc is matched against `File::GetFilename()`,
# which stores the string the loader was given verbatim, and the package reader resolves archive
# paths independently of this script's working directory.
mkdir -p "${OUT_DIR}"
BUILD_DIR="$(cd "${BUILD_DIR}" && pwd)"
OUT_DIR="$(cd "${OUT_DIR}" && pwd)"

# Native driver that binds `hotReloadNative` in the test variants and drives the bare-path
# transaction (tests/ani/hotreload_smoke).
HR_APP="${HR_APP:-${BUILD_DIR}/bin/ani_hotreload_smoke}"
ETSSTDLIB="${BUILD_DIR}/plugins/ets/etsstdlib.abc"
# A Debug build does not ship es2panda (it uses the host compiler for the stdlib), so fall back to
# the default release build. Override with ES2PANDA=<path> if neither applies.
ES2PANDA="${ES2PANDA:-${BUILD_DIR}/bin/es2panda}"
if [[ ! -f "${ES2PANDA}" ]]; then
    ES2PANDA="${PANDA_ROOT}/build/bin/es2panda"
fi

for tool in "${ES2PANDA}" "${HR_APP}" "${ETSSTDLIB}"; do
    if [[ ! -f "${tool}" ]]; then
        echo "FATAL: not found: ${tool}" >&2
        exit 2
    fi
done

# `ark::hotreload::Error` ordinals, keep in sync with runtime/hotreload/hotreload.h
readonly ERR_NONE=0
readonly ERR_CLASS_UNMODIFIABLE=8
readonly ERR_FIELD_CHANGED=17
readonly ERR_TARGET_ABC_AMBIGUOUS=21
readonly ERR_PATCH_OPEN_FAILED=22
readonly ERR_CALLER_NOT_BOOT=28

# The multi-coroutine case must finish well within this; exceeding it means a deadlock.
readonly MT_TIMEOUT_SECONDS=120

mkdir -p "${OUT_DIR}"
FAILURES=0

function compile() {
    # compile <variant> -> ${OUT_DIR}/<variant>/t.abc
    local variant="$1"
    mkdir -p "${OUT_DIR}/${variant}"
    if ! "${ES2PANDA}" --extension=ets \
            --output="${OUT_DIR}/${variant}/t.abc" \
            "${SCRIPT_DIR}/${variant}/t.ets" >"${OUT_DIR}/${variant}/es2panda.log" 2>&1; then
        echo "FATAL: es2panda failed for ${variant}, see ${OUT_DIR}/${variant}/es2panda.log" >&2
        exit 2
    fi
}

function package() {
    # package <abc-path> <package-path>
    #
    # Builds a hap/hsp/hqf-shaped archive: a plain zip whose `ets/modules_static.abc` entry holds
    # the abc. Entries are STORED (`-0`), like real packages, because the runtime mmaps them.
    local abc="$1" pkg="$2"
    local stage="${OUT_DIR}/stage"
    rm -rf "${stage}" "${pkg}"
    mkdir -p "${stage}/ets"
    cp "${abc}" "${stage}/ets/modules_static.abc"
    (cd "${stage}" && zip -0 -X -q -r "${pkg}" ets)
    rm -rf "${stage}"
}

function run_case() {
    # run_case <name> <program> <patch> <expected-rc> <expected-after-tag> <expected-after-calc>
    #          <expected-lazy> [target]
    #
    #   <program> the abc `ark` runs
    #   [target]  the path handed to hotreload as the abc to replace; defaults to <program>, and
    #             differs from it only when the framework would name the same file by its package
    local name="$1" program="$2" patch="$3"
    local want_rc="$4" want_tag="$5" want_calc="$6" want_lazy="$7"
    local target="${8:-$2}"
    local log="${OUT_DIR}/${name}.log"

    # --compiler-enable-jit=false is REQUIRED: the option defaults to true and hotreload refuses
    # to run under JIT (Error::JIT_ENABLED).
    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${program}" "${target}" "${patch}" >"${log}" 2>&1
    local exitcode=$?

    local before after rc lazy
    before="$(grep -m1 '^before ' "${log}")"
    after="$(grep -m1 '^after ' "${log}")"
    rc="$(sed -n 's/^rc=\([0-9-]*\)$/\1/p' "${log}" | head -1)"
    lazy="$(sed -n 's/^lazy=\(.*\)$/\1/p' "${log}" | head -1)"

    local want_after="after  tag=${want_tag} n=2 calc=${want_calc} base=777"
    local want_before="before tag=V1 n=2 calc=1 base=777"

    local ok=1
    [[ ${exitcode} -eq 0 ]]           || { echo "  ark exited with ${exitcode}"; ok=0; }
    [[ "${before}" == "${want_before}" ]] || { echo "  before: got '${before}' want '${want_before}'"; ok=0; }
    [[ "${rc}" == "${want_rc}" ]]     || { echo "  rc: got '${rc}' want '${want_rc}'"; ok=0; }
    [[ "${after}" == "${want_after}" ]]   || { echo "  after : got '${after}' want '${want_after}'"; ok=0; }
    [[ "${lazy}" == "${want_lazy}" ]] || { echo "  lazy: got '${lazy}' want '${want_lazy}'"; ok=0; }
    # A successful reload must announce itself as COMPLETE. Without this, a regression that starts
    # silently dropping classes would still show green everywhere else in this case.
    if [[ "${want_rc}" == "${ERR_NONE}" ]]; then
        grep -q "HOTRELOAD_RESULT .* skipped=0$" "${log}" ||
            { echo "  no 'HOTRELOAD_RESULT ... skipped=0' line"; ok=0; }
        ! grep -q "HOTRELOAD_SKIPPED_CLASS" "${log}" ||
            { echo "  unexpected HOTRELOAD_SKIPPED_CLASS"; ok=0; }
    else
        # a failed reload must NOT claim a result
        ! grep -q "HOTRELOAD_RESULT" "${log}" || { echo "  HOTRELOAD_RESULT on a failed reload"; ok=0; }
    fi

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

echo "== compiling =="
compile basic_base
compile basic_patch
compile patch_add_field
compile workers_base
compile workers_patch
compile partial_v1
compile partial_v2
compile partial_v3
compile inherit_base
compile inherit_patch
compile repeat_base
compile repeat_patch
compile ambiguity_driver
compile fields_base
compile patch_field_order
compile patch_field_retype
compile funcval_base
compile funcval_patch
compile patch_partial
compile chain_v1
compile chain_v2
compile chain_v3
compile concurrent_base
compile concurrent_patch

echo "== packaging =="
mkdir -p "${OUT_DIR}/pkg"
package "${OUT_DIR}/basic_patch/t.abc" "${OUT_DIR}/pkg/patch.hqf"

# A package holding a perfectly valid abc, but NOT at the entry the loader looks for. It exists to
# pin down that a package which cannot be unpacked is refused rather than quietly loaded some other
# way: `OpenPandaFileOrZip` would happily open `classes.abc` here, and the hap/hsp loader does fall
# back to it. The transaction must not.
rm -rf "${OUT_DIR}/stage" "${OUT_DIR}/pkg/wrong-entry.hqf"
mkdir -p "${OUT_DIR}/stage"
cp "${OUT_DIR}/basic_patch/t.abc" "${OUT_DIR}/stage/classes.abc"
(cd "${OUT_DIR}/stage" && zip -0 -X -q "${OUT_DIR}/pkg/wrong-entry.hqf" classes.abc)
rm -rf "${OUT_DIR}/stage"

# Reproduce the on-device path shape for the TARGET.
#
# `ark` cannot boot from a package (it resolves the entry point itself and only understands a plain
# abc or a `classes.abc` zip), while on a device the framework loads the hap through
# `AbcFile.loadAbcFile`. What matters for hotreload is only the resulting path: the loader records
# the DERIVED `<package without suffix>/ets/modules_static.abc` in `File::GetFilename()`, never the
# `.hap` string the caller passed. So the abc is placed at exactly that derived location and the
# test passes the `.hap` path as the target, which is what the framework would pass. That exercises
# the derivation rule end to end without needing `ark` to boot from an archive.
readonly PKG_BASE="${OUT_DIR}/pkg/target"
mkdir -p "${PKG_BASE}/ets"
cp "${OUT_DIR}/basic_base/t.abc" "${PKG_BASE}/ets/modules_static.abc"
package "${OUT_DIR}/basic_base/t.abc" "${PKG_BASE}.hap"

readonly BARE_TARGET="${OUT_DIR}/basic_base/t.abc"

echo "== running =="
# A1/A2/A3/A4/A5/A6: method bodies swapped, instance and static state preserved
# A9: a class first used AFTER the reload is loaded from the patch (lazy=L2)
run_case positive   "${BARE_TARGET}" "${OUT_DIR}/basic_patch/t.abc"        "${ERR_NONE}"              V2 42 L2
# A7: structural change (added field) refused, nothing modified
run_case patch_add_field  "${BARE_TARGET}" "${OUT_DIR}/patch_add_field/t.abc" "${ERR_FIELD_CHANGED}"     V1 1  L1
# A8: unreadable patch refused, nothing modified
run_case neg_nofile "${BARE_TARGET}" "${OUT_DIR}/does-not-exist.abc" "${ERR_PATCH_OPEN_FAILED}" V1 1  L1
# A10: same thing through the framework's entry, `AbcRuntimeLinker.hotReload` -- driven from
# native code by tests/ani/hotreload_app, since the entry is not callable from ArkTS
# A11: device shape end to end -- target named by its .hap path, patched from a .hqf.
# The program itself runs from the derived location, see the packaging step above.
run_case hqf        "${PKG_BASE}/ets/modules_static.abc" "${OUT_DIR}/pkg/patch.hqf" "${ERR_NONE}" V2 42 L2 \
                    "${PKG_BASE}.hap"
# A12: a package that cannot be unpacked is refused, nothing modified
run_case neg_badpkg "${BARE_TARGET}" "${OUT_DIR}/pkg/does-not-exist.hqf" "${ERR_PATCH_OPEN_FAILED}" V1 1  L1
# A16: a package whose abc sits at the wrong entry is refused, NOT silently opened another way
run_case neg_wrongentry "${BARE_TARGET}" "${OUT_DIR}/pkg/wrong-entry.hqf" "${ERR_PATCH_OPEN_FAILED}" V1 1  L1
# A20: the entry invoked under a non-boot managed frame is refused (caller authorization);
#      the bound native calls the entry itself while the bridge's `run` frame is on the stack
HR_VIA_ENTRY=1 run_case caller_not_boot "${BARE_TARGET}" "${OUT_DIR}/basic_patch/t.abc" "${ERR_CALLER_NOT_BOOT}" V1 1  L1

# A17: incremental patch -- ONLY `Greeter` is carried. The swapped class comes from the patch
# (tag=V2), the loaded-but-omitted `Calc` keeps its code (calc=1), and the NEVER-loaded
# `LazyHolder`/`Lazy` must still resolve after the reload: lookup falls through the patch to the
# retained old file (lazy=L1). Regression: the slot used to be REPLACED, stranding every class
# the patch omits.
run_case inc "${BARE_TARGET}" "${OUT_DIR}/patch_partial/t.abc" "${ERR_NONE}" V2 1  L1

function run_chain_case() {
    # run_chain_case <name> -- two successive INCREMENTAL reloads, each patch carrying only
    # `Counter`, followed by the first-ever use of `Late`, which no patch contains
    local name="$1"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/chain_v1/t.abc" "${OUT_DIR}/chain_v1/t.abc" "${OUT_DIR}/chain_v2/t.abc" \
            "${OUT_DIR}/chain_v3/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    # `Counter` resolves to the newest generation that carries it, and the instance field
    # survives both reloads
    local want=("gen0=V1 n=1" "rc1=0 gen1=V2 n=2" "rc2=0 gen2=V3 n=3")
    local keys=("gen0=" "rc1=" "rc2=")
    for i in 0 1 2; do
        local got
        got="$(grep -m1 "^${keys[$i]}" "${log}")"
        [[ "${got}" == "${want[$i]}" ]] || { echo "  got '${got}' want '${want[$i]}'"; ok=0; }
    done
    # `Late` is in no patch and unused until now: it must fall through BOTH patches to the
    # retained original file
    [[ "$(grep -m1 '^late=' "${log}")" == "late=LATE1" ]] ||
        { echo "  late class did not resolve through the patch chain to the original file"; ok=0; }
    # Both generations carry `Counter` and the module globals, and drop nothing
    [[ "$(grep -c "HOTRELOAD_RESULT .* reloaded=2 skipped=0$" "${log}")" == "2" ]] ||
        { echo "  both generations did not report reloaded=2 skipped=0"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

function run_gen_case() {
    # run_gen_case <name> -- two successive PARTIAL reloads driven by ONE stable target path
    local name="$1"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/partial_v1/t.abc" "${OUT_DIR}/partial_v1/t.abc" "${OUT_DIR}/partial_v2/t.abc" \
            "${OUT_DIR}/partial_v3/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    # generation N must be live after reload N, and the instance field must survive both
    local want=("gen0=V1 n=1" "rc1=0 gen1=V2 n=2" "rc2=0 gen2=V3 n=3")
    local keys=("gen0=" "rc1=" "rc2=")
    for i in 0 1 2; do
        local got
        got="$(grep -m1 "^${keys[$i]}" "${log}")"
        [[ "${got}" == "${want[$i]}" ]] || { echo "  got '${got}' want '${want[$i]}'"; ok=0; }
    done

    # `Marker` cannot be swapped. It remains published from generation 1, so each transaction must
    # carry it forward, report the skip, and count the partial result instead of silently losing it.
    [[ "$(grep -c "HOTRELOAD_SKIPPED_CLASS Lt/Marker; reason=interface$" "${log}")" == "2" ]] ||
        { echo "  Marker was not reported as skipped by both generations"; ok=0; }
    [[ "$(grep -c "HOTRELOAD_RESULT .* reloaded=2 skipped=1$" "${log}")" == "2" ]] ||
        { echo "  both generations did not report reloaded=2 skipped=1"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

function run_mt_case() {
    # run_mt_case <name> <workers> <want-started> <mode> <want-new-entry>
    #   <want-started>   expected value of the "workers already spinning" counter, or "any"
    #   <mode>           "wait" or "nowait", see the test source
    #   <want-new-entry> how many coroutines ENTERED the new `worker` body, which is a different
    #                    question from whether the call it makes reached new code. A coroutine that
    #                    was already running keeps its frame and finishes on the old body, which is
    #                    documented behaviour --- so the `wait` case expects 0. A coroutine that had
    #                    NOT started enters through the function value's dispatch, which resolves
    #                    `worker` freshly after the commit --- so the `nowait` case expects 4. The
    #                    second number went from 0 to 4 when the patch file's own `PandaCache` was
    #                    included in the commit's cache sweep; see "function values and functions
    #                    called by name" in docs/05-status-and-gaps.md.
    local name="$1" workers="$2" want_started="$3" mode="$4" want_new_entry="$5"
    local log="${OUT_DIR}/${name}.log"

    # A hard timeout is the point of this case: a broken STW shows up as a hang, not as bad output.
    HR_WORKERS="${workers}" timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/workers_base/t.abc" "${OUT_DIR}/workers_base/t.abc" \
            "${OUT_DIR}/workers_patch/t.abc" "${mode}" >"${log}" 2>&1
    local exitcode=$?

    local started rc done on_new on_new_entry
    started="$(sed -n 's/^started=\([0-9]*\)$/\1/p' "${log}" | head -1)"
    rc="$(sed -n 's/^rc=\([0-9-]*\)$/\1/p' "${log}" | head -1)"
    done="$(sed -n 's/^done=\([0-9]*\)$/\1/p' "${log}" | head -1)"
    on_new="$(sed -n 's/^onNewBody=\([0-9]*\)$/\1/p' "${log}" | head -1)"
    on_new_entry="$(sed -n 's/^onNewEntry=\([0-9]*\)$/\1/p' "${log}" | head -1)"

    local ok=1
    if [[ ${exitcode} -eq 124 ]]; then
        echo "  TIMED OUT after ${MT_TIMEOUT_SECONDS}s -- STW most likely deadlocked"
        ok=0
    fi
    [[ ${exitcode} -eq 0 ]]          || { echo "  ark exited with ${exitcode}"; ok=0; }
    if [[ "${want_started}" != "any" ]]; then
        # the workers must really have been inside their loop, else the case is vacuous
        [[ "${started}" == "${want_started}" ]] ||
            { echo "  started: got '${started}' want '${want_started}'"; ok=0; }
    fi
    [[ "${rc}" == "${ERR_NONE}" ]]   || { echo "  rc: got '${rc}' want '${ERR_NONE}'"; ok=0; }
    [[ "${done}" == "4" ]]           || { echo "  done: got '${done}' want '4'"; ok=0; }
    # every worker individually, not just one shared observation
    [[ "${on_new}" == "4" ]]         || { echo "  onNewBody: got '${on_new}' want '4'"; ok=0; }
    [[ "${on_new_entry}" == "${want_new_entry}" ]] ||
        { echo "  onNewEntry: got '${on_new_entry}' want '${want_new_entry}'"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

# A15: two successive partial reloads driven by one stable target path (the IDE pressing the button twice)
run_gen_case gen_three

# A18: the same, with each patch carrying only ONE class, plus a class that no patch contains
run_chain_case chain_inc

function run_concurrent_case() {
    # run_concurrent_case <name> -- reloads racing real worker threads that lazily load classes
    #
    # Every reload must either commit (rc 0) or be refused with CONCURRENT_LOADING (rc 27) when it
    # raced a load; after the workers finish, a final reload must succeed and every class of the
    # target file must report the new generation. The red behaviour (a lost update) is
    # probabilistic -- it needs a load to land in a collection window -- while the green one is
    # deterministic.
    local name="$1"
    local log="${OUT_DIR}/${name}.log"

    HR_WORKERS=2 timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/concurrent_base/t.abc" "${OUT_DIR}/concurrent_base/t.abc" \
            "${OUT_DIR}/concurrent_patch/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    [[ "$(grep -m1 '^ok=' "${log}")" =~ ^ok=[0-9]+\ refused=[0-9]+$ ]] ||
        { echo "  no 'ok=<n> refused=<n>' summary"; ok=0; }
    [[ -z "$(grep -m1 '^unexpected rc=' "${log}")" ]] ||
        { echo "  a reload returned an unexpected code"; ok=0; }
    [[ -z "$(grep -m1 '^stale=' "${log}")" ]] ||
        { echo "  a class was silently left on the old generation"; ok=0; }
    [[ "$(grep -m1 '^all=' "${log}")" == "all=C2" ]] ||
        { echo "  final generation check missing"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

# A19: concurrent lazy loading of target classes vs the transaction -- a load that lands in the
# collection window must abort the reload (CONCURRENT_LOADING), never be lost by a success
run_concurrent_case concurrent_load

function run_ambiguous_target_case() {
    # Error 21 must remain a real refusal after superseded generations stop looking ambiguous.
    # Two independent linkers open the same path and each publish its own `t.Counter` Class/File.
    local name="target_ambiguous"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/ambiguity_driver/t.abc" "${OUT_DIR}/partial_v1/t.abc" \
            "${OUT_DIR}/partial_v2/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    [[ "$(grep -m1 '^distinct=' "${log}")" == "distinct=true" ]] ||
        { echo "  the two linker contexts did not publish distinct classes"; ok=0; }
    [[ "$(grep -m1 '^rc=' "${log}")" == "rc=${ERR_TARGET_ABC_AMBIGUOUS}" ]] ||
        { echo "  target was not refused with rc=${ERR_TARGET_ABC_AMBIGUOUS}"; ok=0; }
    grep -q "resolves to more than one live panda file" "${log}" ||
        { echo "  missing target-ambiguity diagnostic"; ok=0; }
    ! grep -q "HOTRELOAD_SKIPPED_CLASS" "${log}" || { echo "  class was skipped on a refused reload"; ok=0; }
    ! grep -q "HOTRELOAD_RESULT" "${log}" || { echo "  HOTRELOAD_RESULT on a refused reload"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_ambiguous_target_case

# A23: the linker-specific entry point carries its own context -- even when another linker has
#      opened the same path, each call must update only the generation belonging to its receiver.
#      Lives in tests/ani/hotreload_app since the entry became protected (native-only, exactly
#      how the framework calls it).

function run_field_storage_case() {
    # run_field_storage_case <name> <patch-variant>
    #
    # The two ways a patch can keep every DECLARED property of a field and still describe different
    # storage. Both are legal source edits, both used to pass validation, and the swap installs the
    # patch's `Field` array --- offsets included --- on the class the live objects point at.
    #
    #   patch_field_order    two same-width fields swapped in the source, so their offsets swap
    #   patch_field_retype  a reference field retyped, which `Field::GetTypeId()` cannot see
    #
    # Both patches carry V2 method bodies on purpose, the same trick as `patch_add_field`: with V1 bodies
    # a wrongly applied patch would print exactly what a refused one prints.
    #
    # `a=11 b=22` are assigned at runtime and are not what either declaration initialises to, so a
    # swapped offset shows up as values no version of the source could have produced. Verified
    # without the fix: the reorder patch returns rc=0 and the live object then reads `a=22 b=11`.
    #
    # The retype case asserts only the refusal. Making its corruption visible needs a method that
    # exists on the new type and not the old one, which the baseline cannot compile --- so the
    # observable is that the patch is refused, not what it would have done.
    local name="$1" patch="$2"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/fields_base/t.abc" "${OUT_DIR}/fields_base/t.abc" \
            "${OUT_DIR}/${patch}/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    local want=("before a=11 b=22 r=A tag=V1" \
                "rc=${ERR_FIELD_CHANGED}" \
                "after  a=11 b=22 r=A tag=V1")
    local keys=("before " "rc=" "after  ")
    for i in 0 1 2; do
        local got
        got="$(grep -m1 "^${keys[$i]}" "${log}")"
        [[ "${got}" == "${want[$i]}" ]] || { echo "  got '${got}' want '${want[$i]}'"; ok=0; }
    done
    ! grep -q "HOTRELOAD_RESULT" "${log}" || { echo "  HOTRELOAD_RESULT on a refused reload"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_field_storage_case fld_order   patch_field_order
run_field_storage_case fld_reftype patch_field_retype

function run_function_value_case() {
    # Calling a function BY NAME and calling it THROUGH A VALUE must agree after a reload. `f` is
    # an ordinary function and only its body changes, so every shape below reaches the new body:
    #
    #   direct  calling `f` by name
    #   local   through a value taken before the reload
    #   static  same, parked in a static field
    #   fresh   through a value taken after the reload
    #
    # The value shapes go through the patch file's OWN `PandaCache`: loading the temporary classes
    # during Prepare resolves the function-reference target against the outgoing generation and
    # caches it in the patch file, and that file is not registered with the class linker yet when
    # the commit sweeps the registered files' caches. The commit clears the transaction's own files
    # explicitly (`ClearAllPandaCaches`); this case turns red if that sweep is lost, and `fresh`
    # is the shape that distinguishes the cache from a stale capture. See "function values and
    # functions called by name" in docs/05-status-and-gaps.md.
    #
    # The `eq` line measures the other half, which is identity rather than dispatch: `late` comes
    # from a reference site whose synthesised class is first loaded AFTER the reload, so it resolves
    # its target against the new generation, while the value taken beforehand carries a cached
    # pointer the swap does not reach (`EtsClass::typeMetaData_`). Equal only if the commit
    # re-points that slot -- `RedirectFunctionReferenceTargets`.
    local name="fnval"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/funcval_base/t.abc" "${OUT_DIR}/funcval_base/t.abc" \
            "${OUT_DIR}/funcval_patch/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    local want=("before direct=100 local=100 static=100 tag=V1" \
                "rc=${ERR_NONE}" \
                "after  direct=200 local=200 static=200 fresh=200" \
                "eq     late=200 eq(local,late)=true")
    local keys=("before " "rc=" "after  " "eq     ")
    for i in 0 1 2 3; do
        local got
        got="$(grep -m1 "^${keys[$i]}" "${log}")"
        [[ "${got}" == "${want[$i]}" ]] || { echo "  got '${got}' want '${want[$i]}'"; ok=0; }
    done
    # The reload itself is complete: nothing was skipped, so the difference above is not a drop
    grep -q "HOTRELOAD_RESULT .* skipped=0$" "${log}" ||
        { echo "  no 'HOTRELOAD_RESULT ... skipped=0' line"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_function_value_case

function run_inh_case() {
    # A17-A20: a module of several classes that know about each other, plus a handle cached over the
    # reload. Each of the four observations is a separate repair, so this one case fails four
    # different ways:
    #   who=  inherited vtable slot            area= inherited ITable / IMT entry
    #   own=  the class's own method           refl= the ANI / reflection redirect table
    # It also pins down that an INTERFACE in the module does not make the module un-reloadable:
    # `Shape` is loaded and collected, cannot be swapped, and must be dropped rather than refused.
    local name="inheritance"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/inherit_base/t.abc" "${OUT_DIR}/inherit_base/t.abc" \
            "${OUT_DIR}/inherit_patch/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    local want=("brk=erroneous" \
                "before who=BASE-V1 area=1 own=OWN-V1 refl=OWN-V1" \
                "rc=${ERR_NONE}" \
                "after  who=BASE-V2 area=42 own=OWN-V2 refl=OWN-V2")
    local keys=("brk=" "before " "rc=" "after  ")
    for i in 0 1 2 3; do
        local got
        got="$(grep -m1 "^${keys[$i]}" "${log}")"
        [[ "${got}" == "${want[$i]}" ]] || { echo "  got '${got}' want '${want[$i]}'"; ok=0; }
    done
    # The tooling contract for a PARTIAL reload: rc is 0, so the log is the only thing that can
    # tell the IDE which edits did not take effect. Both classes named, with their reason token,
    # and a result line that says the reload was incomplete.
    grep -q "HOTRELOAD_SKIPPED_CLASS Lt/Shape; reason=interface$" "${log}" ||
        { echo "  the interface was not reported as skipped"; ok=0; }
    grep -q "HOTRELOAD_SKIPPED_CLASS Lt/Broken; reason=erroneous$" "${log}" ||
        { echo "  the erroneous class was not reported as skipped"; ok=0; }
    # The driver's bridge class is part of the module and reloads with it, hence reloaded=4.
    grep -q "HOTRELOAD_RESULT .* reloaded=4 skipped=2$" "${log}" ||
        { echo "  no 'HOTRELOAD_RESULT ... reloaded=4 skipped=2' line"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_inh_case

function run_repeat_case() {
    # A21: many reloads in a row. Guards the per-reload cost, which `gen_three` cannot see: work
    # that is proportional to the number of generations already registered stays invisible at two
    # reloads and turns into a timeout here.
    local name="repeat"
    local log="${OUT_DIR}/${name}.log"

    timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${OUT_DIR}/repeat_base/t.abc" "${OUT_DIR}/repeat_base/t.abc" \
            "${OUT_DIR}/repeat_patch/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    if [[ ${exitcode} -eq 124 ]]; then
        echo "  TIMED OUT after ${MT_TIMEOUT_SECONDS}s -- per-reload cost most likely grows with the reload count"
        ok=0
    fi
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    local want=("reloads=400" "tag=V1")
    local keys=("reloads=" "tag=")
    for i in 0 1; do
        local got
        got="$(grep -m1 "^${keys[$i]}" "${log}")"
        [[ "${got}" == "${want[$i]}" ]] || { echo "  got '${got}' want '${want[$i]}'"; ok=0; }
    done

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_repeat_case

function run_boot_case() {
    # A22: a class loaded into the BOOT context is never swapped, whatever the patch says.
    #
    # This is the one refusal that is not about the shape of the change. The runtime resolves the
    # well-known stdlib entities once and keeps the raw `Method *` in plain C++ members
    # (`EtsPlatformTypes`), read directly on hot paths with no conversion boundary anywhere for the
    # redirect table to sit on; the dispatch-table repair cannot reach them either. So
    # `EtsHotreload::LangSpecificValidateClasses` refuses the whole transaction rather than dropping
    # the class the way the un-swappable SHAPES are dropped -- a boot class in the set means the
    # caller is not in the situation it thinks it is in, and a skip would report that as a success
    # with `reloaded=0`.
    #
    # Unreachable from the supported direction: a module's patch abc cannot declare `Lstd/core/...;`.
    # It is reached from the other side instead. Listing the program's own abc in
    # `--boot-panda-files` makes `Runtime::CreateApplicationClassLinkerContext` return early -- the
    # file is already loaded -- so no application context is ever created and the executable's own
    # classes carry the boot context. Everything else about the run is the `positive` case verbatim,
    # which is what makes the comparison mean something: the same patch that reloads cleanly there
    # is refused here, and refused for this reason.
    local name="boot_ctx"
    local log="${OUT_DIR}/${name}.log"

    HR_BOOT_APPEND=1 timeout "${MT_TIMEOUT_SECONDS}" "${HR_APP}" "${ETSSTDLIB}" \
            "${BARE_TARGET}" "${BARE_TARGET}" "${OUT_DIR}/basic_patch/t.abc" >"${log}" 2>&1
    local exitcode=$?

    local ok=1
    [[ ${exitcode} -eq 0 ]] || { echo "  driver exited with ${exitcode}"; ok=0; }
    # Refused, and nothing modified: the `after` line must still read exactly like `before`.
    local want=("before tag=V1 n=2 calc=1 base=777" \
                "rc=${ERR_CLASS_UNMODIFIABLE}" \
                "after  tag=V1 n=2 calc=1 base=777" \
                "lazy=L1")
    local keys=("before " "rc=" "after  " "lazy=")
    for i in 0 1 2 3; do
        local got
        got="$(grep -m1 "^${keys[$i]}" "${log}")"
        [[ "${got}" == "${want[$i]}" ]] || { echo "  got '${got}' want '${want[$i]}'"; ok=0; }
    done
    # Refused for the RIGHT reason. Without this the case would still pass if the transaction
    # started failing earlier for some unrelated reason, e.g. the target no longer being found.
    grep -q "belongs to the boot context" "${log}" ||
        { echo "  not refused as a boot-context class"; ok=0; }
    # A refusal is not a partial reload: no class may be dropped and no result may be claimed.
    ! grep -q "HOTRELOAD_SKIPPED_CLASS" "${log}" || { echo "  a class was dropped, not refused"; ok=0; }
    ! grep -q "HOTRELOAD_RESULT" "${log}" || { echo "  HOTRELOAD_RESULT on a refused reload"; ok=0; }

    if [[ ${ok} -eq 1 ]]; then
        echo "PASS  ${name}"
    else
        echo "FAIL  ${name}  (log: ${log})"
        FAILURES=$((FAILURES + 1))
    fi
}

run_boot_case

# A13: reload while several coroutines are RUNNING managed code on real worker threads.
#      This is the case that makes the commit phase actually suspend and resume other mutators.
# One worker more than the spinning coroutines: `main` busy-waits on a worker of its own, and none
# of the spinners yields, so with only WORKERS workers the last coroutine would never get scheduled.
run_mt_case mt_workers4 5 4 wait 0
# A14: reload while the coroutines exist but have NOT started yet. With a single worker they are
#      fibers queued behind the main coroutine, so they only begin after the transaction. This is
#      what a device does all the time (posted tasks that have not been picked up yet). The queued
#      entry dispatches through the function value taken at `launch` time, whose resolution happens
#      after the commit, so these coroutines enter the NEW body: onNewEntry=4. Historically this is
#      the case that uncovered the missing obsolete-class mutex registration, back when the pending
#      patch still cached the old `worker`; today what it covers is that cache sweep.
run_mt_case mt_workers1 1 0 nowait 4

echo "== done =="
if [[ ${FAILURES} -eq 0 ]]; then
    echo "ALL PASS"
    exit 0
fi
echo "${FAILURES} case(s) FAILED"
exit 1
