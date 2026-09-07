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
 * Native driver for the coldreload smoke suite.
 *
 * The test module (tests/coldreload/base/t.ets) exposes `TestRunner.pre` (optionally run
 * BEFORE the reload, the startup-contract violation case) and `TestRunner.observe` (always
 * run AFTER it). This driver creates the module's AbcRuntimeLinker the way the application
 * framework does, drives the cold-reload transaction directly from native code while the
 * module is still in its startup state (no class of it loaded yet, exactly the state the
 * framework calls from), prints the rc lines, and only then loads the test class and calls
 * `observe`. The test's own console output is the observable the smoke script asserts on.
 *
 * Usage: coldreload_app <etsstdlib.abc> <program.abc> <patch> <patch2> <mode>
 * Environment:
 *   COLDRELOAD_AOT_FILE   when set, passed to the VM as --aot-file (the AOT refusal case)
 */

#include <ani.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string>

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/coldreload/ets_coldreload.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_class.h"

namespace {

using ark::ets::EtsCoroutine;

constexpr ani_size LOCAL_SCOPE_CAPACITY = 64U;

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

// Runs the cold-reload transaction on the receiver, the same call the production intrinsic
// makes: while the module is still in its startup state.
int RunColdReload(ani_env *env, ani_object linker, const char *patchPath)
{
    ark::ets::ani::ScopedManagedCodeFix scope(env);
    auto *coro = EtsCoroutine::GetCurrent();
    auto *abcLinker = ark::ets::EtsAbcRuntimeLinker::FromEtsObject(scope.ToInternalType(static_cast<ani_ref>(linker)));
    auto err = ark::ets::coldreload::EtsColdReload().Reload(coro, ark::PandaString(patchPath), abcLinker);
    return static_cast<int>(err);
}

// Loads the test class through the linker and instantiates it. The linker's loadClass
// returns a Class OBJECT, so the instance is created through std.core.Class.createInstance.
ani_object LoadRunnerInstance(ani_env *env, ani_object linker)
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
    if (Check(env->String_NewUTF8("t.TestRunner", std::strlen("t.TestRunner"), &className),
              "cannot create class name string")) {
        return nullptr;
    }

    ani_ref classObject = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (Check(env->Object_CallMethod_Ref(linker, loadClass, &classObject, className, booleanTrue),
              "cannot load t.TestRunner")) {
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
              "cannot create runner instance")) {
        return nullptr;
    }
    return static_cast<ani_object>(instance);
}

// Usage: <driver> <etsstdlib.abc> <program.abc> <patch> <patch2> <mode>
constexpr int STDLIB_ARG = 1;
constexpr int PROGRAM_ARG = 2;
constexpr int PATCH_ARG = 3;
constexpr int PATCH2_ARG = 4;
constexpr int MODE_ARG = 5;
constexpr int MIN_ARGC = MODE_ARG + 1;
constexpr int EXIT_USAGE = 2;
constexpr ani_size BASE_OPT_COUNT = 4U;

// Builds the VM with the smoke-run options; the AOT file comes from COLDRELOAD_AOT_FILE.
bool CreateSmokeVm(ani_vm **vm, ani_env **env, const char *stdlibPath)
{
    std::string boot = std::string("--ext:boot-panda-files=") + stdlibPath;
    std::string noJit = "--ext:compiler-enable-jit=false";
    std::string logCmp = "--ext:log-components=coldreload";
    std::string logLvl = "--ext:log-level=info";
    std::string aotFile;
    const char *aot = std::getenv("COLDRELOAD_AOT_FILE");
    if (aot != nullptr) {
        aotFile = std::string("--ext:aot-files=") + aot;
    }

    std::array<ani_option, 5U> opts = {
        ani_option {boot.c_str(), nullptr},
        ani_option {noJit.c_str(), nullptr},
        ani_option {logCmp.c_str(), nullptr},
        ani_option {logLvl.c_str(), nullptr},
        ani_option {aotFile.empty() ? "" : aotFile.c_str(), nullptr},
    };
    ani_size nopts = aotFile.empty() ? BASE_OPT_COUNT : opts.size();
    ani_options options = {nopts, opts.data()};
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

// The startup-contract violation case: load the test class and run `pre` before the reload.
bool RunPrePhase(ani_env *env, ani_object linker)
{
    ani_object runner = LoadRunnerInstance(env, linker);
    if (runner == nullptr) {
        return false;
    }
    ani_type runnerType = nullptr;
    if (Check(env->Object_GetType(runner, &runnerType), "cannot get runner type")) {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return !Check(env->Class_CallStaticMethodByName_Void(static_cast<ani_class>(runnerType), "pre", ":"),
                  "cannot call pre");
}

// Observation: loads the test class (through the possibly patched order) and calls `observe`.
bool CallObserve(ani_env *env, ani_object linker, ani_int *rc)
{
    ani_object runner = LoadRunnerInstance(env, linker);
    if (runner == nullptr) {
        return false;
    }
    ani_type runnerType = nullptr;
    if (Check(env->Object_GetType(runner, &runnerType), "cannot get runner type")) {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return !Check(env->Class_CallStaticMethodByName_Int(static_cast<ani_class>(runnerType), "observe", ":i", rc),
                  "cannot call observe");
}

}  // namespace

int main(int argc, char **argv)
{
    if (argc < MIN_ARGC) {
        std::cerr << "usage: " << ArgAt(argv, 0) << " <etsstdlib.abc> <program.abc> <patch> <patch2> <mode>\n";
        return EXIT_USAGE;
    }

    ani_vm *vm = nullptr;
    ani_env *env = nullptr;
    if (!CreateSmokeVm(&vm, &env, ArgAt(argv, STDLIB_ARG))) {
        return 1;
    }

    ani_object linker = CreateLinker(env, ArgAt(argv, PROGRAM_ARG));
    if (linker == nullptr) {
        return 1;
    }
    if (std::strcmp(ArgAt(argv, MODE_ARG), "pre") == 0 && !RunPrePhase(env, linker)) {
        return 1;
    }

    // The transaction itself, driven from native code in the module's startup state.
    int rc = RunColdReload(env, linker, ArgAt(argv, PATCH_ARG));
    std::cout << "rc=" << rc << "\n";
    if (std::strcmp(ArgAt(argv, PATCH2_ARG), "") != 0) {
        int rc2 = RunColdReload(env, linker, ArgAt(argv, PATCH2_ARG));
        std::cout << "rc2=" << rc2 << "\n";
    }

    ani_int observeRc = -1;
    if (!CallObserve(env, linker, &observeRc)) {
        return 1;
    }

    if (env->DestroyLocalScope() != ANI_OK) {
        return 1;
    }
    if (vm->DestroyVM() != ANI_OK) {
        std::cerr << "FAIL: cannot destroy VM\n";
        return 1;
    }
    return observeRc == 0 ? 0 : 1;
}
