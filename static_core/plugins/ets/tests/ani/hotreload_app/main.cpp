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
 * VerifyANI + hot reload, two scenarios over one target/patch pair.
 *
 * Scenario 1 -- cached ANI handles across a reload of their declaring class.
 *
 * The flow mirrors the application framework: the target abc is loaded through an
 * AbcRuntimeLinker (an APPLICATION context, not the boot context of this binary), an
 * instance and its method/field handles are cached, the class is hot reloaded through
 * the linker's `hotReload` entry, and the cached handles are reused on the same
 * instance.
 *
 * Under `--verify:ani` the reuse used to abort at the ownership checks: a cached handle
 * names an entity of the retired generation, and VerifyMethod/VerifyReadFieldImpl
 * rejected it before the release-layer converters could translate it. The test passes
 * only when the verified entity resolution redirects the handle
 * (ani/verify/verify_ani_resolve.h).
 *
 * Scenario 2 -- the linker entry point is per-receiver.
 *
 * Two linkers open the SAME target path, so two independent contexts each publish their
 * own `t.Greeter`. A reload driven through the first receiver must swap only the first
 * context (the second instance keeps calling the old body), and a second reload through
 * the other receiver must then update its own context the same way. The entry is
 * protected -- native code calling it by name is exactly how the framework reaches it,
 * and the reason this scenario lives here and not in the .ets smoke suite.
 *
 * Environment:
 *   ARK_ETS_STDLIB_PATH        etsstdlib.abc
 *   ANI_HOTRELOAD_TARGET_ABC   the abc the linker loads (target/t.ets compiled)
 *   ANI_HOTRELOAD_PATCH_ABC    the patch abc (patch/t.ets compiled)
 */

#include <ani.h>

#include <array>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {

constexpr const char *TARGET_CLASS_NAME = "t.Greeter";
// Generous bound: strings, arrays, objects and wrappers created over the whole run, which
// spans two scenarios and three linkers
constexpr ani_size LOCAL_SCOPE_CAPACITY = 128;
// Non-default on purpose: a surviving value proves state preservation across the swap
constexpr ani_int FIELD_TEST_VALUE = 41;

bool CheckStatus(ani_status status, const char *message)
{
    if (status != ANI_OK) {
        std::cerr << "FAIL: " << message << ", status=" << static_cast<int>(status) << "\n";
        return true;
    }
    return false;
}

bool CheckCondition(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
    }
    return !condition;
}

std::string ToCString(ani_env *env, ani_string str)
{
    ani_size size = 0;
    if (env->String_GetUTF8Size(str, &size) != ANI_OK) {
        return "<error>";
    }
    // One extra byte for the terminator: the API requires utfBufferSize > substrSize.
    std::string out(size + 1U, '\0');
    ani_size real = 0;
    if (env->String_GetUTF8SubString(str, 0, size, out.data(), size + 1U, &real) != ANI_OK) {
        return "<error>";
    }
    return out.substr(0, real);
}

// Creates an AbcRuntimeLinker whose abcFiles hold the target abc, the way the
// application framework builds the linker for a module.
ani_object CreateLinker(ani_env *env)
{
    const char *targetAbc = std::getenv("ANI_HOTRELOAD_TARGET_ABC");
    if (CheckCondition(targetAbc != nullptr, "ANI_HOTRELOAD_TARGET_ABC is not set")) {
        return nullptr;
    }

    ani_class linkerClass = nullptr;
    if (CheckStatus(env->FindClass("std.core.AbcRuntimeLinker", &linkerClass),
                    "cannot find std.core.AbcRuntimeLinker")) {
        return nullptr;
    }

    ani_string targetPath = nullptr;
    if (CheckStatus(env->String_NewUTF8(targetAbc, std::strlen(targetAbc), &targetPath),
                    "cannot create target path string")) {
        return nullptr;
    }

    ani_array paths = nullptr;
    if (CheckStatus(env->Array_New(1, targetPath, &paths), "cannot create paths array")) {
        return nullptr;
    }

    ani_ref undefined = nullptr;
    if (CheckStatus(env->GetUndefined(&undefined), "cannot get undefined")) {
        return nullptr;
    }

    ani_method linkerCtor = nullptr;
    ani_status ctorStatus =
        env->Class_FindMethod(linkerClass, "<ctor>", "C{std.core.RuntimeLinker}C{std.core.Array}:", &linkerCtor);
    if (CheckStatus(ctorStatus, "cannot find AbcRuntimeLinker ctor")) {
        return nullptr;
    }

    ani_object linker = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_New(linkerClass, linkerCtor, &linker, undefined, paths),
                    "cannot create AbcRuntimeLinker")) {
        return nullptr;
    }
    return linker;
}

// Loads the target class through the linker and instantiates it. The linker's loadClass
// returns a Class OBJECT, not an ani_class, so the instance is created through
// std.core.Class.createInstance; the driver derives the real class handle from the
// instance afterwards.
ani_object LoadTargetInstance(ani_env *env, ani_object linker)
{
    ani_type linkerType = nullptr;
    if (CheckStatus(env->Object_GetType(linker, &linkerType), "cannot get linker type")) {
        return nullptr;
    }

    // std.core.Boolean TRUE, the `init` argument of loadClass
    ani_class booleanClass = nullptr;
    if (CheckStatus(env->FindClass("std.core.Boolean", &booleanClass), "cannot find std.core.Boolean")) {
        return nullptr;
    }
    ani_method booleanCtor = nullptr;
    if (CheckStatus(env->Class_FindMethod(booleanClass, "<ctor>", "z:", &booleanCtor), "cannot find Boolean ctor")) {
        return nullptr;
    }
    ani_object booleanTrue = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_New(booleanClass, booleanCtor, &booleanTrue, ANI_TRUE), "cannot create Boolean TRUE")) {
        return nullptr;
    }

    ani_method loadClass = nullptr;
    if (CheckStatus(env->Class_FindMethod(static_cast<ani_class>(linkerType), "loadClass",
                                          "C{std.core.String}C{std.core.Boolean}:C{std.core.Class}", &loadClass),
                    "cannot find loadClass")) {
        return nullptr;
    }

    ani_string className = nullptr;
    if (CheckStatus(env->String_NewUTF8(TARGET_CLASS_NAME, std::strlen(TARGET_CLASS_NAME), &className),
                    "cannot create class name string")) {
        return nullptr;
    }

    ani_ref classObject = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_CallMethod_Ref(linker, loadClass, &classObject, className, booleanTrue),
                    "cannot load target class")) {
        return nullptr;
    }

    ani_class classClass = nullptr;
    if (CheckStatus(env->FindClass("std.core.Class", &classClass), "cannot find std.core.Class")) {
        return nullptr;
    }
    ani_method createInstance = nullptr;
    if (CheckStatus(env->Class_FindMethod(classClass, "createInstance", ":C{std.core.Object}", &createInstance),
                    "cannot find createInstance")) {
        return nullptr;
    }

    ani_ref instance = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_CallMethod_Ref(static_cast<ani_object>(classObject), createInstance, &instance),
                    "cannot create target instance")) {
        return nullptr;
    }
    return static_cast<ani_object>(instance);
}

// Caches the method/field handles under test, derived from the instance's real class.
int CacheHandles(ani_env *env, ani_object object, ani_method *tagMethod, ani_field *nField)
{
    ani_type targetType = nullptr;
    if (CheckStatus(env->Object_GetType(object, &targetType), "cannot get target type")) {
        return 1;
    }
    auto targetClass = static_cast<ani_class>(targetType);

    if (CheckStatus(env->Class_FindMethod(targetClass, "tag", ":C{std.core.String}", tagMethod),
                    "cannot find t.Greeter.tag")) {
        return 1;
    }
    if (CheckStatus(env->Class_FindField(targetClass, "n", nField), "cannot find t.Greeter.n")) {
        return 1;
    }
    return 0;
}

// Before the reload: the cached handles work and observe version 1.
int VerifyBeforeReload(ani_env *env, ani_object object, ani_method tagMethod, ani_field nField)
{
    ani_ref tag = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_CallMethod_Ref(object, tagMethod, &tag), "cannot call tag (v1)")) {
        return 1;
    }
    if (CheckCondition(ToCString(env, static_cast<ani_string>(tag)) == "V1", "tag before reload must be V1")) {
        return 1;
    }
    if (CheckStatus(env->Object_SetField_Int(object, nField, FIELD_TEST_VALUE), "cannot set n (v1)")) {
        return 1;
    }
    ani_int n = 0;
    if (CheckStatus(env->Object_GetField_Int(object, nField, &n), "cannot get n (v1)")) {
        return 1;
    }
    if (CheckCondition(n == FIELD_TEST_VALUE, "n before reload must be the test value")) {
        return 1;
    }
    return 0;
}

// Hot reload through the framework's own entry on the linker.
int ExecuteHotReload(ani_env *env, ani_object linker)
{
    const char *targetAbc = std::getenv("ANI_HOTRELOAD_TARGET_ABC");
    const char *patchAbc = std::getenv("ANI_HOTRELOAD_PATCH_ABC");
    if (CheckCondition(targetAbc != nullptr, "ANI_HOTRELOAD_TARGET_ABC is not set")) {
        return 1;
    }
    if (CheckCondition(patchAbc != nullptr, "ANI_HOTRELOAD_PATCH_ABC is not set")) {
        return 1;
    }
    ani_string targetStr = nullptr;
    ani_string patchStr = nullptr;
    if (CheckStatus(env->String_NewUTF8(targetAbc, std::strlen(targetAbc), &targetStr),
                    "cannot create target string")) {
        return 1;
    }
    if (CheckStatus(env->String_NewUTF8(patchAbc, std::strlen(patchAbc), &patchStr), "cannot create patch string")) {
        return 1;
    }
    ani_int rc = -1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_CallMethodByName_Int(linker, "hotReload", "C{std.core.String}C{std.core.String}:i", &rc,
                                                     targetStr, patchStr),
                    "hotReload call failed")) {
        return 1;
    }
    if (CheckCondition(rc == 0, "hotReload must return 0")) {
        return 1;
    }
    return 0;
}

// After the reload: reuse the CACHED handles on the SAME instance. Under --verify:ani this is
// the step that used to abort with "does not belong to" before the resolution was fixed to
// redirect.
int VerifyAfterReload(ani_env *env, ani_object object, ani_method tagMethod, ani_field nField)
{
    ani_ref tag = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_CallMethod_Ref(object, tagMethod, &tag), "cannot call cached tag after reload")) {
        return 1;
    }
    if (CheckCondition(ToCString(env, static_cast<ani_string>(tag)) == "V2",
                       "cached tag after reload must be V2 (new method body)")) {
        return 1;
    }
    ani_int n = 0;
    if (CheckStatus(env->Object_GetField_Int(object, nField, &n), "cannot read cached n after reload")) {
        return 1;
    }
    if (CheckCondition(n == FIELD_TEST_VALUE, "cached n after reload must keep its value (state preserved)")) {
        return 1;
    }
    return 0;
}

// Calls `tag` on the instance by name and compares the result with the expected literal.
int CheckTag(ani_env *env, ani_object object, const char *expected, const char *context)
{
    ani_ref tag = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (CheckStatus(env->Object_CallMethodByName_Ref(object, "tag", ":C{std.core.String}", &tag), "cannot call tag")) {
        return 1;
    }
    std::string message = "tag must be " + std::string(expected) + " (" + context + ")";
    if (CheckCondition(ToCString(env, static_cast<ani_string>(tag)) == expected, message.c_str())) {
        return 1;
    }
    return 0;
}

// Scenario 2: the linker entry point is per-receiver. Two linkers open the same target path;
// a reload through one must leave the other context on the old generation.
int RunTwoLinkersCase(ani_env *env)
{
    ani_object firstLinker = CreateLinker(env);
    if (firstLinker == nullptr) {
        return 1;
    }
    ani_object secondLinker = CreateLinker(env);
    if (secondLinker == nullptr) {
        return 1;
    }

    ani_object firstObject = LoadTargetInstance(env, firstLinker);
    if (firstObject == nullptr) {
        return 1;
    }
    ani_object secondObject = LoadTargetInstance(env, secondLinker);
    if (secondObject == nullptr) {
        return 1;
    }

    // Distinct contexts must publish distinct classes for the same record descriptor.
    ani_type firstType = nullptr;
    ani_type secondType = nullptr;
    if (CheckStatus(env->Object_GetType(firstObject, &firstType), "cannot get type of first instance")) {
        return 1;
    }
    if (CheckStatus(env->Object_GetType(secondObject, &secondType), "cannot get type of second instance")) {
        return 1;
    }
    if (CheckCondition(firstType != secondType, "the two linkers must publish distinct classes")) {
        return 1;
    }

    // Reload through the FIRST receiver: its context swaps, the other must stay on V1.
    if (ExecuteHotReload(env, firstLinker) != 0) {
        return 1;
    }
    if (CheckTag(env, firstObject, "V2", "first receiver after its reload") != 0) {
        return 1;
    }
    if (CheckTag(env, secondObject, "V1", "second receiver must be untouched") != 0) {
        return 1;
    }

    // Reload through the SECOND receiver: now its own context swaps as well.
    if (ExecuteHotReload(env, secondLinker) != 0) {
        return 1;
    }
    if (CheckTag(env, secondObject, "V2", "second receiver after its reload") != 0) {
        return 1;
    }
    return 0;
}

int RunTest(ani_env *env)
{
    // Under --verify:ani every local reference must be created inside a native scope.
    if (CheckStatus(env->CreateLocalScope(LOCAL_SCOPE_CAPACITY), "cannot create local scope")) {
        return 1;
    }

    ani_object linker = CreateLinker(env);
    if (linker == nullptr) {
        return 1;
    }
    ani_object object = LoadTargetInstance(env, linker);
    if (object == nullptr) {
        return 1;
    }
    ani_method tagMethod = nullptr;
    ani_field nField = nullptr;
    if (CacheHandles(env, object, &tagMethod, &nField) != 0) {
        return 1;
    }
    if (VerifyBeforeReload(env, object, tagMethod, nField) != 0) {
        return 1;
    }
    if (ExecuteHotReload(env, linker) != 0) {
        return 1;
    }
    if (VerifyAfterReload(env, object, tagMethod, nField) != 0) {
        return 1;
    }

    if (CheckStatus(env->DestroyLocalScope(), "cannot destroy local scope")) {
        return 1;
    }

    if (CheckStatus(env->CreateLocalScope(LOCAL_SCOPE_CAPACITY), "cannot create local scope (two linkers)")) {
        return 1;
    }
    if (RunTwoLinkersCase(env) != 0) {
        return 1;
    }
    if (CheckStatus(env->DestroyLocalScope(), "cannot destroy local scope (two linkers)")) {
        return 1;
    }
    return 0;
}

}  // namespace

int main()
{
    const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
    if (stdlib == nullptr) {
        std::cerr << "FAIL: ARK_ETS_STDLIB_PATH is not set\n";
        return 1;
    }

    ani_vm *vm = nullptr;
    ani_env *env = nullptr;

    std::string bootFiles = std::string("--ext:boot-panda-files=") + stdlib;
    std::string verifyAni = "--verify:ani";
    std::string noJit = "--ext:compiler-enable-jit=false";
    std::array<ani_option, 3U> optionsArray = {
        ani_option {bootFiles.c_str(), nullptr},
        ani_option {verifyAni.c_str(), nullptr},
        ani_option {noJit.c_str(), nullptr},
    };
    ani_options options = {optionsArray.size(), optionsArray.data()};

    ani_status status = ANI_CreateVM(&options, ANI_VERSION_1, &vm);
    if (CheckStatus(status, "cannot create VM")) {
        return 1;
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (CheckStatus(status, "cannot get env")) {
        (void)vm->DestroyVM();
        return 1;
    }

    int result = RunTest(env);
    if (vm->DestroyVM() != ANI_OK) {
        std::cerr << "FAIL: cannot destroy VM\n";
        return 1;
    }
    return result;
}
