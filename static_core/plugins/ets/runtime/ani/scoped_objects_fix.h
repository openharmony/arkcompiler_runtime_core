/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_SCOPED_OBJECTS_FIX_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_SCOPED_OBJECTS_FIX_H

#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets::ani {

class ManagedCodeAccessor {
public:
    explicit ManagedCodeAccessor(PandaEnv *env) : env_(env) {}
    ~ManagedCodeAccessor() = default;

    EtsCoroutine *GetCoroutine() const
    {
        return env_->GetEtsCoroutine();
    }

    PandaEnv *GetPandaEnv()
    {
        return env_;
    }

    EtsObject *ToInternalType(ani_ref ref)
    {
        // NOTE: Add undefined and null support
        if (ref == nullptr) {
            return nullptr;
        }
        return reinterpret_cast<EtsObject *>(GetInternalType(env_, ref));
    }

    EtsObject *ToInternalType(ani_object obj)
    {
        ASSERT(obj != nullptr);
        return reinterpret_cast<EtsObject *>(GetInternalType(env_, obj));
    }

    EtsClass *ToInternalType(ani_class cls)
    {
        ASSERT(cls != nullptr);
        return reinterpret_cast<EtsClass *>(GetInternalType(env_, static_cast<ani_object>(cls)));
    }

    ani_ref AddLocalRef(EtsObject *obj)
    {
        return AddLocalRef(env_, obj);
    }

    static ani_ref AddLocalRef(PandaEnv *env, EtsObject *obj)
    {
        ASSERT_MANAGED_CODE();
        EtsReference *ref = GetEtsReferenceStorage(env)->NewEtsRef(obj, EtsReference::EtsObjectType::LOCAL);
        return EtsRefToAniRef(ref);
    }

    void DelLocalRef(ani_object obj)
    {
        DelLocalRef(env_, obj);
    }

    static void DelLocalRef(PandaEnv *env, ani_ref ref)
    {
        ASSERT_MANAGED_CODE();
        if (ref == nullptr) {
            return;
        }
        EtsReference *etsRef = AniRefToEtsRef(ref);
        if (!etsRef->IsLocal()) {
            LOG(WARNING, RUNTIME) << "Try to remove non-local ref: " << std::hex << etsRef;
            return;
        }
        env->GetEtsReferenceStorage()->RemoveEtsRef(etsRef);
    }

    NO_COPY_SEMANTIC(ManagedCodeAccessor);
    NO_MOVE_SEMANTIC(ManagedCodeAccessor);

protected:
    EtsReferenceStorage *GetEtsReferenceStorage()
    {
        return env_->GetEtsReferenceStorage();
    }

    static EtsReferenceStorage *GetEtsReferenceStorage(PandaEnv *env)
    {
        return env->GetEtsReferenceStorage();
    }

    static EtsObject *GetInternalType(PandaEnv *env, ani_ref ref)
    {
        if (ref == nullptr) {
            return nullptr;
        }
        EtsReference *etsRef = AniRefToEtsRef(ref);
        return GetEtsReferenceStorage(env)->GetEtsObject(etsRef);
    }

private:
    static inline EtsReference *AniRefToEtsRef(ani_ref ref)
    {
        return reinterpret_cast<EtsReference *>(ref);
    }

    static inline ani_ref EtsRefToAniRef(EtsReference *ref)
    {
        return reinterpret_cast<ani_ref>(ref);
    }

    PandaEnv *env_;
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
    explicit ScopedManagedCodeFix(PandaEnv *env)
        : ManagedCodeAccessor(env), alreadyInManaged_(ManagedThread::IsManagedScope())
    {
        ASSERT(env == PandaEnv::GetCurrent());

        if (alreadyInManaged_ && IsAccessFromManagedAllowed()) {
            return;
        }

        GetCoroutine()->ManagedCodeBegin();
    }

    ~ScopedManagedCodeFix()
    {
        if (exceptionData_) {
            // NOTE: Throw exception
        }

        if (!alreadyInManaged_) {
            GetCoroutine()->ManagedCodeEnd();
        }
    }

    bool HasPendingException()
    {
        return exceptionData_ || GetCoroutine()->HasPendingException();
    }

    NO_COPY_SEMANTIC(ScopedManagedCodeFix);
    NO_MOVE_SEMANTIC(ScopedManagedCodeFix);

private:
    inline bool IsAccessFromManagedAllowed()
    {
#ifndef NDEBUG
        auto stack = StackWalker::Create(GetCoroutine());
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

}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_SCOPED_OBJECTS_FIX_H
