/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_NAPI_ETS_SCOPED_OBJECTS_FIX_H
#define PANDA_PLUGINS_ETS_RUNTIME_NAPI_ETS_SCOPED_OBJECTS_FIX_H

#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/mem/ets_reference.h"

namespace ark::ets::napi {
enum class EtsNapiException {
    ARRAY_INDEX_OUT_OF_BOUNDS,
    ARRAY_STORE,
    ILLEGAL_MONITOR_STATE,
    INSTANTIATION,
    NO_CLASS_DEF_FOUND,
    NO_SUCH_FIELD,
    NO_SUCH_METHOD,
    NULL_POINTER_ERROR,
    OUT_OF_MEMORY,
    STRING_INDEX_OUT_OF_BOUNDS
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RETURN_NULL_IF_NULL(ptr) \
    {                            \
        if ((ptr) == nullptr) {  \
            return nullptr;      \
        }                        \
    }

class ManagedCodeAccessor {
public:
    explicit ManagedCodeAccessor(PandaEtsNapiEnv *env) : env_(env) {}
    ~ManagedCodeAccessor() = default;

    EtsClass *ToInternalType(ets_class cls)
    {
        RETURN_NULL_IF_NULL(cls);
        return reinterpret_cast<EtsClass *>(GetInternalType(env_, reinterpret_cast<ets_object>(cls)));
    }

    EtsObject *ToInternalType(ets_object obj)
    {
        RETURN_NULL_IF_NULL(obj);
        return GetInternalType(env_, obj);
    }

    EtsString *ToInternalType(ets_string str)
    {
        RETURN_NULL_IF_NULL(str);
        return reinterpret_cast<EtsString *>(GetInternalType(env_, reinterpret_cast<ets_object>(str)));
    }

    EtsArray *ToInternalType(ets_array arr)
    {
        RETURN_NULL_IF_NULL(arr);
        return reinterpret_cast<EtsArray *>(GetInternalType(env_, reinterpret_cast<ets_object>(arr)));
    }

    EtsObjectArray *ToInternalType(ets_objectArray arr)
    {
        RETURN_NULL_IF_NULL(arr);
        return reinterpret_cast<EtsObjectArray *>(ToInternalType(reinterpret_cast<ets_object>(arr)));
    }

    EtsPromise *ToInternalType(ets_deferred deferred)
    {
        RETURN_NULL_IF_NULL(deferred);
        return reinterpret_cast<EtsPromise *>(ToInternalType(reinterpret_cast<ets_object>(deferred)));
    }

    template <typename T>
    T *Convert(ets_object obj)
    {
        return Convert<T>(env_, obj);
    }

    template <typename T>
    static T *Convert(PandaEtsNapiEnv *env, ets_object obj)
    {
        return reinterpret_cast<T *>(GetInternalType(env, obj));
    }

    EtsCoroutine *Coroutine()
    {
        return env_->GetEtsCoroutine();
    }

    PandaEtsNapiEnv *EtsNapiEnv()
    {
        return env_;
    }

    ets_object AddGlobalRef(EtsObject *obj)
    {
        EtsReference *ref = GetEtsReferenceStorage()->NewEtsRef(obj, EtsReference::EtsObjectType::GLOBAL);
        return EtsRefToEtsObject(ref);
    }

    ets_object AddLocalRef(EtsObject *obj)
    {
        return AddLocalRef(env_, obj);
    }

    static ets_object AddLocalRef(PandaEtsNapiEnv *env, EtsObject *obj)
    {
        ASSERT_MANAGED_CODE();
        EtsReference *ref = GetEtsReferenceStorage(env)->NewEtsRef(obj, EtsReference::EtsObjectType::LOCAL);
        return EtsRefToEtsObject(ref);
    }

    ets_object AddWeakGlobalRef(EtsObject *obj)
    {
        return AddWeakGlobalRef(env_, obj);
    }

    static ets_object AddWeakGlobalRef(PandaEtsNapiEnv *env, EtsObject *obj)
    {
        ASSERT_MANAGED_CODE();
        RETURN_NULL_IF_NULL(obj);
        EtsReferenceStorage *refStorage = GetEtsReferenceStorage(env);
        EtsReference *ref = refStorage->NewEtsRef(obj, EtsReference::EtsObjectType::WEAK);
        return EtsRefToEtsNapiWeak(ref);
    }

    void DelLocalRef(ets_object obj)
    {
        DelLocalRef(env_, obj);
    }

    static void DelLocalRef(PandaEtsNapiEnv *env, ets_object obj)
    {
        ASSERT_MANAGED_CODE();
        if (obj == nullptr) {
            return;
        }
        EtsReference *ref = EtsObjectToEtsRef(obj);
        if (!ref->IsLocal()) {
            LOG(WARNING, RUNTIME) << "Try to remove non-local ref: " << std::hex << ref;
            return;
        }
        env->GetEtsReferenceStorage()->RemoveEtsRef(ref);
    }

    bool IsValidRef(ets_object obj)
    {
        EtsReference *ref = EtsObjectToEtsRef(obj);
        return GetEtsReferenceStorage()->IsValidEtsRef(ref);
    }

    void TraverseReferenceTable()
    {
        mem::ReferenceStorage *refStorage = GetEtsReferenceStorage()->GetAsReferenceStorage();
        PandaVector<ObjectHeader *> table = refStorage->GetAllObjects();
        for (const auto *obj : table) {
            std::cout << "object classname = ";
            std::cout << obj->ClassAddr<Class>()->GetName() << std::endl;
        }
    }

    EtsCoroutine *GetEtsCoroutine() const
    {
        return env_->GetEtsCoroutine();
    }

    NO_COPY_SEMANTIC(ManagedCodeAccessor);
    NO_MOVE_SEMANTIC(ManagedCodeAccessor);

protected:
    EtsReferenceStorage *GetEtsReferenceStorage()
    {
        return env_->GetEtsReferenceStorage();
    }

    static EtsReferenceStorage *GetEtsReferenceStorage(PandaEtsNapiEnv *env)
    {
        return env->GetEtsReferenceStorage();
    }

    static EtsObject *GetInternalType(PandaEtsNapiEnv *env, ets_object obj)
    {
        if (obj == nullptr) {
            return nullptr;
        }
        EtsReference *ref = EtsObjectToEtsRef(obj);
        return GetEtsReferenceStorage(env)->GetEtsObject(ref);
    }

private:
    PandaEtsNapiEnv *env_;
};

class ScopedManagedCodeFix : public ManagedCodeAccessor {
    class ExceptionData {
    public:
        ExceptionData(const char *name, const char *message) : className_(name)
        {
            if (message != nullptr) {
                message_ = message;
            }
        }

        const char *GetClassName() const
        {
            return className_.c_str();
        }

        const char *GetMessage() const
        {
            return message_ ? message_.value().c_str() : nullptr;
        }

    private:
        PandaString className_;
        std::optional<PandaString> message_;
    };

public:
    explicit ScopedManagedCodeFix(PandaEtsNapiEnv *env)
        : ManagedCodeAccessor(env), alreadyInManaged_(ManagedThread::IsManagedScope())
    {
        ASSERT(env == PandaEtsNapiEnv::GetCurrent());

        if (alreadyInManaged_ && IsAccessFromManagedAllowed()) {
            return;
        }

        Coroutine()->ManagedCodeBegin();
    }

    ~ScopedManagedCodeFix()
    {
        if (exceptionData_) {
            ThrowEtsException(Coroutine(), exceptionData_->GetClassName(), exceptionData_->GetMessage());
        }

        if (!alreadyInManaged_) {
            Coroutine()->ManagedCodeEnd();
        }
    }

    void ThrowNewException(EtsNapiException kind, const char *message)
    {
        ASSERT(!exceptionData_);

        std::string_view className {};
        switch (kind) {
            case EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS:
                className = panda_file_items::class_descriptors::ARRAY_INDEX_OUT_OF_BOUNDS_ERROR;
                break;
            case EtsNapiException::ARRAY_STORE:
                className = panda_file_items::class_descriptors::ARRAY_STORE_ERROR;
                break;
            case EtsNapiException::ILLEGAL_MONITOR_STATE:
                className = panda_file_items::class_descriptors::ILLEGAL_MONITOR_STATE_EXCEPTION;
                break;
            case EtsNapiException::INSTANTIATION:
                className = panda_file_items::class_descriptors::INSTANTIATION_ERROR;
                break;
            case EtsNapiException::NO_CLASS_DEF_FOUND:
                className = panda_file_items::class_descriptors::NO_CLASS_DEF_FOUND_ERROR;
                break;
            case EtsNapiException::NO_SUCH_FIELD:
                className = panda_file_items::class_descriptors::NO_SUCH_FIELD_ERROR;
                break;
            case EtsNapiException::NO_SUCH_METHOD:
                className = panda_file_items::class_descriptors::NO_SUCH_METHOD_ERROR;
                break;
            case EtsNapiException::NULL_POINTER_ERROR:
                className = panda_file_items::class_descriptors::NULL_POINTER_ERROR;
                break;
            case EtsNapiException::OUT_OF_MEMORY:
                className = panda_file_items::class_descriptors::OUT_OF_MEMORY_ERROR;
                break;
            case EtsNapiException::STRING_INDEX_OUT_OF_BOUNDS:
                className = panda_file_items::class_descriptors::STRING_INDEX_OUT_OF_BOUNDS_ERROR;
                break;
            default:
                UNREACHABLE();
        }

        exceptionData_ = MakePandaUnique<ExceptionData>(className.data(), message);
    }

    bool HasPendingException()
    {
        return exceptionData_ || Coroutine()->HasPendingException();
    }

    NO_COPY_SEMANTIC(ScopedManagedCodeFix);
    NO_MOVE_SEMANTIC(ScopedManagedCodeFix);

private:
    bool IsAccessFromManagedAllowed()
    {
#ifndef NDEBUG
        auto stack = StackWalker::Create(GetEtsCoroutine());
        auto method = EtsMethod::FromRuntimeMethod(stack.GetMethod());
        ASSERT(method != nullptr);
        return method->IsFastNative();
#else
        return true;
#endif  // NDEBUG
    }

    PandaUniquePtr<ExceptionData> exceptionData_;
    bool alreadyInManaged_;
};

class ScopedManagedCodeFastNative : public ManagedCodeAccessor {
public:
    explicit ScopedManagedCodeFastNative(PandaEtsNapiEnv *env) : ManagedCodeAccessor(env)
    {
        ASSERT(Coroutine()->IsManagedCode());
    }

    ~ScopedManagedCodeFastNative()
    {
        ASSERT(Coroutine()->IsManagedCode());
    }

    NO_COPY_SEMANTIC(ScopedManagedCodeFastNative);
    NO_MOVE_SEMANTIC(ScopedManagedCodeFastNative);
};
}  // namespace ark::ets::napi

#endif  // PANDA_PLUGINS_ETS_RUNTIME_NAPI_ETS_SCOPED_OBJECTS_FIX_H
