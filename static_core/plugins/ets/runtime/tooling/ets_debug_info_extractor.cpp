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

#include "ets_debug_info_extractor.h"

namespace ark::ets::tooling {
// TODO: implement pandasm -> EtsType
static panda_file::Type::TypeId GetTypeIdBySignature(char signature)
{
    switch (signature) {
        case 'V':
            return panda_file::Type::TypeId::VOID;
        case 'Z':
            return panda_file::Type::TypeId::U1;
        case 'B':
            return panda_file::Type::TypeId::I8;
        case 'H':
            return panda_file::Type::TypeId::U8;
        case 'S':
            return panda_file::Type::TypeId::I16;
        case 'C':
            return panda_file::Type::TypeId::U16;
        case 'I':
            return panda_file::Type::TypeId::I32;
        case 'U':
            return panda_file::Type::TypeId::U32;
        case 'F':
            return panda_file::Type::TypeId::F32;
        case 'D':
            return panda_file::Type::TypeId::F64;
        case 'J':
            return panda_file::Type::TypeId::I64;
        case 'Q':
            return panda_file::Type::TypeId::U64;
        case 'A':
            return panda_file::Type::TypeId::TAGGED;
        case 'L':
        case '[':
            return panda_file::Type::TypeId::REFERENCE;
        default:
            return panda_file::Type::TypeId::INVALID;
    }
}

// NOLINTBEGIN(readability-magic-numbers)
static TypedValue CreateTypedValueFromReg(uint64_t reg, panda_file::Type::TypeId type)
{
    switch (type) {
        case panda_file::Type::TypeId::INVALID:
            return TypedValue::Invalid();
        case panda_file::Type::TypeId::VOID:
            return TypedValue::Void();
        case panda_file::Type::TypeId::U1:
            return TypedValue::U1((reg & 0x1ULL) != 0);
        case panda_file::Type::TypeId::I8:
            return TypedValue::I8(reg & 0xffULL);
        case panda_file::Type::TypeId::U8:
            return TypedValue::U8(reg & 0xffULL);
        case panda_file::Type::TypeId::I16:
            return TypedValue::I16(reg & 0xffffULL);
        case panda_file::Type::TypeId::U16:
            return TypedValue::U16(reg & 0xffffULL);
        case panda_file::Type::TypeId::I32:
            return TypedValue::I32(reg & 0xffffffffULL);
        case panda_file::Type::TypeId::U32:
            return TypedValue::U32(reg & 0xffffffffULL);
        case panda_file::Type::TypeId::F32:
            return TypedValue::F32(bit_cast<float>(static_cast<int32_t>(reg & 0xffffffffULL)));
        case panda_file::Type::TypeId::F64:
            return TypedValue::F64(bit_cast<double>(reg & 0xffffffffULL));
        case panda_file::Type::TypeId::I64:
            return TypedValue::I64(reg);
        case panda_file::Type::TypeId::U64:
            return TypedValue::U64(reg);
        case panda_file::Type::TypeId::REFERENCE:
            return TypedValue::Reference(reinterpret_cast<ObjectHeader *>(reg));
        case panda_file::Type::TypeId::TAGGED:
            return TypedValue::Tagged(coretypes::TaggedValue(static_cast<coretypes::TaggedType>(reg)));
        default:
            UNREACHABLE();
            return TypedValue::Invalid();
    }
}
// NOLINTEND(readability-magic-numbers)

static TypedValue CreateTypedValueFromReg(const std::string &signature, uint64_t reg, tFrame::RegisterKind kind)
{
    auto type = signature.empty() ? panda_file::Type::TypeId::INVALID : GetTypeIdBySignature(signature[0]);
    if (type == panda_file::Type::TypeId::INVALID) {
        switch (kind) {
            case PtFrame::RegisterKind::PRIMITIVE:
                type = panda_file::Type::TypeId::U64;
                break;
            case PtFrame::RegisterKind::REFERENCE:
                type = panda_file::Type::TypeId::REFERENCE;
                break;
            case PtFrame::RegisterKind::TAGGED:
                type = panda_file::Type::TypeId::TAGGED;
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    return CreateTypedValueFromReg(reg, type);
}

const panda_file::LocalVariableTable &EtsDebugInfoExtractor::GetLocalsWithArguments(Frame *frame)
{
    ASSERT(frame);
    auto *method = frame->GetMethod();
    ASSERT(method);

    auto iter = localsCache_.find(method);
    if (iter != localsCache_.end()) {
        return iter->second;
    }

    auto methodId = method->GetFileId();
    auto numVregs = method->GetNumVregs();
    auto codeSize = method->GetCodeSize();
    auto frameOffset = frame->GetBytecodeOffset();
    const auto &parameters = GetParameterInfo(methodId);
    const auto &localVariableTable = GetLocalVariableTable(methodId);

    // TODO: verify that implicit `this` parameter is added
    for (auto i = 0U; i < parameters.size(); i++) {
        auto &parameter = parameters[i];
        localVariableTable.emplace_back(parameter.name, parameter.signature, parameter.signature, numVregs + i, 0,
                                        codeSize);
    }

    return localsCache_.emplace(method, std::move(localVariableTable)).first->second;
}
}  // namespace ark::ets::tooling
