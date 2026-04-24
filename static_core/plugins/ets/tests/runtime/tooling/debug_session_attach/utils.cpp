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

#include <algorithm>
#include <array>
#include <iostream>

#include <ani.h>
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/utf.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/runtime.h"
#include "runtime/tooling/tools.h"

class BytecodeEventListener final : public ark::RuntimeListener {
public:
    struct Event final {
        ark::ManagedThread *thread;
        ark::Method *method;
        uint32_t bcOffset;
    };

    void BytecodePcChanged(ark::ManagedThread *thread, ark::Method *method, uint32_t bcOffset) override
    {
        ark::os::memory::LockHolder lock(mutex_);
        events_.push_back({thread, method, bcOffset});
    }

    auto GetEvents() const
    {
        ark::os::memory::LockHolder lock(mutex_);
        return events_;
    }

private:
    mutable ark::os::memory::Mutex mutex_ {};
    std::vector<Event> events_ {};
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
BytecodeEventListener g_bytecodeEventListener;

static ani_int AttachDebugSession([[maybe_unused]] ani_env *env)
{
    auto *runtime = ark::Runtime::GetCurrent();
    auto status = runtime->GetTools().AttachDebugSession();
    runtime->GetNotificationManager()->AddListener(&g_bytecodeEventListener,
                                                   ark::RuntimeNotificationManager::BYTECODE_PC_CHANGED);
    return static_cast<ani_int>(status);
}

static ani_int CountEventsForMethod(ani_env *env, ani_string methodName)
{
    ani_size sz = 0;
    auto s = env->String_GetUTF8Size(methodName, &sz);
    if (s != ANI_OK) {
        return -1;
    }
    std::string str(sz + 1, '\0');
    ani_size res = 0;
    s = env->String_GetUTF8(methodName, str.data(), str.size(), &res);
    if (s != ANI_OK) {
        return -1;
    }
    str.resize(res);

    auto events = g_bytecodeEventListener.GetEvents();
    ani_int count = std::count_if(events.begin(), events.end(),
                                  [targetName = std::string_view(str)](const BytecodeEventListener::Event &event) {
                                      std::string_view name = ark::utf::Mutf8AsCString(event.method->GetName().data);
                                      return name == targetName;
                                  });
    return count;
}

static ani_boolean IsAotEnabled([[maybe_unused]] ani_env *env)
{
    return static_cast<ani_boolean>(ark::Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles());
}

static constexpr const char *NS_NAME = "utils.attach";

static ani_int AttachViaAni(ani_env *env)
{
    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(NS_NAME, &ns)) {
        std::cerr << "Not found '" << NS_NAME << "'" << std::endl;
        return -1;
    }
    ani_function fn {};
    if (ANI_OK != env->Namespace_FindFunction(ns, "attachImpl", nullptr, &fn)) {
        std::cerr << "Not found 'attachImpl' in '" << NS_NAME << "'" << std::endl;
        return -1;
    }
    ani_int res = -1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (ANI_OK != env->Function_Call_Int(fn, &res)) {
        std::cerr << "Failed to invoke 'attachImpl'" << std::endl;
        return -1;
    }
    return res;
}

static void ThrowErrorViaAni(ani_env *env)
{
    ani_ref undef {};
    if (ANI_OK != env->GetUndefined(&undef)) {
        std::cerr << "Failed to get undefined" << std::endl;
        return;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass("std.core.Error", &cls)) {
        std::cerr << "Not found 'std.core.Error'" << std::endl;
        return;
    }

    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{std.core.ErrorOptions}:", &ctor)) {
        std::cerr << "Not found std.core.Error constructor" << std::endl;
        return;
    }

    ani_object errObj {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (ANI_OK != env->Object_New(cls, ctor, &errObj, undef, undef)) {
        std::cerr << "Failed to create std.core.Error" << std::endl;
        return;
    }

    if (ANI_OK != env->ThrowError(static_cast<ani_error>(errObj))) {
        std::cerr << "Failed to throw std.core.Error" << std::endl;
    }
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(NS_NAME, &ns)) {
        std::cerr << "Not found '" << NS_NAME << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array fns = {
        ani_native_function {"attachDebugSession", ":i", reinterpret_cast<void *>(AttachDebugSession)},
        ani_native_function {"countEventsForMethod", "C{std.core.String}:i",
                             reinterpret_cast<void *>(CountEventsForMethod)},
        ani_native_function {"isAotEnabled", ":z", reinterpret_cast<void *>(IsAotEnabled)},
        ani_native_function {"attachViaAni", ":i", reinterpret_cast<void *>(AttachViaAni)},
        ani_native_function {"throwErrorViaAni", ":", reinterpret_cast<void *>(ThrowErrorViaAni)},
    };

    ani_status status = env->Namespace_BindNativeFunctions(ns, fns.data(), fns.size());
    if (status != ANI_OK) {
        std::cerr << "Cannot bind native methods to '" << NS_NAME << "'"
                  << ":" << status << std::endl;
        return ANI_ERROR;
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}
