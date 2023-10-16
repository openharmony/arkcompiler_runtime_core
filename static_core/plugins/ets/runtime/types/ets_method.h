/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_METHOD_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_METHOD_H_

#include "libpandabase/macros.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/file_items.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"
#include "runtime/include/thread.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_value.h"

namespace panda {
class Value;
}  // namespace panda

namespace panda::ets {
namespace napi {
class ScopedManagedCodeFix;
}  // namespace napi

class PandaEtsEnv;

class EtsMethod {
public:
    PANDA_PUBLIC_API static EtsMethod *FromTypeDescriptor(const PandaString &td);

    PANDA_PUBLIC_API static bool IsMethod(const PandaString &td);

    PANDA_PUBLIC_API EtsValue Invoke(napi::ScopedManagedCodeFix *s, Value *args);

    void InvokeVoid(napi::ScopedManagedCodeFix *s, Value *args)
    {
        Invoke(s, args);
    }

    uint32_t GetParametersNum() const
    {
        if (IsStatic()) {
            return GetNumArgs();
        }
        return GetNumArgs() - 1;
    }

    uint32_t GetNumArgs() const
    {
        return GetPandaMethod()->GetNumArgs();
    }

    PANDA_PUBLIC_API uint32_t GetNumArgSlots() const;

    EtsType GetArgType(size_t idx) const
    {
        EtsType ets_type = ConvertPandaTypeToEtsType(GetPandaMethod()->GetArgType(idx));
        if (ets_type == EtsType::VOID) {
            LOG(FATAL, RUNTIME) << "VOID parameter";
        }
        return ets_type;
    }

    EtsType GetReturnValueType() const
    {
        panda_file::Type panda_type = GetPandaMethod()->GetReturnType();
        return ConvertPandaTypeToEtsType(panda_type);
    }

    EtsType GetEffectiveArgType(size_t idx) const
    {
        EtsType ets_type = ConvertPandaTypeToEtsType(GetPandaMethod()->GetEffectiveArgType(idx));
        if (ets_type == EtsType::VOID) {
            LOG(FATAL, RUNTIME) << "VOID parameter";
        }
        return ets_type;
    }

    EtsType GetEffectiveReturnValueType() const
    {
        panda_file::Type panda_type = GetPandaMethod()->GetEffectiveReturnType();
        return ConvertPandaTypeToEtsType(panda_type);
    }

    const char *GetRefArgType(size_t idx) const
    {
        return utf::Mutf8AsCString(GetPandaMethod()->GetRefArgType(idx).data);
    }

    const char *GetName() const
    {
        return utf::Mutf8AsCString(GetPandaMethod()->GetName().data);
    }

    PandaString GetFullName(bool with_signature = false) const
    {
        return GetPandaMethod()->GetFullName(with_signature);
    }

    EtsString *GetNameString()
    {
        auto name_data = GetPandaMethod()->GetName();
        return EtsString::Resolve(name_data.data, name_data.utf16_length);
    }

    bool IsStatic() const
    {
        return GetPandaMethod()->IsStatic();
    }

    bool IsSynthetic() const
    {
        return GetPandaMethod()->IsSynthetic();
    }

    PANDA_PUBLIC_API bool IsEqualParametersType(EtsArray *params) const;

    EtsClass *GetClass() const
    {
        return EtsClass::FromRuntimeClass(GetPandaMethod()->GetClass());
    }

    PANDA_PUBLIC_API EtsClass *ResolveArgType(uint32_t idx);

    EtsClass *ResolveReturnType()
    {
        Method::Proto proto = GetPandaMethod()->GetProto();
        const char *descriptor = proto.GetReturnTypeDescriptor().data();
        Runtime::GetCurrent()->GetClassLinker();
        return EtsClass::FromRuntimeClass(Runtime::GetCurrent()->GetClassLinker()->GetClass(
            utf::CStringAsMutf8(descriptor), false, GetClass()->GetClassLoader()));
    }

    size_t GetVTableID() const
    {
        return GetPandaMethod()->GetVTableIndex();
    }

    bool IsPublic() const
    {
        return GetPandaMethod()->IsPublic();
    }

    bool IsProtected() const
    {
        return GetPandaMethod()->IsProtected();
    }

    bool IsPrivate() const
    {
        return GetPandaMethod()->IsPrivate();
    }

    bool IsNative() const
    {
        return GetPandaMethod()->IsNative();
    }

    bool IsFastNative() const
    {
        return (GetAccessFlags() & ACC_FAST_NATIVE) != 0;
    }

    bool IsCriticalNative() const
    {
        return (GetAccessFlags() & ACC_CRITICAL_NATIVE) != 0;
    }

    bool IsConstructor() const
    {
        return GetPandaMethod()->IsConstructor();
    }

    bool IsInstanceConstructor() const
    {
        return GetPandaMethod()->IsInstanceConstructor();
    }

    bool IsAbstract() const
    {
        return GetPandaMethod()->IsAbstract();
    }

    void RegisterNativeImpl(void *impl)
    {
        ASSERT(IsNative());
        GetPandaMethod()->SetNativePointer(impl);
    }

    void UnregisterNativeImpl()
    {
        ASSERT(IsNative());
        GetPandaMethod()->SetNativePointer(nullptr);
    }

    uint32_t GetAccessFlags() const
    {
        return GetPandaMethod()->GetAccessFlags();
    }

    AccessLevel GetAccessLevel()
    {
        if (GetPandaMethod()->IsPublic()) {
            return AccessLevel::PUBLIC;
        }
        if (GetPandaMethod()->IsProtected()) {
            return AccessLevel::PROTECTED;
        }
        if (GetPandaMethod()->IsPrivate()) {
            return AccessLevel::PRIVATE;
        }
        return AccessLevel::DEFAULT;
    }

    uint32_t GetMethodId() const
    {
        return GetPandaMethod()->GetFileId().GetOffset();
    }

    PANDA_PUBLIC_API EtsMethod *GetOverriddenMethod();

    int32_t GetLineNumFromBytecodeOffset(uint32_t bc_offset) const
    {
        if (IsNative()) {
            return -2;  // -2
        }

        if (IsAbstract()) {
            return -1;
        }

        return GetPandaMethod()->GetLineNumFromBytecodeOffset(bc_offset);
    }

    panda_file::File::StringData GetClassSourceFile() const
    {
        return GetPandaMethod()->GetClassSourceFile();
    }

    static EtsMethod *FromRuntimeMethod(Method *method)
    {
        return reinterpret_cast<EtsMethod *>(method);
    }

    static Method *ToRuntimeMethod(EtsMethod *ets_method)
    {
        return reinterpret_cast<Method *>(ets_method);
    }

    EtsMethod() = delete;
    ~EtsMethod() = delete;

    const Method *GetPandaMethod() const
    {
        return reinterpret_cast<const Method *>(this);
    }

    Method *GetPandaMethod()
    {
        return reinterpret_cast<Method *>(this);
    }

    std::string_view GetReturnTypeDescriptor()
    {
        return GetPandaMethod()->GetProto().GetReturnTypeDescriptor();
    }

    PANDA_PUBLIC_API PandaString GetMethodSignature(bool include_return_type = true) const;

    PANDA_PUBLIC_API PandaString GetDescriptor() const;

    NO_COPY_SEMANTIC(EtsMethod);
    NO_MOVE_SEMANTIC(EtsMethod);
};

}  // namespace panda::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_METHOD_H_
