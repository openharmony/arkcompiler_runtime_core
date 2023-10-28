/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef LIBLLVMBACKEND_TRANSFORMS_LLVM_OPTIMIZER_H
#define LIBLLVMBACKEND_TRANSFORMS_LLVM_OPTIMIZER_H

#include "llvm_compiler_options.h"

#include <memory>

namespace llvm {
class Module;
class TargetMachine;
}  // namespace llvm

namespace panda::llvmbackend {
class LLVMArkInterface;
}  // namespace panda::llvmbackend

namespace panda::llvmbackend {
class LLVMOptimizer {
public:
    void OptimizeModule(llvm::Module *module) const;

    explicit LLVMOptimizer(panda::llvmbackend::LLVMCompilerOptions options,
                           std::shared_ptr<llvm::TargetMachine> target_machine);

private:
    void DoOptimizeModule(llvm::Module *module) const;

private:
    panda::llvmbackend::LLVMCompilerOptions options_;
    std::shared_ptr<llvm::TargetMachine> target_machine_;
};

}  // namespace panda::llvmbackend

#endif  // LIBLLVMBACKEND_TRANSFORMS_LLVM_OPTIMIZER_H
