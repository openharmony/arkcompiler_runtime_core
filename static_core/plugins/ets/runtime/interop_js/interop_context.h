/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_CONTEXT_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_CONTEXT_H_

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/js_job_queue.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/intrinsics/std_js_jsruntime.h"
#include "runtime/include/value.h"

#include <node_api.h>
#include <unordered_map>

namespace panda {

class Class;
class ClassLinkerContext;

namespace mem {
class GlobalObjectStorage;
class Reference;
}  // namespace mem

}  // namespace panda

namespace panda::ets::interop::js {

class JSValue;

// Work-around for String JSValue and node_api
class JSValueStringStorage {
public:
    class CachedEntry {
    public:
        std::string const *Data()
        {
            return data_;
        }

    private:
        friend class JSValue;
        friend class JSValueStringStorage;

        explicit CachedEntry(std::string const *data) : data_(data) {}

        std::string const *data_ {};
    };

    explicit JSValueStringStorage() = default;

    CachedEntry Get(std::string &&str)
    {
        auto [it, inserted] = stringTab_.insert({str, 0});
        it->second++;
        return CachedEntry(&it->first);
    }

    void Release(CachedEntry str)
    {
        auto it = stringTab_.find(*str.Data());
        ASSERT(it != stringTab_.end());
        if (--(it->second) == 0) {
            stringTab_.erase(it);
        }
    }

private:
    std::unordered_map<std::string, uint64_t> stringTab_;
};

using ArgValueBox = std::variant<uint64_t, ObjectHeader **>;

class LocalScopesStorage {
public:
    LocalScopesStorage() = default;

    ~LocalScopesStorage()
    {
        ASSERT(localScopesStorage_.empty());
    }

    NO_COPY_SEMANTIC(LocalScopesStorage);
    NO_MOVE_SEMANTIC(LocalScopesStorage);

    void CreateLocalScope(napi_env env, Frame *frame)
    {
        napi_handle_scope scope;
        [[maybe_unused]] auto status = napi_open_handle_scope(env, &scope);
        ASSERT(status == napi_ok);
        localScopesStorage_.emplace_back(frame, scope);
    }

    void DestroyTopLocalScope(napi_env env, [[maybe_unused]] Frame *currFrame)
    {
        ASSERT(!localScopesStorage_.empty());
        auto &[frame, scope] = localScopesStorage_.back();
        ASSERT(currFrame == frame);
        localScopesStorage_.pop_back();
        [[maybe_unused]] auto status = napi_close_handle_scope(env, scope);
        ASSERT(status == napi_ok);
    }

    void DestroyLocalScopeForTopFrame(napi_env env, Frame *currFrame)
    {
        if (localScopesStorage_.empty()) {
            return;
        }
        auto &[frame, scope] = localScopesStorage_.back();
        while (frame == currFrame) {
            localScopesStorage_.pop_back();
            [[maybe_unused]] auto status = napi_close_handle_scope(env, scope);
            ASSERT(status == napi_ok);
            if (localScopesStorage_.empty()) {
                return;
            }
            std::tie(frame, scope) = localScopesStorage_.back();
        }
    }

private:
    std::vector<std::pair<const Frame *, napi_handle_scope>> localScopesStorage_ {};
};

class InteropCtx {
public:
    static void Init(EtsCoroutine *coro, napi_env env)
    {
        // Initialize InteropCtx in VM ExternalData
        new (InteropCtx::Current(coro)) InteropCtx(coro, env);
    }

    static InteropCtx *Current(PandaEtsVM *etsVm)
    {
        static_assert(sizeof(PandaEtsVM::ExternalData) >= sizeof(InteropCtx));
        static_assert(alignof(PandaEtsVM::ExternalData) >= alignof(InteropCtx));
        return reinterpret_cast<InteropCtx *>(etsVm->GetExternalData());
    }

    static InteropCtx *Current(EtsCoroutine *coro)
    {
        return Current(coro->GetPandaVM());
    }

    static InteropCtx *Current()
    {
        return Current(EtsCoroutine::GetCurrent());
    }

    PandaEtsVM *GetPandaEtsVM()
    {
        return PandaEtsVM::FromExternalData(reinterpret_cast<void *>(this));
    }

    napi_env GetJSEnv() const
    {
        ASSERT(jsEnv_ != nullptr);
        return jsEnv_;
    }

    void SetJSEnv(napi_env env)
    {
        jsEnv_ = env;
    }

    mem::GlobalObjectStorage *Refstor() const
    {
        return refstor_;
    }

    ClassLinkerContext *LinkerCtx() const
    {
        return linkerCtx_;
    }

    JSValueStringStorage *GetStringStor()
    {
        return &jsValueStringStor_;
    }

    LocalScopesStorage *GetLocalScopesStorage()
    {
        return &localScopesStorage_;
    }

    void DestroyLocalScopeForTopFrame(Frame *frame)
    {
        GetLocalScopesStorage()->DestroyLocalScopeForTopFrame(jsEnv_, frame);
    }

    mem::Reference *GetJSValueFinalizationQueue() const
    {
        return jsvalueFqueueRef_;
    }

    Method *GetRegisterFinalizerMethod() const
    {
        return jsvalueFqueueRegister_;
    }

    // NOTE(vpukhov): implement in native code
    [[nodiscard]] bool PushOntoFinalizationQueue(EtsCoroutine *coro, EtsObject *obj, EtsObject *cbarg)
    {
        auto queue = Refstor()->Get(jsvalueFqueueRef_);
        std::array<Value, 4U> args = {Value(queue), Value(obj->GetCoreType()), Value(cbarg->GetCoreType()),
                                      Value(static_cast<ObjectHeader *>(nullptr))};
        jsvalueFqueueRegister_->Invoke(coro, args.data());
        return !coro->HasPendingException();
    }

    template <typename T, size_t OPT_SZ>
    struct TempArgs {
    public:
        explicit TempArgs(size_t sz)
        {
            if (LIKELY(sz <= OPT_SZ)) {
                sp_ = {new (inlArr_.data()) T[sz], sz};
            } else {
                sp_ = {new T[sz], sz};
            }
        }

        ~TempArgs()
        {
            if (UNLIKELY(static_cast<void *>(sp_.data()) != inlArr_.data())) {
                delete[] sp_.data();
            } else {
                static_assert(std::is_trivially_destructible_v<T>);
            }
        }

        NO_COPY_SEMANTIC(TempArgs);
#if defined(__clang__)
        NO_MOVE_SEMANTIC(TempArgs);
#elif defined(__GNUC__)  // RVO bug
        NO_MOVE_OPERATOR(TempArgs);
        TempArgs(TempArgs &&other) : sp_(other.sp_), inlArr_(other.inlArr_)
        {
            if (LIKELY(sp_.size() <= OPT_SZ)) {
                sp_ = Span<T>(reinterpret_cast<T *>(inlArr_.data()), sp_.size());
            }
        }
#endif

        Span<T> &operator*()
        {
            return sp_;
        }
        Span<T> *operator->()
        {
            return &sp_;
        }
        T &operator[](size_t idx)
        {
            return sp_[idx];
        }

    private:
        Span<T> sp_;
        alignas(T) std::array<uint8_t, sizeof(T) * OPT_SZ> inlArr_;
    };

    template <typename T, size_t OPT_SZ = 8U>
    ALWAYS_INLINE static auto GetTempArgs(size_t sz)
    {
        return TempArgs<T, OPT_SZ>(sz);
    }

    // NOTE(vpukhov): implement as flags in IFrame
    struct InteropFrameRecord {
        void *etsFrame {};
        bool toJs {};
    };

    std::vector<InteropFrameRecord> &GetInteropFrames()
    {
        return interopFrames_;
    }

    JSRefConvertCache *GetRefConvertCache()
    {
        return &refconvertCache_;
    }

    Class *GetJSValueClass() const
    {
        return jsValueClass_;
    }

    Class *GetJSErrorClass() const
    {
        return jsErrorClass_;
    }

    Class *GetObjectClass() const
    {
        return objectClass_;
    }

    Class *GetStringClass() const
    {
        return stringClass_;
    }

    Class *GetVoidClass() const
    {
        return voidClass_;
    }

    Class *GetUndefinedClass() const
    {
        return undefinedClass_;
    }

    Class *GetPromiseClass() const
    {
        return promiseClass_;
    }

    Class *GetErrorClass() const
    {
        return errorClass_;
    }

    Class *GetExceptionClass() const
    {
        return exceptionClass_;
    }

    Class *GetTypeClass() const
    {
        return typeClass_;
    }

    Class *GetBoxIntClass() const
    {
        return boxIntClass_;
    }

    Class *GetBoxLongClass() const
    {
        return boxLongClass_;
    }

    Class *GetArrayClass() const
    {
        return arrayClass_;
    }

    Class *GetArrayBufferClass() const
    {
        return arraybufClass_;
    }

    EtsObject *CreateETSCoreJSError(EtsCoroutine *coro, JSValue *jsvalue);

    static void ThrowETSError(EtsCoroutine *coro, napi_value val);
    static void ThrowETSError(EtsCoroutine *coro, const char *msg);
    static void ThrowETSError(EtsCoroutine *coro, const std::string &msg)
    {
        ThrowETSError(coro, msg.c_str());
    }

    static void ThrowJSError(napi_env env, const std::string &msg);
    static void ThrowJSTypeError(napi_env env, const std::string &msg);
    static void ThrowJSValue(napi_env env, napi_value val);

    void ForwardEtsException(EtsCoroutine *coro);
    void ForwardJSException(EtsCoroutine *coro);

    [[nodiscard]] static bool SanityETSExceptionPending()
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = InteropCtx::Current(coro)->GetJSEnv();
        return coro->HasPendingException() && !NapiIsExceptionPending(env);
    }

    [[nodiscard]] static bool SanityJSExceptionPending()
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = InteropCtx::Current(coro)->GetJSEnv();
        return !coro->HasPendingException() && NapiIsExceptionPending(env);
    }

    // Die and print execution stacks
    [[noreturn]] static void Fatal(const char *msg);
    [[noreturn]] static void Fatal(const std::string &msg)
    {
        Fatal(msg.c_str());
    }

    void SetPendingNewInstance(EtsObject *newInstance)
    {
        pendingNewInstance_ = newInstance;
    }

    EtsObject *AcquirePendingNewInstance()
    {
        auto res = pendingNewInstance_;
        pendingNewInstance_ = nullptr;
        return res;
    }

    ets_proxy::EtsMethodWrappersCache *GetEtsMethodWrappersCache()
    {
        return &etsMethodWrappersCache_;
    }

    ets_proxy::EtsClassWrappersCache *GetEtsClassWrappersCache()
    {
        return &etsClassWrappersCache_;
    }

    ets_proxy::SharedReferenceStorage *GetSharedRefStorage()
    {
        return etsProxyRefStorage_.get();
    }

    EtsObject *GetUndefinedObject()
    {
        return EtsObject::FromCoreType(GetPandaEtsVM()->GetUndefinedObject());
    }

private:
    explicit InteropCtx(EtsCoroutine *coro, napi_env env);

    napi_env jsEnv_ {};

    mem::GlobalObjectStorage *refstor_ {};
    ClassLinkerContext *linkerCtx_ {};
    JSValueStringStorage jsValueStringStor_ {};
    LocalScopesStorage localScopesStorage_ {};
    mem::Reference *jsvalueFqueueRef_ {};

    std::vector<InteropFrameRecord> interopFrames_ {};

    JSRefConvertCache refconvertCache_;

    Class *jsRuntimeClass_ {};
    Class *jsValueClass_ {};
    Class *jsErrorClass_ {};
    Class *objectClass_ {};
    Class *stringClass_ {};
    Class *voidClass_ {};
    Class *undefinedClass_ {};
    Class *promiseClass_ {};
    Class *errorClass_ {};
    Class *exceptionClass_ {};
    Class *typeClass_ {};

    Class *boxIntClass_ {};
    Class *boxLongClass_ {};

    Class *arrayClass_ {};
    Class *arraybufClass_ {};

    Method *jsvalueFqueueRegister_ {};

    // ets_proxy data
    EtsObject *pendingNewInstance_ {};
    ets_proxy::EtsMethodWrappersCache etsMethodWrappersCache_ {};
    ets_proxy::EtsClassWrappersCache etsClassWrappersCache_ {};
    std::unique_ptr<ets_proxy::SharedReferenceStorage> etsProxyRefStorage_ {};

    friend class EtsJSNapiEnvScope;
};

inline JSRefConvertCache *RefConvertCacheFromInteropCtx(InteropCtx *ctx)
{
    return ctx->GetRefConvertCache();
}
inline napi_env JSEnvFromInteropCtx(InteropCtx *ctx)
{
    return ctx->GetJSEnv();
}
inline mem::GlobalObjectStorage *RefstorFromInteropCtx(InteropCtx *ctx)
{
    return ctx->Refstor();
}

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_CONTEXT_H_
