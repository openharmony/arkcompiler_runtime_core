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

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_NAN:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_NAN: {
    BuildIsNanIntrinsic(bcInst, accRead);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_FINITE:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_FINITE: {
    /* Inlining makes sense for 64-bits archs as for 32-bits archs
     * the implementation has to be made much more complicated to
     * deal with 64-bits data (double type), so, for for 32-bit archs
     * (e.g. ARM32) it is just better to use c++ implementation */
    if (Is64BitsArch(GetGraph()->GetArch())) {
        BuildIsFiniteIntrinsic(bcInst, accRead);
    } else {
        BuildDefaultStaticIntrinsic(bcInst, false, accRead);
    }
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS: {
    BuildAbsIntrinsic(bcInst, accRead);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SQRT: {
    BuildSqrtIntrinsic(bcInst, accRead);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SIGNBIT: {
    BuildSignbitIntrinsic(bcInst, accRead);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F64: {
    BuildBinaryOperationIntrinsic<Opcode::Max>(bcInst, accRead);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I64:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F32:
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F64: {
    BuildBinaryOperationIntrinsic<Opcode::Min>(bcInst, accRead);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MOD: {
    BuildBinaryOperationIntrinsic<Opcode::Mod>(bcInst, accRead);
    break;
}

case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_IS_UPPER_CASE: {
    BuildCharIsUpperCaseIntrinsic(bcInst, accRead);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_UPPER_CASE: {
    BuildCharToUpperCaseIntrinsic(bcInst, accRead);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_IS_LOWER_CASE: {
    BuildCharIsLowerCaseIntrinsic(bcInst, accRead);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_LOWER_CASE: {
    BuildCharToLowerCaseIntrinsic(bcInst, accRead);
    break;
}
