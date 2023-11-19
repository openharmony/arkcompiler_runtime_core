/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H

#include "runtime/coroutines/coroutine_context.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_manager.h"

namespace panda::ets {
class PandaEtsVM;

/// @brief The eTS coroutine. It is aware of the native interface and reference storage existance.
class EtsCoroutine : public Coroutine {
public:
    NO_COPY_SEMANTIC(EtsCoroutine);
    NO_MOVE_SEMANTIC(EtsCoroutine);

    /**
     * @brief EtsCoroutine factory: the preferrable way to create a coroutine. See CoroutineManager::CoroutineFactory
     * for details.
     *
     * Since C++ requires function type to exactly match the formal parameter type, we have to make this factory a
     * template. The sole purpose for this is to be able to return both Coroutine* and EtsCoroutine*
     */
    template <class T>
    static T *Create(Runtime *runtime, PandaVM *vm, PandaString name, CoroutineContext *context,
                     std::optional<EntrypointInfo> &&ep_info = std::nullopt)
    {
        mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
        auto co = allocator->New<EtsCoroutine>(os::thread::GetCurrentThreadId(), allocator, vm, std::move(name),
                                               context, std::move(ep_info));
        co->Initialize();
        return co;
    }
    ~EtsCoroutine() override = default;

    static EtsCoroutine *CastFromThread(Thread *thread)
    {
        ASSERT(thread != nullptr);
        return static_cast<EtsCoroutine *>(thread);
    }

    static EtsCoroutine *GetCurrent()
    {
        Coroutine *co = Coroutine::GetCurrent();
        if ((co != nullptr) && co->GetThreadLang() == panda::panda_file::SourceLang::ETS) {
            return CastFromThread(co);
        }
        return nullptr;
    }

    void SetPromiseClass(void *promise_class)
    {
        promise_class_ptr_ = promise_class;
    }

    static constexpr uint32_t GetTlsPromiseClassPointerOffset()
    {
        return MEMBER_OFFSET(EtsCoroutine, promise_class_ptr_);
    }

    ALWAYS_INLINE ObjectHeader *GetUndefinedObject() const
    {
        return undefined_obj_;
    }

    // For mainthread initializer
    void SetUndefinedObject(ObjectHeader *obj)
    {
        undefined_obj_ = obj;
    }

    static constexpr uint32_t GetTlsUndefinedObjectOffset()
    {
        return MEMBER_OFFSET(EtsCoroutine, undefined_obj_);
    }

    PANDA_PUBLIC_API PandaEtsVM *GetPandaVM() const;
    PANDA_PUBLIC_API CoroutineManager *GetCoroutineManager() const;

    PandaEtsNapiEnv *GetEtsNapiEnv() const
    {
        return ets_napi_env_.get();
    }

    void Initialize() override;
    void RequestCompletion(Value return_value) override;
    void FreeInternalMemory() override;

protected:
    // we would like everyone to use the factory to create a EtsCoroutine
    explicit EtsCoroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, PandaString name,
                          CoroutineContext *context, std::optional<EntrypointInfo> &&ep_info);

private:
    panda_file::Type GetReturnType();
    EtsObject *GetReturnValueAsObject(panda_file::Type return_type, Value return_value);

    std::unique_ptr<PandaEtsNapiEnv> ets_napi_env_;
    void *promise_class_ptr_ {nullptr};

    ObjectHeader *undefined_obj_ {};

    // Allocator calls our protected ctor
    friend class mem::Allocator;
};
}  // namespace panda::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H
