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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_INL_H_
#define PANDA_RUNTIME_VTABLE_BUILDER_INL_H_

#include "runtime/include/vtable_builder.h"

namespace ark {

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::BuildForInterface(panda_file::ClassDataAccessor *cda)
{
    ASSERT(cda->IsInterface());
    cda->EnumerateMethods([this](panda_file::MethodDataAccessor &mda) {
        if (mda.IsStatic()) {
            return;
        }

        if (!mda.IsAbstract()) {
            hasDefaultMethods_ = true;
        }

        ++numVmethods_;
    });
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::BuildForInterface(Span<Method> methods)
{
    for (const auto &method : methods) {
        if (method.IsStatic()) {
            continue;
        }

        if (!method.IsAbstract()) {
            hasDefaultMethods_ = true;
        }

        ++numVmethods_;
    }
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::AddBaseMethods(Class *baseClass)
{
    if (baseClass != nullptr) {
        auto baseClassVtable = baseClass->GetVTable();

        for (auto *method : baseClassVtable) {
            vtable_.AddBaseMethod(MethodInfo(method, 0, true));
        }
    }
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::AddClassMethods(panda_file::ClassDataAccessor *cda,
                                                                         ClassLinkerContext *ctx)
{
    cda->EnumerateMethods([this, ctx](panda_file::MethodDataAccessor &mda) {
        if (mda.IsStatic()) {
            return;
        }

        MethodInfo methodInfo(mda, numVmethods_, ctx);
        if (!vtable_.AddMethod(methodInfo).first) {
            vtable_.AddBaseMethod(methodInfo);
        }

        ++numVmethods_;
    });
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::AddClassMethods(Span<Method> methods)
{
    for (auto &method : methods) {
        if (method.IsStatic()) {
            continue;
        }

        MethodInfo methodInfo(&method, numVmethods_);
        if (!vtable_.AddMethod(methodInfo).first) {
            vtable_.AddBaseMethod(methodInfo);
        }

        ++numVmethods_;
    }
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::AddDefaultInterfaceMethods(ITable itable)
{
    for (size_t i = itable.Size(); i > 0; i--) {
        auto entry = itable[i - 1];
        auto iface = entry.GetInterface();
        if (!iface->HasDefaultMethods()) {
            continue;
        }

        auto methods = iface->GetVirtualMethods();
        for (auto &method : methods) {
            if (method.IsAbstract()) {
                continue;
            }

            auto [flag, idx] = vtable_.AddMethod(MethodInfo(&method, copiedMethods_.size(), false, true));
            if (!flag) {
                continue;
            }
            // if the default method is added for the first time, just add it.
            if (idx == MethodInfo::INVALID_METHOD_IDX) {
                CopiedMethod cmethod(&method, false, false);
                if (!IsMaxSpecificMethod(iface, method, i, itable)) {
                    cmethod.defaultAbstract = true;
                }
                copiedMethods_.push_back(cmethod);
                continue;
            }
            // use the following algorithm to judge whether we have to replace existing DEFAULT METHOD.
            // 1. if existing default method is ICCE, just skip.
            // 2. if this new method is not max-specific method, just skip.
            //    existing default method can be AME or not, has no effect on final result. its okay.
            // 3. if this new method is max-specific method, check whether existing default method is AME
            //   3.1  if no, set ICCE flag for exist method
            //   3.2  if yes, replace exist method with new method(new method becomes a candidate)
            if (copiedMethods_[idx].defaultConflict) {
                continue;
            }
            if (!IsMaxSpecificMethod(iface, method, i, itable)) {
                continue;
            }

            if (!copiedMethods_[idx].defaultAbstract) {
                copiedMethods_[idx].defaultConflict = true;
                continue;
            }
            CopiedMethod cmethod(&method, false, false);
            copiedMethods_[idx] = cmethod;
        }
    }
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::Build(panda_file::ClassDataAccessor *cda, Class *baseClass,
                                                               ITable itable, ClassLinkerContext *ctx)
{
    if (cda->IsInterface()) {
        return BuildForInterface(cda);
    }

    AddBaseMethods(baseClass);
    AddClassMethods(cda, ctx);
    AddDefaultInterfaceMethods(itable);
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::Build(Span<Method> methods, Class *baseClass, ITable itable,
                                                               bool isInterface)
{
    if (isInterface) {
        return BuildForInterface(methods);
    }

    AddBaseMethods(baseClass);
    AddClassMethods(methods);
    AddDefaultInterfaceMethods(itable);
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::UpdateClass(Class *klass) const
{
    if (klass->IsInterface()) {
        if (hasDefaultMethods_) {
            klass->SetHasDefaultMethods();
        }

        size_t idx = 0;
        for (auto &method : klass->GetVirtualMethods()) {
            method.SetVTableIndex(idx++);
        }
    }

    vtable_.UpdateClass(klass);
}

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_H_
