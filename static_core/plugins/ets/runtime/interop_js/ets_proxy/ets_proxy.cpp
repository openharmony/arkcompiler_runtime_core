/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include <string>

#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_set.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_error.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_record.h"

namespace ark::ets::interop::js::ets_proxy {

napi_value GetETSFunction(napi_env env, std::string_view packageName, std::string_view methodName)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    INTEROP_CODE_SCOPE_JS_TO_ETS(executionCtx);

    std::ostringstream classDescriptorBuilder;
    classDescriptorBuilder << "L" << packageName << (packageName.empty() ? "ETSGLOBAL;" : "/ETSGLOBAL;");
    std::string classDescriptor = classDescriptorBuilder.str();

    if (IsEtsGlobalClassName(packageName.data())) {
        // Old-style value for legacy code
        // NOTE remove this check after all tests fixed
        classDescriptor = packageName;
    }

    napi_value jsClass = GetETSClass(env, classDescriptor);
    ASSERT(GetValueType(env, jsClass) == napi_function);

    napi_value jsMethod;
    const napi_status resolveStatus = napi_get_named_property(env, jsClass, methodName.data(), &jsMethod);
    if (UNLIKELY(napi_ok != resolveStatus || GetValueType(env, jsMethod) != napi_function)) {
        InteropCtx::ThrowJSError(env, INTEROP_METHOD_NOT_FOUND,
                                 "GetETSFunction: class " + std::string(classDescriptor) + " doesn't contain " +
                                     std::string(methodName) + " method");
        return nullptr;
    }
    NAPI_CHECK_FATAL(napi_object_seal(env, jsMethod));

    return jsMethod;
}

napi_value GetETSClassImpl(napi_env env, std::string_view classDescriptor)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);

    EtsClass *etsKlass =
        executionCtx->GetPandaVM()->GetClassLinker()->GetClass(classDescriptor.data(), true, ctx->LinkerCtx());
    if (UNLIKELY(etsKlass == nullptr)) {
        ctx->ForwardEtsException(executionCtx);
        return nullptr;
    }

    // When a module class is imported by dynamic runtime,
    // initialize the class to ensure top-level statements are executed.
    // ** ClassLinker::InitializeClass() is a run-once method. **
    if (UNLIKELY(etsKlass->IsModule())) {
        executionCtx->GetPandaVM()->GetClassLinker()->InitializeClass(executionCtx, etsKlass);
    }

    EtsClassWrapper *etsClassWrapper = EtsClassWrapper::Get(ctx, etsKlass);
    if (UNLIKELY(etsClassWrapper == nullptr)) {
        ctx->ForwardEtsException(executionCtx);
        return nullptr;
    }

    return etsClassWrapper->GetJsCtor(env);
}

napi_value CreateEtsRecordInstance(napi_env env)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);

    INTEROP_CODE_SCOPE_JS_TO_ETS(executionCtx);
    ScopedManagedCodeThread managedScope(executionCtx->GetMT());

    EtsClass *etsClass = PlatformTypes()->coreRecord;
    EtsObject *etsInstance = etsClass->CreateInstance();
    if (UNLIKELY(etsInstance == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        InteropCtx::ThrowJSError(env, INTEROP_OBJECT_CREATION_FAILED, "Failed to create ETS record instance");
        return nullptr;
    }

    JSRefConvertRecord converter(ctx);
    napi_value jsWrapper = converter.WrapImpl(ctx, etsInstance);

    return jsWrapper;
}

napi_value GetETSInstance(napi_env env, std::string_view classDescriptor)
{
    if (classDescriptor == PlatformTypes()->coreRecord->GetDescriptor()) {
        return CreateEtsRecordInstance(env);
    }

    InteropCtx::ThrowJSError(env, INTEROP_UNSUPPORTED_TYPE_CONVERSION,
                             "Unsupported ETS instance type: " + std::string(classDescriptor));
    return nullptr;
}

napi_value GetETSClass(napi_env env, std::string_view classDescriptor)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    INTEROP_CODE_SCOPE_JS_TO_ETS(executionCtx);
    ScopedManagedCodeThread managedScope(executionCtx->GetMT());

    return GetETSClassImpl(env, classDescriptor);
}

static void FillExportedClasses(napi_env env, EtsClass *globalClass, napi_value moduleObject)
{
    std::vector<std::string> exportedClasses;
    if (!GetExportedClassDescriptorsFromModule(globalClass, exportedClasses)) {
        return;
    }

    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);
    ClassLinkerContext *ctxForLoad = globalClass->GetLoadContext();
    auto *classLinker = executionCtx->GetPandaVM()->GetClassLinker();

    for (const std::string &clsDesc : exportedClasses) {
        EtsClass *exportedKlass = classLinker->GetClass(clsDesc.c_str(), true, ctxForLoad);
        if (exportedKlass == nullptr) {
            LOG(WARNING, INTEROP) << "Failed to resolve exported class: " << clsDesc;
            continue;
        }

        EtsClassWrapper *wrapper = EtsClassWrapper::Get(ctx, exportedKlass);
        if (wrapper == nullptr) {
            LOG(WARNING, INTEROP) << "Failed to get wrapper for exported class: " << clsDesc;
            continue;
        }

        napi_value clsProxy = wrapper->GetJsCtor(env);

        std::string simpleName = clsDesc.substr(clsDesc.find_last_of('/') + 1);
        if (!simpleName.empty() && simpleName.back() == ';') {
            simpleName.pop_back();
        }

        NAPI_CHECK_FATAL(napi_set_named_property(env, moduleObject, simpleName.c_str(), clsProxy));
    }
}

static void CopyNamedProperties(napi_env env, napi_value from, napi_value to)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
    napi_value keys;
    NAPI_CHECK_FATAL(napi_get_property_names(env, from, &keys));
    uint32_t len;
    NAPI_CHECK_FATAL(napi_get_array_length(env, keys, &len));
    for (uint32_t i = 0; i < len; i++) {
        napi_value key;
        NAPI_CHECK_FATAL(napi_get_element(env, keys, i, &key));
        napi_value val;
        NAPI_CHECK_FATAL(napi_get_property(env, from, key, &val));
        NAPI_CHECK_FATAL(napi_set_property(env, to, key, val));
    }
}

static void ProcessModuleRecursive(napi_env env, EtsClass *globalClass, napi_value moduleObject,
                                   std::unordered_set<EtsClass *> &visitedModules)
{
    if (visitedModules.count(globalClass) > 0) {
        return;
    }
    visitedModules.insert(globalClass);

    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);

    napi_value globalProxy = EtsClassWrapper::Get(ctx, globalClass)->GetJsCtor(env);
    FillExportedClasses(env, globalClass, moduleObject);
    CopyNamedProperties(env, globalProxy, moduleObject);

    std::vector<std::string> exportedClasses;
    if (!GetExportedClassDescriptorsFromModule(globalClass, exportedClasses)) {
        return;
    }

    auto *classLinker = executionCtx->GetPandaVM()->GetClassLinker();
    auto *ctxForLoad = globalClass->GetLoadContext();

    for (const auto &clsDesc : exportedClasses) {
        EtsClass *exportedKlass = classLinker->GetClass(clsDesc.c_str(), true, ctxForLoad);
        if (exportedKlass == nullptr) {
            continue;
        }

        if (exportedKlass->IsModule()) {
            ProcessModuleRecursive(env, exportedKlass, moduleObject, visitedModules);
        }
    }
}

napi_value GetETSModule(napi_env env, const std::string &moduleName)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    if (executionCtx == nullptr) {
        return InteropCtx::CreateJSTypeError(env, std::to_string(INTEROP_STATIC_CONTEXT_NOT_LOADED),
                                             "Static context not loaded");
    }
    ScopedManagedCodeThread managedScope(executionCtx->GetMT());
    InteropCtx *ctx = InteropCtx::Current(executionCtx);

    std::string descriptor = "L" + moduleName + "/ETSGLOBAL;";
    EtsClass *globalClass =
        executionCtx->GetPandaVM()->GetClassLinker()->GetClass(descriptor.c_str(), true, ctx->LinkerCtx());
    if (globalClass == nullptr) {
        ctx->ForwardEtsException(executionCtx);
        return nullptr;
    }

    napi_value moduleObject;
    NAPI_CHECK_FATAL(napi_create_object(env, &moduleObject));

    std::unordered_set<EtsClass *> visitedModules;
    ProcessModuleRecursive(env, globalClass, moduleObject, visitedModules);
    return moduleObject;
}

}  // namespace ark::ets::interop::js::ets_proxy
