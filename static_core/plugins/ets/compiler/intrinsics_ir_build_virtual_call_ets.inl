/**
* Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_RUNTIME_IS_SAME_REFERENCE: {
    Builder()->BuildStdRuntimeEquals(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_GET:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CHAR_AT: {
    if (!Builder()->TryBuildStringCharAtIntrinsic(bcInst_, ACC_READ)) {
        BuildDefaultVirtualCallIntrinsic(intrinsicId);
    }
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_GET_LENGTH: {
    Builder()->BuildStringLengthIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_LENGTH: {
    Builder()->BuildStringLengthIntrinsic(bcInst_, ACC_READ);
    auto input = Builder()->GetDefinitionAcc();
    auto inst = GetGraph()->CreateInstCast(DataType::FLOAT64, Builder()->GetPc(bcInst_->GetAddress()),
                                           input, DataType::INT32);
    Builder()->UpdateDefinitionAcc(inst);
    Builder()->AddInstruction(inst);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_IS_EMPTY: {
    Builder()->BuildStringIsEmptyIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_GET: {
    Builder()->BuildEscompatArrayGetIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_SET: {
    Builder()->BuildEscompatArraySetIntrinsic(bcInst_, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_INT8_ARRAY_SET_INT:
case RuntimeInterface::IntrinsicId::INTRINSIC_INT8_ARRAY_SET_BYTE: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::INT8, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_INT8_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::INT8, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_INT16_ARRAY_SET_INT:
case RuntimeInterface::IntrinsicId::INTRINSIC_INT16_ARRAY_SET_SHORT: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::INT16, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_INT16_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::INT16, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_INT32_ARRAY_SET_INT: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::INT32, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_INT32_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::INT32, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_BIG_INT64_ARRAY_SET_LONG: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::INT64, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_BIG_INT64_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::INT64, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_FLOAT32_ARRAY_SET_FLOAT: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::FLOAT32, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_FLOAT32_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::FLOAT32, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_FLOAT64_ARRAY_SET_DOUBLE: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::FLOAT64, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_FLOAT64_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::FLOAT64, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT8_CLAMPED_ARRAY_SET_INT: {
    Builder()->BuildUint8ClampedArraySetIntrinsic(bcInst_, DataType::UINT8, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT8_ARRAY_SET_INT: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::UINT8, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT8_ARRAY_GET:
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT8_CLAMPED_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::UINT8, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT16_ARRAY_SET_INT: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::UINT16, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT16_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::UINT16, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT32_ARRAY_SET_INT:
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT32_ARRAY_SET_LONG: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::UINT32, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT32_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::UINT32, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_BIG_U_INT64_ARRAY_SET_INT:
case RuntimeInterface::IntrinsicId::INTRINSIC_BIG_U_INT64_ARRAY_SET_LONG: {
    Builder()->BuildTypedArraySetIntrinsic(bcInst_, DataType::UINT64, ACC_READ);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_BIG_U_INT64_ARRAY_GET: {
    Builder()->BuildTypedArrayGetIntrinsic(bcInst_, DataType::UINT64, ACC_READ);
    break;
}