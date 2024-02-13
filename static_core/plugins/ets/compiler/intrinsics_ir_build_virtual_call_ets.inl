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
    BuildStdRuntimeEquals(bcInst, accRead);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CHAR_AT: {
    if (!TryBuildStringCharAtIntrinsic(bcInst, accRead)) {
        BuildDefaultVirtualCallIntrinsic(bcInst, isRange, accRead);
    }
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_GET_LENGTH: {
    BuildStringLengthIntrinsic(bcInst, accRead);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_LENGTH: {
    BuildStringLengthIntrinsic(bcInst, accRead);
    auto input = GetDefinitionAcc();
    auto inst = graph_->CreateInstCast(DataType::FLOAT64, GetPc(bcInst->GetAddress()), input, DataType::INT32);
    UpdateDefinitionAcc(inst);
    AddInstruction(inst);
    break;
}
case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_IS_EMPTY: {
    BuildStringIsEmptyIntrinsic(bcInst, accRead);
    break;
}
