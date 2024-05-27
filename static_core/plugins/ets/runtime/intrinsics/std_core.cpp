/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "runtime/runtime_helpers.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_atomic_flag.h"
#include "runtime/include/thread_scopes.h"

#include "runtime/include/stack_walker.h"
#include "runtime/include/thread.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"

namespace ark::ets::intrinsics {

extern "C" EtsArray *StdCoreStackTraceLines()
{
    auto runtime = Runtime::GetCurrent();
    auto linker = runtime->GetClassLinker();
    auto ext = linker->GetExtension(panda_file::SourceLang::ETS);
    auto klass = ext->GetClassRoot(ClassRoot::ARRAY_STRING);

    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::ETS);

    auto thread = ManagedThread::GetCurrent();
    auto walker = StackWalker::Create(thread);

    std::vector<std::string> lines;

    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        Method *method = stack.GetMethod();
        auto *source = method->GetClassSourceFile().data;
        auto lineNum = method->GetLineNumFromBytecodeOffset(stack.GetBytecodePc());

        if (source == nullptr) {
            source = utf::CStringAsMutf8("<unknown>");
        }

        std::stringstream ss;
        ss << method->GetClass()->GetName() << "." << method->GetName().data << " at " << source << ":" << lineNum;
        lines.push_back(ss.str());
    }

    auto coroutine = Coroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    auto *arr = ark::coretypes::Array::Create(klass, lines.size());

    VMHandle<coretypes::Array> arrayHandle(coroutine, arr);

    for (ark::ArraySizeT i = 0; i < (ark::ArraySizeT)lines.size(); i++) {
        auto *str = coretypes::String::CreateFromMUtf8(utf::CStringAsMutf8(lines[i].data()), lines[i].length(), ctx,
                                                       thread->GetVM());
        arrayHandle.GetPtr()->Set(i, str);
    }

    return reinterpret_cast<EtsArray *>(arrayHandle.GetPtr());
}

extern "C" void StdCorePrintStackTrace()
{
    ark::PrintStackTrace();
}

static PandaString ResolveLibraryName(const PandaString &name)
{
#ifdef PANDA_TARGET_UNIX
    return PandaString("lib") + name + ".so";
#else
    // Unsupported on windows platform
    UNREACHABLE();
#endif  // PANDA_TARGET_UNIX
}

extern "C" void LoadLibrary(ark::ets::EtsString *name)
{
    ASSERT(name->AsObject()->IsStringClass());

    if (name->IsUtf16()) {
        LOG(FATAL, RUNTIME) << "UTF-16 native library pathes are not supported";
        return;
    }

    auto coroutine = EtsCoroutine::GetCurrent();
    auto nameStr = name->GetMutf8();
    if (nameStr.empty()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::FILE_NOT_FOUND_EXCEPTION,
                          "The native library path is empty");
        return;
    }

    ScopedNativeCodeThread snct(coroutine);

    auto env = coroutine->GetEtsNapiEnv();
    if (!coroutine->GetPandaVM()->LoadNativeLibrary(env, ResolveLibraryName(nameStr))) {
        ScopedManagedCodeThread smct(coroutine);

        PandaStringStream ss;
        ss << "Cannot load native library " << nameStr;

        ThrowEtsException(coroutine, panda_file_items::class_descriptors::EXCEPTION_IN_INITIALIZER_ERROR, ss.str());
    }
}

extern "C" void StdSystemScheduleCoroutine()
{
    auto *cm = static_cast<CoroutineManager *>(Coroutine::GetCurrent()->GetVM()->GetThreadManager());
    cm->Schedule();
}

extern "C" void StdSystemSetCoroutineSchedulingPolicy(int32_t policy)
{
    constexpr auto POLICIES_MAPPING =
        std::array {CoroutineSchedulingPolicy::DEFAULT, CoroutineSchedulingPolicy::NON_MAIN_WORKER};
    ASSERT((policy >= 0) && (static_cast<size_t>(policy) < POLICIES_MAPPING.size()));
    CoroutineSchedulingPolicy newPolicy = POLICIES_MAPPING[policy];

    auto *cm = static_cast<CoroutineManager *>(Coroutine::GetCurrent()->GetVM()->GetThreadManager());
    cm->SetSchedulingPolicy(newPolicy);
}

extern "C" int32_t StdSystemGetCoroutineId()
{
    return EtsCoroutine::GetCurrent()->GetCoroutineId();
}

extern "C" void StdSystemAtomicFlagSet(EtsAtomicFlag *instance, EtsBoolean v)
{
    instance->SetValue(v);
}

extern "C" EtsBoolean StdSystemAtomicFlagGet(EtsAtomicFlag *instance)
{
    return instance->GetValue();
}

}  // namespace ark::ets::intrinsics
