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
        auto [it, inserted] = string_tab_.insert({str, 0});
        it->second++;
        return CachedEntry(&it->first);
    }

    void Release(CachedEntry str)
    {
        auto it = string_tab_.find(*str.Data());
        ASSERT(it != string_tab_.end());
        if (--(it->second) == 0) {
            string_tab_.erase(it);
        }
    }

private:
    std::unordered_map<std::string, uint64_t> string_tab_;
};

using ArgValueBox = std::variant<uint64_t, ObjectHeader **>;

class LocalScopesStorage {
public:
    LocalScopesStorage() = default;

    ~LocalScopesStorage()
    {
        ASSERT(local_scopes_storage_.empty());
    }

    NO_COPY_SEMANTIC(LocalScopesStorage);
    NO_MOVE_SEMANTIC(LocalScopesStorage);

    void CreateLocalScope(napi_env env, Frame *frame)
    {
        napi_handle_scope scope;
        [[maybe_unused]] auto status = napi_open_handle_scope(env, &scope);
        ASSERT(status == napi_ok);
        local_scopes_storage_.emplace_back(frame, scope);
    }

    void DestroyTopLocalScope(napi_env env, [[maybe_unused]] Frame *curr_frame)
    {
        ASSERT(!local_scopes_storage_.empty());
        auto &[frame, scope] = local_scopes_storage_.back();
        ASSERT(curr_frame == frame);
        local_scopes_storage_.pop_back();
        [[maybe_unused]] auto status = napi_close_handle_scope(env, scope);
        ASSERT(status == napi_ok);
    }

    void DestroyLocalScopeForTopFrame(napi_env env, Frame *curr_frame)
    {
        if (local_scopes_storage_.empty()) {
            return;
        }
        auto &[frame, scope] = local_scopes_storage_.back();
        while (frame == curr_frame) {
            local_scopes_storage_.pop_back();
            [[maybe_unused]] auto status = napi_close_handle_scope(env, scope);
            ASSERT(status == napi_ok);
            if (local_scopes_storage_.empty()) {
                return;
            }
            std::tie(frame, scope) = local_scopes_storage_.back();
        }
    }

private:
    std::vector<std::pair<const Frame *, napi_handle_scope>> local_scopes_storage_ {};
};

class InteropCtx {
public:
    static void Init(EtsCoroutine *coro, napi_env env)
    {
        // Initialize InteropCtx in VM ExternalData
        new (InteropCtx::Current(coro)) InteropCtx(coro, env);
    }

    static InteropCtx *Current(PandaEtsVM *ets_vm)
    {
        static_assert(sizeof(PandaEtsVM::ExternalData) >= sizeof(InteropCtx));
        static_assert(alignof(PandaEtsVM::ExternalData) >= alignof(InteropCtx));
        return reinterpret_cast<InteropCtx *>(ets_vm->GetExternalData());
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
        ASSERT(js_env_ != nullptr);
        return js_env_;
    }

    void SetJSEnv(napi_env env)
    {
        js_env_ = env;
    }

    mem::GlobalObjectStorage *Refstor() const
    {
        return refstor_;
    }

    ClassLinkerContext *LinkerCtx() const
    {
        return linker_ctx_;
    }

    JSValueStringStorage *GetStringStor()
    {
        return &js_value_string_stor_;
    }

    LocalScopesStorage *GetLocalScopesStorage()
    {
        return &local_scopes_storage_;
    }

    void DestroyLocalScopeForTopFrame(Frame *frame)
    {
        GetLocalScopesStorage()->DestroyLocalScopeForTopFrame(js_env_, frame);
    }

    mem::Reference *GetJSValueFinalizationQueue() const
    {
        return jsvalue_fqueue_ref_;
    }

    Method *GetRegisterFinalizerMethod() const
    {
        return jsvalue_fqueue_register_;
    }

    // NOTE(vpukhov): implement in native code
    [[nodiscard]] bool PushOntoFinalizationQueue(EtsCoroutine *coro, EtsObject *obj, EtsObject *cbarg)
    {
        auto queue = Refstor()->Get(jsvalue_fqueue_ref_);
        std::array<Value, 4> args = {Value(queue), Value(obj->GetCoreType()), Value(cbarg->GetCoreType()),
                                     Value(static_cast<ObjectHeader *>(nullptr))};
        jsvalue_fqueue_register_->Invoke(coro, args.data());
        return !coro->HasPendingException();
    }

    template <typename T, size_t OPT_SZ>
    struct TempArgs {
    public:
        explicit TempArgs(size_t sz)
        {
            if (LIKELY(sz <= OPT_SZ)) {
                sp_ = {new (inl_arr_.data()) T[sz], sz};
            } else {
                sp_ = {new T[sz], sz};
            }
        }

        ~TempArgs()
        {
            if (UNLIKELY(static_cast<void *>(sp_.data()) != inl_arr_.data())) {
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
        TempArgs(TempArgs &&other) : sp_(other.sp_), inl_arr_(other.inl_arr_)
        {
            if (LIKELY(sp_.size() <= OPT_SZ)) {
                sp_ = Span<T>(reinterpret_cast<T *>(inl_arr_.data()), sp_.size());
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
        alignas(T) std::array<uint8_t, sizeof(T) * OPT_SZ> inl_arr_;
    };

    template <typename T, size_t OPT_SZ = 8U>
    ALWAYS_INLINE static auto GetTempArgs(size_t sz)
    {
        return TempArgs<T, OPT_SZ>(sz);
    }

    // NOTE(vpukhov): implement as flags in IFrame
    struct InteropFrameRecord {
        void *ets_frame {};
        bool to_js {};
    };

    std::vector<InteropFrameRecord> &GetInteropFrames()
    {
        return interop_frames_;
    }

    JSRefConvertCache *GetRefConvertCache()
    {
        return &refconvert_cache_;
    }

    Class *GetJSValueClass() const
    {
        return jsvalue_class_;
    }

    Class *GetJSErrorClass() const
    {
        return jserror_class_;
    }

    Class *GetObjectClass() const
    {
        return object_class_;
    }

    Class *GetStringClass() const
    {
        return string_class_;
    }

    Class *GetVoidClass() const
    {
        return void_class_;
    }

    Class *GetPromiseClass() const
    {
        return promise_class_;
    }

    Class *GetErrorClass() const
    {
        return error_class_;
    }

    Class *GetExceptionClass() const
    {
        return exception_class_;
    }

    Class *GetTypeClass() const
    {
        return type_class_;
    }

    Class *GetBoxIntClass() const
    {
        return box_int_class_;
    }

    Class *GetBoxLongClass() const
    {
        return box_long_class_;
    }

    Class *GetArrayClass() const
    {
        return array_class_;
    }

    Class *GetArrayBufferClass() const
    {
        return arraybuf_class_;
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

    void SetPendingNewInstance(EtsObject *new_instance)
    {
        pending_new_instance_ = new_instance;
    }

    EtsObject *AcquirePendingNewInstance()
    {
        auto res = pending_new_instance_;
        pending_new_instance_ = nullptr;
        return res;
    }

    ets_proxy::EtsMethodWrappersCache *GetEtsMethodWrappersCache()
    {
        return &ets_method_wrappers_cache_;
    }

    ets_proxy::EtsClassWrappersCache *GetEtsClassWrappersCache()
    {
        return &ets_class_wrappers_cache_;
    }

    ets_proxy::SharedReferenceStorage *GetSharedRefStorage()
    {
        return ets_proxy_ref_storage_.get();
    }

private:
    explicit InteropCtx(EtsCoroutine *coro, napi_env env);

    napi_env js_env_ {};

    mem::GlobalObjectStorage *refstor_ {};
    ClassLinkerContext *linker_ctx_ {};
    JSValueStringStorage js_value_string_stor_ {};
    LocalScopesStorage local_scopes_storage_ {};
    mem::Reference *jsvalue_fqueue_ref_ {};

    std::vector<InteropFrameRecord> interop_frames_ {};

    JSRefConvertCache refconvert_cache_;

    Class *jsruntime_class_ {};
    Class *jsvalue_class_ {};
    Class *jserror_class_ {};
    Class *object_class_ {};
    Class *string_class_ {};
    Class *void_class_ {};
    Class *promise_class_ {};
    Class *error_class_ {};
    Class *exception_class_ {};
    Class *type_class_ {};

    Class *box_int_class_ {};
    Class *box_long_class_ {};

    Class *array_class_ {};
    Class *arraybuf_class_ {};

    Method *jsvalue_fqueue_register_ {};

    // ets_proxy data
    EtsObject *pending_new_instance_ {};
    ets_proxy::EtsMethodWrappersCache ets_method_wrappers_cache_ {};
    ets_proxy::EtsClassWrappersCache ets_class_wrappers_cache_ {};
    std::unique_ptr<ets_proxy::SharedReferenceStorage> ets_proxy_ref_storage_ {};

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
