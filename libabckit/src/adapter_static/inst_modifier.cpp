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

#include "inst_modifier.h"

#include "libabckit/src/logger.h"
#include "metadata_inspect_static.h"
#include "name_util.h"
#include "helpers_static.h"
#include "string_util.h"

using ark::pandasm::Opcode;

namespace {
using OpcodeList = std::vector<Opcode>;

/**
 * opcodes of using field_id ins
 */
const OpcodeList FIELD_INST_LIST = {
    Opcode::STSTATIC,    Opcode::STSTATIC_64,  Opcode::STSTATIC_OBJ, Opcode::STOBJ,       Opcode::STOBJ_64,
    Opcode::STOBJ_OBJ,   Opcode::STOBJ_V,      Opcode::STOBJ_V_64,   Opcode::STOBJ_V_OBJ, Opcode::LDSTATIC,
    Opcode::LDSTATIC_64, Opcode::LDSTATIC_OBJ, Opcode::LDOBJ,        Opcode::LDOBJ_64,    Opcode::LDOBJ_OBJ,
    Opcode::LDOBJ_V,     Opcode::LDOBJ_V_64,   Opcode::LDOBJ_V_OBJ,
};

/**
 * opcodes of using type_id ins
 */
const OpcodeList CLASS_INST_LIST = {
    Opcode::NEWOBJ, Opcode::NEWARR, Opcode::ISINSTANCE, Opcode::CHECKCAST, Opcode::LDA_TYPE,
};

/**
 * opcodes of using method_id ins
 */
const OpcodeList METHOD_INST_LIST = {
    Opcode::INITOBJ,
    Opcode::INITOBJ_SHORT,
    Opcode::INITOBJ_RANGE,
    Opcode::CALL,
    Opcode::CALL_SHORT,
    Opcode::CALL_RANGE,
    Opcode::CALL_ACC,
    Opcode::CALL_ACC_SHORT,
    Opcode::CALL_VIRT,
    Opcode::CALL_VIRT_SHORT,
    Opcode::CALL_VIRT_RANGE,
    Opcode::CALL_VIRT_ACC,
    Opcode::CALL_VIRT_ACC_SHORT,
};

/**
 * check whether ins is in the opcode list
 */
bool InOpcodeList(const ark::pandasm::Ins &ins, const OpcodeList &list)
{
    return std::any_of(list.begin(), list.end(), [&](const auto &elem) { return elem == ins.opcode; });
}
}  // namespace

void libabckit::InstModifier::EnumerateInst(const std::function<void(ark::pandasm::Ins &ins)> &visitor) const
{
    LIBABCKIT_LOG_FUNC;

    for (auto &[funcName, function] : this->file_->nameToFunctionStatic) {
        LIBABCKIT_LOG(DEBUG) << "Enumerate function inst for:" << funcName << std::endl;
        for (auto &ins : function->GetArkTSImpl()->GetStaticImpl()->ins) {
            visitor(ins);
        }
    }

    for (auto &[funcName, function] : this->file_->nameToFunctionInstance) {
        LIBABCKIT_LOG(DEBUG) << "Enumerate function inst for:" << funcName << std::endl;
        for (auto &ins : function->GetArkTSImpl()->GetStaticImpl()->ins) {
            visitor(ins);
        }
    }
}

AbckitCoreFunction *libabckit::InstModifier::GetFunction(const std::string &name) const
{
    if (this->file_->nameToFunctionStatic.find(name) != this->file_->nameToFunctionStatic.end()) {
        return this->file_->nameToFunctionStatic.at(name);
    }

    if (this->file_->nameToFunctionInstance.find(name) != this->file_->nameToFunctionInstance.end()) {
        return this->file_->nameToFunctionInstance.at(name);
    }

    return nullptr;
}

void libabckit::InstModifier::ModifyInstFunction(ark::pandasm::Ins &ins) const
{
    if (!ins.HasFlag(ark::pandasm::STATIC_METHOD_ID) && !ins.HasFlag(ark::pandasm::METHOD_ID) &&
        !InOpcodeList(ins, METHOD_INST_LIST)) {
        return;
    }

    LIBABCKIT_LOG_FUNC;

    auto oldName = ins.GetID(0);
    const auto function = this->GetFunction(oldName);
    if (function == nullptr) {
        return;
    }

    if (FunctionIsExternalStatic(function)) {  // ignore external function
        return;
    }

    const auto newName = NameUtil::GetFullName(function);
    if (oldName == newName) {
        return;
    }

    LIBABCKIT_LOG(DEBUG) << "ins old function: " << oldName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "ins new function: " << newName << std::endl;

    ins.SetID(0, newName);
}

void libabckit::InstModifier::ModifyInstField(ark::pandasm::Ins &ins) const
{
    if (!InOpcodeList(ins, FIELD_INST_LIST)) {
        return;
    }

    LIBABCKIT_LOG_FUNC;

    const auto oldName = ins.GetID(0);
    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    if (this->file_->nameToField.find(oldName) != this->file_->nameToField.end()) {
        std::visit(
            [&](auto *object) {
                const auto newName =
                    object->GetOwnerStaticImpl()->name + "." + object->GetArkTSImpl()->GetStaticImpl()->name;
                if (oldName == newName) {
                    return;
                }

                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "ins old fieldName: " << oldName << std::endl;
                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "ins new fieldName: " << newName << std::endl;

                ins.SetID(0, newName);
            },
            this->file_->nameToField[oldName]);
    }
}

void libabckit::InstModifier::ModifyInstClass(ark::pandasm::Ins &ins) const
{
    if (!InOpcodeList(ins, CLASS_INST_LIST)) {
        return;
    }

    LIBABCKIT_LOG_FUNC;

    const auto oldName = ins.GetID(0);
    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    if (this->file_->nameToClass.find(oldName) != this->file_->nameToClass.end()) {
        std::visit(
            [&](auto *object) {
                const auto newName = object->GetArkTSImpl()->impl.GetStaticClass()->name;
                if (oldName == newName) {
                    return;
                }

                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "ins old className: " << oldName << std::endl;
                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "ins new className: " << newName << std::endl;

                ins.SetID(0, newName);
            },
            this->file_->nameToClass[oldName]);
    }
}

void libabckit::InstModifier::RefreshFunctions(std::unordered_map<std::string, AbckitCoreFunction *> &nameToFunction)
{
    LIBABCKIT_LOG_FUNC;

    for (auto it = nameToFunction.begin(); it != nameToFunction.end();) {
        auto name = GetMangleFuncName(it->second);
        if (name != it->first) {
            LIBABCKIT_LOG(DEBUG) << "refresh file nameToFunction: " << it->first << " --> " << name << std::endl;
            auto node = nameToFunction.extract(it++);
            node.key() = name;
            nameToFunction.insert(std::move(node));
        } else {
            ++it;
        }
    }
}

void libabckit::InstModifier::RefreshFields()
{
    LIBABCKIT_LOG_FUNC;

    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    for (auto it = this->file_->nameToField.begin(); it != this->file_->nameToField.end();) {
        std::visit(
            [&](auto *object) {
                const auto name =
                    object->GetOwnerStaticImpl()->name + "." + object->GetArkTSImpl()->GetStaticImpl()->name;
                if (name != it->first) {
                    LIBABCKIT_LOG_NO_FUNC(DEBUG)
                        << funcName << "refresh file nameToField: " << it->first << " --> " << name << std::endl;
                    auto node = this->file_->nameToField.extract(it++);
                    node.key() = name;
                    this->file_->nameToField.insert(std::move(node));
                } else {
                    ++it;
                }
            },
            it->second);
    }
}

void libabckit::InstModifier::RefreshClasses()
{
    LIBABCKIT_LOG_FUNC;

    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    for (auto it = this->file_->nameToClass.begin(); it != this->file_->nameToClass.end();) {
        std::visit(
            [&](auto *object) {
                const auto name = object->GetArkTSImpl()->impl.GetStaticClass()->name;
                if (name != it->first) {
                    LIBABCKIT_LOG_NO_FUNC(DEBUG)
                        << funcName << "refresh file class: " << it->first << " --> " << name << std::endl;
                    auto node = this->file_->nameToClass.extract(it++);
                    node.key() = name;
                    this->file_->nameToClass.insert(std::move(node));
                } else {
                    ++it;
                }
            },
            it->second);
    }
}

void libabckit::InstModifier::Modify()
{
    LIBABCKIT_LOG_FUNC;

    ASSERT(this->file_ != nullptr);

    this->EnumerateInst([this](ark::pandasm::Ins &ins) {
        this->ModifyInstFunction(ins);
        this->ModifyInstField(ins);
        this->ModifyInstClass(ins);
    });

    RefreshFunctions(this->file_->nameToFunctionStatic);
    RefreshFunctions(this->file_->nameToFunctionInstance);

    this->RefreshFields();
    this->RefreshClasses();
}