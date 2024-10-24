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

#include "libabckit/src/wrappers/pandasm_wrapper.h"
#include "libabckit/src/ir_impl.h"

#include "assembler/assembly-function.h"
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-literals.h"
#include "assembler/mangling.h"

#include <generated/opc_to_string.h>

namespace libabckit {
// CC-OFFNXT(WordsTool.95 Google) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda;

std::string InsWrapper::opcodeInvalid_ = "INVALID";

void FunctionWrapper::Fill()
{
    auto *func = reinterpret_cast<pandasm::Function *>(impl);
    name = func->name;
    regsNum = func->regs_num;
    valueOfFirstParam = func->value_of_first_param;
    for (const auto &cb : func->catch_blocks) {
        auto catchBlockWrap =
            FunctionWrapper::CatchBlockWrapper(cb.whole_line, cb.exception_record, cb.try_begin_label, cb.try_end_label,
                                               cb.catch_begin_label, cb.catch_end_label);
        catchBlocks.push_back(catchBlockWrap);
    }
}

void FunctionWrapper::Update()
{
    auto *func = reinterpret_cast<pandasm::Function *>(impl);
    func->ins.clear();
    func->catch_blocks.clear();
    func->name = name;
    func->regs_num = regsNum;
    func->value_of_first_param = valueOfFirstParam;
    func->slots_num = slotsNum;
    for (auto insWrap : ins) {
        auto insDebug = pandasm::debuginfo::Ins(insWrap.insDebug.lineNumber, insWrap.insDebug.wholeLine,
                                                insWrap.insDebug.boundLeft, insWrap.insDebug.boundRight);
        auto insn = pandasm::Ins();
        insn.opcode = OpcFromName(insWrap.opcode);
        insn.regs = std::move(insWrap.regs);
        insn.ids = std::move(insWrap.ids);
        insn.imms = std::move(insWrap.imms);
        insn.label = std::move(insWrap.label);
        insn.set_label = insWrap.setLabel;
        insn.ins_debug = insDebug;
        func->ins.push_back(insn);
    }
    for (auto cbWrap : catchBlocks) {
        auto catchBlock = pandasm::Function::CatchBlock();
        catchBlock.whole_line = std::move(cbWrap.wholeLine);
        catchBlock.exception_record = std::move(cbWrap.exceptionRecord);
        catchBlock.try_begin_label = std::move(cbWrap.tryBeginLabel);
        catchBlock.try_end_label = std::move(cbWrap.tryEndLabel);
        catchBlock.catch_begin_label = std::move(cbWrap.catchBeginLabel);
        catchBlock.catch_end_label = std::move(cbWrap.catchEndLabel);
        func->catch_blocks.push_back(catchBlock);
    }
}

FunctionWrapper *PandasmWrapper::CreateWrappedFunction()
{
    auto *newFunc = new panda::pandasm::Function("newCode", panda::panda_file::SourceLang::ECMASCRIPT);
    return GetWrappedFunction(newFunc);
}

std::map<std::string, void *> PandasmWrapper::GetUnwrappedLiteralArrayTable(void *program)
{
    auto *prog = reinterpret_cast<pandasm::Program *>(program);
    std::map<std::string, void *> res;
    for (auto &[id, s] : prog->literalarray_table) {
        res[id] = reinterpret_cast<void *>(&s);
    }
    return res;
}

}  // namespace libabckit
