/*
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

namespace panda {

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::BuildForInterface(panda_file::ClassDataAccessor *cda)
{
    ASSERT(cda->IsInterface());
    cda->EnumerateMethods([this](panda_file::MethodDataAccessor &mda) {
        if (mda.IsStatic()) {
            return;
        }

        if (!mda.IsAbstract()) {
            has_default_methods_ = true;
        }

        ++num_vmethods_;
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
            has_default_methods_ = true;
        }

        ++num_vmethods_;
    }
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::AddBaseMethods(Class *base_class)
{
    if (base_class != nullptr) {
        auto base_class_vtable = base_class->GetVTable();

        for (auto *method : base_class_vtable) {
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

        MethodInfo method_info(mda, num_vmethods_, ctx);
        if (!vtable_.AddMethod(method_info).first) {
            vtable_.AddBaseMethod(method_info);
        }

        ++num_vmethods_;
    });
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::AddClassMethods(Span<Method> methods)
{
    for (auto &method : methods) {
        if (method.IsStatic()) {
            continue;
        }

        MethodInfo method_info(&method, num_vmethods_);
        if (!vtable_.AddMethod(method_info).first) {
            vtable_.AddBaseMethod(method_info);
        }

        ++num_vmethods_;
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

            auto [flag, idx] = vtable_.AddMethod(MethodInfo(&method, copied_methods_.size(), false, true));
            if (!flag) {
                continue;
            }
            // if the default method is added for the first time, just add it.
            if (idx == MethodInfo::INVALID_METHOD_IDX) {
                CopiedMethod cmethod(&method, false, false);
                if (!IsMaxSpecificMethod(iface, method, i, itable)) {
                    cmethod.default_abstract_ = true;
                }
                copied_methods_.push_back(cmethod);
                continue;
            }
            // use the following algorithm to judge whether we have to replace existing DEFAULT METHOD.
            // 1. if existing default method is ICCE, just skip.
            // 2. if this new method is not max-specific method, just skip.
            //    existing default method can be AME or not, has no effect on final result. its okay.
            // 3. if this new method is max-specific method, check whether existing default method is AME
            //   3.1  if no, set ICCE flag for exist method
            //   3.2  if yes, replace exist method with new method(new method becomes a candidate)
            if (copied_methods_[idx].default_conflict_) {
                continue;
            }
            if (!IsMaxSpecificMethod(iface, method, i, itable)) {
                continue;
            }

            if (!copied_methods_[idx].default_abstract_) {
                copied_methods_[idx].default_conflict_ = true;
                continue;
            }
            CopiedMethod cmethod(&method, false, false);
            copied_methods_[idx] = cmethod;
        }
    }
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::Build(panda_file::ClassDataAccessor *cda, Class *base_class,
                                                               ITable itable, ClassLinkerContext *ctx)
{
    if (cda->IsInterface()) {
        return BuildForInterface(cda);
    }

    AddBaseMethods(base_class);
    AddClassMethods(cda, ctx);
    AddDefaultInterfaceMethods(itable);
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::Build(Span<Method> methods, Class *base_class, ITable itable,
                                                               bool is_interface)
{
    if (is_interface) {
        return BuildForInterface(methods);
    }

    AddBaseMethods(base_class);
    AddClassMethods(methods);
    AddDefaultInterfaceMethods(itable);
}

template <class SearchBySignature, class OverridePred>
void VTableBuilderImpl<SearchBySignature, OverridePred>::UpdateClass(Class *klass) const
{
    if (klass->IsInterface()) {
        if (has_default_methods_) {
            klass->SetHasDefaultMethods();
        }

        size_t idx = 0;
        for (auto &method : klass->GetVirtualMethods()) {
            method.SetVTableIndex(idx++);
        }
    }

    vtable_.UpdateClass(klass);
}

}  // namespace panda

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_H_
