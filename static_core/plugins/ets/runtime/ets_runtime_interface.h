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
#ifndef PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H
#define PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H

#include "runtime/compiler.h"
#include "cross_values.h"
#include "plugins/ets/runtime/ets_vm.h"
namespace panda::ets {

class EtsRuntimeInterface : public PandaRuntimeInterface {
public:
    /**********************************************************************************/
    /// Object information
    ClassPtr GetClass(MethodPtr method, IdType id) const override;
    size_t GetTlsPromiseClassPointerOffset(Arch arch) const override
    {
        return panda::cross_values::GetEtsCoroutinePromiseClassOffset(arch);
    }
    size_t GetTlsUndefinedObjectOffset(Arch arch) const override
    {
        return panda::cross_values::GetEtsCoroutineUndefinedObjectOffset(arch);
    }
    uint64_t GetUndefinedObject() const override;
    InteropCallKind GetInteropCallKind(MethodPtr methodPtr) const override;
    char *GetFuncPropName(MethodPtr methodPtr, uint32_t strId) const override;
    uint64_t GetFuncPropNameOffset(MethodPtr methodPtr, uint32_t strId) const override;
    bool IsMethodStringBuilderConstructorWithStringArg(MethodPtr method) const override;
    bool IsMethodStringBuilderToString(MethodPtr method) const override;
    bool IsIntrinsicStringBuilderToString(IntrinsicId id) const override;

    FieldPtr ResolveLookUpField(FieldPtr rawField, ClassPtr klass) override;
    MethodPtr ResolveLookUpCall(FieldPtr rawField, ClassPtr klass, bool isSetter) override;

    template <panda_file::Type::TypeId FIELD_TYPE>
    compiler::RuntimeInterface::MethodPtr GetLookUpCall(FieldPtr rawField, ClassPtr klass, bool isSetter);

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/runtime/interop_js/ets_interop_runtime_interface-inl.h"
#endif
};
}  // namespace panda::ets

#endif  // PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H
