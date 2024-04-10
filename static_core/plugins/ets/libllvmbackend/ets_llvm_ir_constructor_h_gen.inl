/*
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

bool EmitArrayCopyTo(Inst *inst);
bool EmitStdStringSubstring(Inst *inst);
bool EmitStringBuilderAppendBool(Inst *inst);
bool EmitStringBuilderAppendChar(Inst *inst);
bool EmitStringBuilderAppendByte(Inst *inst);
bool EmitStringBuilderAppendShort(Inst *inst);
bool EmitStringBuilderAppendInt(Inst *inst);
bool EmitStringBuilderAppendLong(Inst *inst);
bool EmitStringBuilderAppendString(Inst *inst);

llvm::Value *CreateStringBuilderAppendLong(Inst *inst);
llvm::Value *CreateStringBuilderAppendString(Inst *inst);

void StringBuilderAppendStringMain(Inst *inst, llvm::Value *sbIndexOffset, llvm::Value *sbIndex,
                                   llvm::Value *sbBuffer, llvm::BasicBlock *contBb);
void StringBuilderAppendStringNull(Inst *inst, llvm::PHINode *result, llvm::BasicBlock *contBb);
