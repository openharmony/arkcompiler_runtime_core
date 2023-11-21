/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_PAOC_LLVM_H
#define PANDA_PAOC_LLVM_H

#include "llvm_compiler_creator.h"
#include "llvmaot_options.h"
#include "paoc.h"

namespace panda::compiler {
class LLVMAotBuilder;
}  // namespace panda::compiler

namespace panda::paoc {

class PaocLLVM : public Paoc {
protected:
    void AddExtraOptions(PandArgParser *parser) override
    {
        panda::llvmaot::OPTIONS.AddOptions(parser);
    }
    void ValidateExtraOptions() override;
    void Clear(panda::mem::InternalAllocatorPtr allocator) override;
    LLVMCompilerStatus TryLLVM(CompilingContext *ctx) override;
    void PrepareLLVM(const panda::Span<const char *> &args) override;
    bool EndLLVM() override;

    std::unique_ptr<compiler::AotBuilder> CreateAotBuilder() override;

    compiler::LLVMAotBuilder *GetAotBuilder();

private:
    bool AddGraphToLLVM(CompilingContext *ctx);

private:
    std::unique_ptr<compiler::AotCompilerInterface> llvm_aot_compiler_ {nullptr};
};

}  // namespace panda::paoc
#endif  // PANDA_PAOC_LLVM_H
