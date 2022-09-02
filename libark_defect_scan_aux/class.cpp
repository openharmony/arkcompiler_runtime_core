/*
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

#include "class.h"
#include "abc_file.h"
#include "function.h"

namespace panda::defect_scan_aux {
const std::string &Class::GetClassName() const
{
    return class_name_;
}

const AbcFile *Class::GetAbcFileInstance() const
{
    return abc_file_;
}

const Function *Class::GetDefineFunction() const
{
    return def_func_;
}

size_t Class::GetMemberFunctionCount() const
{
    return member_func_list_.size();
}

const Function *Class::GetMemberFunctionByName(std::string_view func_name) const
{
    for (auto func : member_func_list_) {
        if (func->GetFunctionName() == func_name) {
            return func;
        }
    }
    if (par_class_info_.class_ != nullptr) {
        return par_class_info_.class_->GetMemberFunctionByName(func_name);
    }
    return nullptr;
}

const Function *Class::GetMemberFunctionByIndex(size_t index) const
{
    ASSERT(index < member_func_list_.size());
    return member_func_list_[index];
}

const Class *Class::GetParentClass() const
{
    return par_class_info_.class_;
}

const std::string &Class::GetParentClassName() const
{
    return par_class_info_.class_name_;
}

const std::string &Class::GetParClassExternalModuleName() const
{
    return par_class_info_.external_module_name_;
}

const std::string &Class::GetParClassGlobalVarName() const
{
    return par_class_info_.global_var_name_;
}

void Class::SetParentClass(const Class *parent_class)
{
    ASSERT(parent_class != nullptr);
    par_class_info_.class_ = parent_class;
    par_class_info_.class_name_ = parent_class->GetClassName();
}

void Class::SetParentClassName(std::string_view par_class_name)
{
    par_class_info_.class_name_ = par_class_name;
}

void Class::SetParClassExternalModuleName(std::string_view external_module_name)
{
    par_class_info_.external_module_name_ = external_module_name;
}

void Class::SetParClassGlobalVarName(std::string global_var_name)
{
    par_class_info_.global_var_name_ = global_var_name;
}

void Class::AddMemberFunction(const Function *func)
{
    ASSERT(func != nullptr);
    member_func_list_.push_back(func);
}
}  // namespace panda::defect_scan_aux
