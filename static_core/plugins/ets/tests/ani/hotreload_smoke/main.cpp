/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Native driver for the hotreload smoke suite.
 *
 * The test variants (tests/hotreload/<variant>/t.ets) declare the static native
 * `hotReloadNative` and drive everything from `HotReloadBridge.run`; this driver loads the
 * program and calls `run` with three plain string arguments whose meaning each variant
 * defines itself. The implementation of `hotReloadNative` runs the transaction the way the
 * removed `std.debug` probe did: paths only, target resolved across live panda files.
 *
 * Usage: hotreload_smoke <etsstdlib.abc> <program.abc> <a0> <a1> <a2>
 * Environment:
 *   HR_BOOT_APPEND=1   add <program.abc> to the boot panda files (the boot-context case);
 *                      the bridge class is then found through the boot context
 *   HR_WORKERS=N       pass --coroutine-workers-count=N (the concurrent cases)
 */

#include <ani.h>

#include <array>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string>

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/hotreload/ets_hotreload.h"
#include "runtime/include/thread_scopes.h"

// The protected entry the via-entry mode drives directly; intrinsics have no public header.
extern "C" ark::ets::EtsInt EtsAbcRuntimeLinkerHotReload(ark::ets::EtsAbcRuntimeLinker *runtimeLinker,
                                                         ark::ets::EtsString *targetPath,
                                                         ark::ets::EtsString *patchPath);

namespace {

using ark::ets::EtsCoroutine;

// The single place raw argv indexing is allowed; every caller names its index.
const char *ArgAt(char **argv, int index)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return argv[index];
}

bool Check(ani_status status, const char *msg)
{
    if (status != ANI_OK) {
        std::cerr << "FAIL: " << msg << ", status=" << static_cast<int>(status) << "\n";
        return true;
    }
    return false;
}

// The linker object the via-entry native invokes the entry on; set after CreateLinker. Kept as an
// ani_object so the GC keeps it alive, and re-interpreted inside the native after any move.
ani_object g_entryLinker = nullptr;

// The bound `hotReloadNative`: the bare-path transaction. The target is resolved across the
// live panda files, exactly like the probe this driver replaces.
ani_int NativeHotReload([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class cls, ani_string target,
                        ani_string patch)
{
    auto *coro = EtsCoroutine::GetCurrent();
    if (target == nullptr || patch == nullptr) {
        return static_cast<ani_int>(ark::hotreload::Error::INVALID_INPUT_ARG);
    }

    // Copy the strings out while still in managed state: the transaction runs with the raw
    // `EtsString *` no longer reachable, and a handle would have to be dereferenced anyway.
    ark::ets::ani::ScopedManagedCodeFix scope(env);
    auto *targetStr = scope.ToInternalType(target);
    auto *patchStr = scope.ToInternalType(patch);
    ark::PandaString targetPath = targetStr->GetMutf8();
    ark::PandaString patchPath = patchStr->GetMutf8();

    ark::hotreload::Error err;
    {
        // `ArkHotreloadBase` must be constructed in NATIVE state; its own managed scope switches back
        ark::ScopedNativeCodeThread nativeScope(coro);
        ark::ets::hotreload::EtsHotreload reload(coro);
        err = reload.ReplaceAbc(targetPath, patchPath);
    }

    if (err != ark::hotreload::Error::NONE) {
        LOG(ERROR, HOTRELOAD) << "hotReload failed: " << ark::hotreload::GetErrorString(err);
    } else {
        LOG(INFO, HOTRELOAD) << "hotReload succeeded";
    }
    return static_cast<ani_int>(err);
}

// The bound `hotReloadNative` in HR_VIA_ENTRY mode: invokes the protected entry itself, the way
// the framework does -- but under the bridge's managed frame, so the nearest managed caller is
// the test module's class, not a boot one. Pins down the caller-authorization check.
ani_int NativeHotReloadViaEntry(ani_env *env, [[maybe_unused]] ani_class cls, ani_string target, ani_string patch)
{
    if (target == nullptr || patch == nullptr || g_entryLinker == nullptr) {
        return static_cast<ani_int>(ark::hotreload::Error::INVALID_INPUT_ARG);
    }
    ark::ets::ani::ScopedManagedCodeFix scope(env);
    auto *targetStr = scope.ToInternalType(target);
    auto *patchStr = scope.ToInternalType(patch);
    auto *linker = ark::ets::EtsAbcRuntimeLinker::FromEtsObject(scope.ToInternalType(g_entryLinker));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return EtsAbcRuntimeLinkerHotReload(linker, targetStr, patchStr);
}

// Creates an AbcRuntimeLinker whose abcFiles hold the program abc, the way the
// application framework builds the linker for a module.
ani_object CreateLinker(ani_env *env, const char *programAbc)
{
    ani_class linkerClass = nullptr;
    if (Check(env->FindClass("std.core.AbcRuntimeLinker", &linkerClass), "cannot find std.core.AbcRuntimeLinker")) {
        return nullptr;
    }
    ani_string programPath = nullptr;
    if (Check(env->String_NewUTF8(programAbc, std::strlen(programAbc), &programPath),
              "cannot create program path string")) {
        return nullptr;
    }
    ani_array paths = nullptr;
    if (Check(env->Array_New(1, programPath, &paths), "cannot create paths array")) {
        return nullptr;
    }
    ani_ref undefined = nullptr;
    if (Check(env->GetUndefined(&undefined), "cannot get undefined")) {
        return nullptr;
    }
    ani_method linkerCtor = nullptr;
    ani_status ctorStatus =
        env->Class_FindMethod(linkerClass, "<ctor>", "C{std.core.RuntimeLinker}C{std.core.Array}:", &linkerCtor);
    if (Check(ctorStatus, "cannot find AbcRuntimeLinker ctor")) {
        return nullptr;
    }
    ani_object linker = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (Check(env->Object_New(linkerClass, linkerCtor, &linker, undefined, paths), "cannot create AbcRuntimeLinker")) {
        return nullptr;
    }
    return linker;
}

// Loads the bridge class through the linker and instantiates it. The linker's loadClass
// returns a Class OBJECT, so the instance is created through std.core.Class.createInstance.
ani_object LoadBridgeInstance(ani_env *env, ani_object linker)
{
    ani_type linkerType = nullptr;
    if (Check(env->Object_GetType(linker, &linkerType), "cannot get linker type")) {
        return nullptr;
    }

    // std.core.Boolean TRUE, the `init` argument of loadClass
    ani_class booleanClass = nullptr;
    if (Check(env->FindClass("std.core.Boolean", &booleanClass), "cannot find std.core.Boolean")) {
        return nullptr;
    }
    ani_method booleanCtor = nullptr;
    if (Check(env->Class_FindMethod(booleanClass, "<ctor>", "z:", &booleanCtor), "cannot find Boolean ctor")) {
        return nullptr;
    }
    ani_object booleanTrue = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (Check(env->Object_New(booleanClass, booleanCtor, &booleanTrue, ANI_TRUE), "cannot create Boolean TRUE")) {
        return nullptr;
    }

    ani_method loadClass = nullptr;
    if (Check(env->Class_FindMethod(static_cast<ani_class>(linkerType), "loadClass",
                                    "C{std.core.String}C{std.core.Boolean}:C{std.core.Class}", &loadClass),
              "cannot find loadClass")) {
        return nullptr;
    }

    ani_string className = nullptr;
    if (Check(env->String_NewUTF8("t.HotReloadBridge", std::strlen("t.HotReloadBridge"), &className),
              "cannot create class name string")) {
        return nullptr;
    }

    ani_ref classObject = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (Check(env->Object_CallMethod_Ref(linker, loadClass, &classObject, className, booleanTrue),
              "cannot load t.HotReloadBridge")) {
        return nullptr;
    }

    ani_class classClass = nullptr;
    if (Check(env->FindClass("std.core.Class", &classClass), "cannot find std.core.Class")) {
        return nullptr;
    }
    ani_method createInstance = nullptr;
    if (Check(env->Class_FindMethod(classClass, "createInstance", ":C{std.core.Object}", &createInstance),
              "cannot find createInstance")) {
        return nullptr;
    }

    ani_ref instance = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (Check(env->Object_CallMethod_Ref(static_cast<ani_object>(classObject), createInstance, &instance),
              "cannot create bridge instance")) {
        return nullptr;
    }
    return static_cast<ani_object>(instance);
}

// Boot-context mode: the program was added to the boot panda files, so the bridge class is
// found and instantiated directly, without an application linker.
ani_object LoadBootBridgeInstance(ani_env *env)
{
    ani_class bridgeClass = nullptr;
    if (Check(env->FindClass("t.HotReloadBridge", &bridgeClass), "cannot find t.HotReloadBridge")) {
        return nullptr;
    }
    ani_method bridgeCtor = nullptr;
    if (Check(env->Class_FindMethod(bridgeClass, "<ctor>", ":", &bridgeCtor), "cannot find bridge ctor")) {
        return nullptr;
    }
    ani_object bridge = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (Check(env->Object_New(bridgeClass, bridgeCtor, &bridge), "cannot create bridge instance")) {
        return nullptr;
    }
    return bridge;
}

// Usage: <driver> <etsstdlib.abc> <program.abc> <a0> <a1> [a2]
constexpr int STDLIB_ARG = 1;
constexpr int PROGRAM_ARG = 2;
constexpr int A0_ARG = 3;
constexpr int A1_ARG = 4;
constexpr int A2_ARG = 5;
constexpr int MIN_ARGC = A2_ARG;  // a2 is optional
constexpr int EXIT_USAGE = 2;
constexpr ani_size LOCAL_SCOPE_CAPACITY = 64U;

// Boot-context mode: the program is added to the boot panda files (the boot_ctx case).
bool IsBootAppendMode()
{
    const char *bootAppend = std::getenv("HR_BOOT_APPEND");
    return bootAppend != nullptr && std::strcmp(bootAppend, "1") == 0;
}

// The bound native calls the protected entry itself instead of the bare-path transaction; used
// under the bridge's managed frame to pin down the caller-authorization check.
bool IsViaEntryMode()
{
    const char *viaEntry = std::getenv("HR_VIA_ENTRY");
    return viaEntry != nullptr && std::strcmp(viaEntry, "1") == 0;
}

// Builds the VM with the smoke-run options; the worker count comes from HR_WORKERS.
bool CreateSmokeVm(ani_vm **vm, ani_env **env, char **argv)
{
    std::string boot = std::string("--ext:boot-panda-files=") + ArgAt(argv, STDLIB_ARG);
    if (IsBootAppendMode()) {
        boot += std::string(":") + ArgAt(argv, PROGRAM_ARG);
    }
    std::string noJit = "--ext:compiler-enable-jit=false";
    std::string logCmp = "--ext:log-components=hotreload";
    std::string logLvl = "--ext:log-level=info";
    std::string workers;
    const char *w = std::getenv("HR_WORKERS");
    if (w != nullptr) {
        workers = std::string("--ext:coroutine-workers-count=") + w;
    }

    std::vector<ani_option> opts = {
        ani_option {boot.c_str(), nullptr},
        ani_option {noJit.c_str(), nullptr},
        ani_option {logCmp.c_str(), nullptr},
        ani_option {logLvl.c_str(), nullptr},
    };
    if (!workers.empty()) {
        opts.push_back(ani_option {workers.c_str(), nullptr});
    }
    ani_options options = {static_cast<ani_size>(opts.size()), opts.data()};
    if (ANI_CreateVM(&options, ANI_VERSION_1, vm) != ANI_OK) {
        std::cerr << "FAIL: cannot create VM\n";
        return false;
    }
    if ((*vm)->GetEnv(ANI_VERSION_1, env) != ANI_OK) {
        std::cerr << "FAIL: cannot get env\n";
        (void)(*vm)->DestroyVM();
        return false;
    }
    if ((*env)->CreateLocalScope(LOCAL_SCOPE_CAPACITY) != ANI_OK) {
        std::cerr << "FAIL: cannot create local scope\n";
        return false;
    }
    return true;
}

// Loads the bridge for the run: from the boot files (boot-context mode), or through the
// module's own linker.
ani_object PrepareBridge(ani_env *env, char **argv)
{
    if (IsBootAppendMode()) {
        return LoadBootBridgeInstance(env);
    }
    ani_object linker = CreateLinker(env, ArgAt(argv, PROGRAM_ARG));
    if (linker == nullptr) {
        return nullptr;
    }
    g_entryLinker = linker;
    return LoadBridgeInstance(env, linker);
}

// Binds the static native declaration and calls `run` with the three string arguments.
bool CallRun(ani_env *env, ani_object bridge, int argc, char **argv, ani_int *rc)
{
    ani_type bridgeType = nullptr;
    if (Check(env->Object_GetType(bridge, &bridgeType), "cannot get bridge type")) {
        return false;
    }
    std::array<ani_native_function, 1U> methods = {
        ani_native_function {"hotReloadNative", "C{std.core.String}C{std.core.String}:i",
                             reinterpret_cast<void *>(IsViaEntryMode() ? NativeHotReloadViaEntry : NativeHotReload)}};
    if (Check(env->Class_BindStaticNativeMethods(static_cast<ani_class>(bridgeType), methods.data(), methods.size()),
              "cannot bind hotReloadNative")) {
        return false;
    }

    ani_string a0 = nullptr;
    ani_string a1 = nullptr;
    ani_string a2 = nullptr;
    if (Check(env->String_NewUTF8(ArgAt(argv, A0_ARG), std::strlen(ArgAt(argv, A0_ARG)), &a0), "cannot create arg0")) {
        return false;
    }
    if (Check(env->String_NewUTF8(ArgAt(argv, A1_ARG), std::strlen(ArgAt(argv, A1_ARG)), &a1), "cannot create arg1")) {
        return false;
    }
    // The third argument is optional; the variants that do not use it get an empty string.
    if (argc > A2_ARG &&
        Check(env->String_NewUTF8(ArgAt(argv, A2_ARG), std::strlen(ArgAt(argv, A2_ARG)), &a2), "cannot create arg2")) {
        return false;
    }
    if (a2 == nullptr && Check(env->String_NewUTF8("", 0, &a2), "cannot create empty arg2")) {
        return false;
    }

    ani_int result = -1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ani_status st = env->Object_CallMethodByName_Int(
        bridge, "run", "C{std.core.String}C{std.core.String}C{std.core.String}:i", &result, a0, a1, a2);
    if (Check(st, "cannot call run")) {
        return false;
    }
    *rc = result;
    return true;
}

}  // namespace

int main(int argc, char **argv)
{
    if (argc < MIN_ARGC) {
        std::cerr << "usage: " << ArgAt(argv, 0) << " <etsstdlib.abc> <program.abc> <a0> <a1> [a2]\n";
        return EXIT_USAGE;
    }

    ani_vm *vm = nullptr;
    ani_env *env = nullptr;
    if (!CreateSmokeVm(&vm, &env, argv)) {
        return 1;
    }

    ani_object bridge = PrepareBridge(env, argv);
    if (bridge == nullptr) {
        return 1;
    }

    ani_int rc = -1;
    if (!CallRun(env, bridge, argc, argv, &rc)) {
        return 1;
    }

    if (env->DestroyLocalScope() != ANI_OK) {
        return 1;
    }
    if (vm->DestroyVM() != ANI_OK) {
        std::cerr << "FAIL: cannot destroy VM\n";
        return 1;
    }
    return rc == 0 ? 0 : 1;
}
