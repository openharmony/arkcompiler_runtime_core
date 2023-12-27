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

void GetInfoForInteropCallArgsConversion(
    MethodPtr methodPtr, ArenaVector<std::pair<IntrinsicId, compiler::DataType::Type>> *intrinsics) const override
{
    auto method = MethodCast(methodPtr);
    uint32_t numArgs = method->GetNumArgs() - 2U;
    panda_file::ShortyIterator it(method->GetShorty());
    it.IncrementWithoutCheck();
    it.IncrementWithoutCheck();
    it.IncrementWithoutCheck();
    for (uint32_t argIdx = 0; argIdx < numArgs; ++argIdx, it.IncrementWithoutCheck()) {
        auto type = *it;
        std::pair<IntrinsicId, compiler::DataType::Type> pair = {};
        switch (type.GetId()) {
            case panda_file::Type::TypeId::VOID:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_VOID_TO_LOCAL, compiler::DataType::NO_TYPE};
                break;
            case panda_file::Type::TypeId::U1:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U1_TO_LOCAL, compiler::DataType::BOOL};
                break;
            case panda_file::Type::TypeId::I8:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I8_TO_LOCAL, compiler::DataType::INT8};
                break;
            case panda_file::Type::TypeId::U8:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U8_TO_LOCAL, compiler::DataType::UINT8};
                break;
            case panda_file::Type::TypeId::I16:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I16_TO_LOCAL, compiler::DataType::INT16};
                break;
            case panda_file::Type::TypeId::U16:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U16_TO_LOCAL, compiler::DataType::UINT16};
                break;
            case panda_file::Type::TypeId::I32:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I32_TO_LOCAL, compiler::DataType::INT32};
                break;
            case panda_file::Type::TypeId::U32:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U32_TO_LOCAL, compiler::DataType::UINT32};
                break;
            case panda_file::Type::TypeId::I64:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I64_TO_LOCAL, compiler::DataType::INT64};
                break;
            case panda_file::Type::TypeId::U64:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U64_TO_LOCAL, compiler::DataType::UINT64};
                break;
            case panda_file::Type::TypeId::F32:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_F32_TO_LOCAL, compiler::DataType::FLOAT32};
                break;
            case panda_file::Type::TypeId::F64:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_F64_TO_LOCAL, compiler::DataType::FLOAT64};
                break;
            case panda_file::Type::TypeId::REFERENCE:
                pair = {IntrinsicId::INTRINSIC_COMPILER_CONVERT_REF_TYPE_TO_LOCAL, compiler::DataType::REFERENCE};
                break;
            default: {
                UNREACHABLE();
            }
        }
        intrinsics->push_back(pair);
    }
}

std::optional<std::pair<compiler::RuntimeInterface::IntrinsicId, compiler::DataType::Type>>
GetInfoForInteropCallRetValueConversion(MethodPtr methodPtr) const override
{
    auto method = MethodCast(methodPtr);
    auto type = method->GetReturnType();
    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return {};
        case panda_file::Type::TypeId::U1:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U1, compiler::DataType::BOOL}};
        case panda_file::Type::TypeId::I8:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I8, compiler::DataType::INT8}};
        case panda_file::Type::TypeId::U8:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U8, compiler::DataType::UINT8}};
        case panda_file::Type::TypeId::I16:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I16, compiler::DataType::INT16}};
        case panda_file::Type::TypeId::U16:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U16, compiler::DataType::UINT16}};
        case panda_file::Type::TypeId::I32:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I32, compiler::DataType::INT32}};
        case panda_file::Type::TypeId::U32:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U32, compiler::DataType::UINT32}};
        case panda_file::Type::TypeId::I64:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I64, compiler::DataType::INT64}};
        case panda_file::Type::TypeId::U64:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U64, compiler::DataType::UINT64}};
        case panda_file::Type::TypeId::F32:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_F32, compiler::DataType::FLOAT32}};
        case panda_file::Type::TypeId::F64:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_F64, compiler::DataType::FLOAT64}};
        case panda_file::Type::TypeId::REFERENCE: {
            auto vm = reinterpret_cast<PandaEtsVM *>(Thread::GetCurrent()->GetVM());
            auto classLinker = vm->GetClassLinker();
            auto klass = GetClass(method, GetMethodReturnTypeId(method));
            // start fastpath
            if (klass ==
                classLinker->GetClass(panda_file_items::class_descriptors::JS_VALUE.data())->GetRuntimeClass()) {
                return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_JS_VALUE, compiler::DataType::REFERENCE}};
            }
            if (klass == classLinker->GetClass(panda_file_items::class_descriptors::STRING.data())->GetRuntimeClass()) {
                return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_STRING, compiler::DataType::REFERENCE}};
            }
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_REF_TYPE, compiler::DataType::REFERENCE}};
        }
        default: {
            UNREACHABLE();
        }
    }
    return {};
}
