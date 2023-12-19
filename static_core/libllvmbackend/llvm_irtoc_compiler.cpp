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

#include "compiler/code_info/code_info.h"
#include "compiler/optimizer/code_generator/relocations.h"
#include "events/events.h"
#include "optimizer/ir/graph.h"
#include "runtime/include/method.h"
#include "compiler_options.h"
#include "target_machine_builder.h"

#include "llvm_irtoc_compiler.h"
#include "llvm_logger.h"
#include "llvm_options.h"
#include "mir_compiler.h"

#include "lowering/llvm_ir_constructor.h"
#include "transforms/passes/inline_ir/patch_return_handler_stack_adjustment.h"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/Passes.h>
// Suppress warning about forward Pass declaration defined in another namespace
#include <llvm/Pass.h>
#include <llvm/Support/FileSystem.h>

using panda::compiler::LLVMIrConstructor;

namespace panda::llvmbackend {

std::unique_ptr<IrtocCompilerInterface> CreateLLVMIrtocCompiler(panda::compiler::RuntimeInterface *runtime,
                                                                panda::ArenaAllocator *allocator, panda::Arch arch)
{
    return std::make_unique<LLVMIrtocCompiler>(runtime, allocator, arch, "irtoc_file_name.hack");
}

LLVMIrtocCompiler::LLVMIrtocCompiler(panda::compiler::RuntimeInterface *runtime, panda::ArenaAllocator *allocator,
                                     panda::Arch arch, std::string filename)
    : LLVMCompiler(arch),
      methods_(allocator->Adapter()),
      filename_(std::move(filename)),
      ark_interface_(runtime, GetTripleForArch(arch))
{
    InitializeSpecificLLVMOptions(arch);
    auto llvm_compiler_options = InitializeLLVMCompilerOptions();

    target_machine_ = cantFail(panda::llvmbackend::TargetMachineBuilder {}
                                   .SetCPU(GetCPUForArch(arch))
                                   .SetOptLevel(static_cast<llvm::CodeGenOpt::Level>(llvm_compiler_options.optlevel))
                                   .SetFeatures(GetFeaturesForArch(GetArch()))
                                   .SetTriple(GetTripleForArch(GetArch()))
                                   .Build());
    mir_compiler_ = std::make_unique<MIRCompiler>(
        target_machine_, [this](panda::llvmbackend::InsertingPassManager *manager) -> void {
            manager->InsertBefore(&llvm::FEntryInserterID,
                                  panda::llvmbackend::CreatePatchReturnHandlerStackAdjustmentPass(&ark_interface_));
        });
    optimizer_ =
        std::make_unique<panda::llvmbackend::LLVMOptimizer>(llvm_compiler_options, mir_compiler_->GetTargetMachine());
    InitializeModule();

    debug_data_ = std::make_unique<DebugDataBuilder>(module_.get(), filename_);
}

std::vector<std::string> LLVMIrtocCompiler::GetFeaturesForArch(Arch arch)
{
    if (arch == Arch::X86_64 && panda::compiler::OPTIONS.IsCpuFeatureEnabled(compiler::SSE42)) {
        return {std::string("+sse4.2")};
    }
    return {};
}

bool LLVMIrtocCompiler::AddGraph(compiler::Graph *graph)
{
    ASSERT(graph != nullptr);
    ASSERT(graph->GetArch() == GetArch());
    ASSERT(!graph->SupportManagedCode());
    auto method = graph->GetMethod();

    LLVMIrConstructor ctor(graph, module_.get(), GetLLVMContext(), &ark_interface_, debug_data_);
    auto llvm_function = ctor.GetFunc();
    ASSERT(graph->GetMode().IsInterpreter());
    llvm_function->setCallingConv(llvm::CallingConv::ArkInt);

    bool built_ir = ctor.BuildIr();
    if (!built_ir) {
        ir_failed_ = true;
        LLVM_LOG(ERROR, INFRA) << "LLVM failed to build IR for method " << ark_interface_.GetUniqMethodName(method);
        llvm_function->deleteBody();
        return false;
    }

    LLVM_LOG(DEBUG, INFRA) << "LLVM built LLVM IR for method  " << ark_interface_.GetUniqMethodName(method);
    methods_.push_back(static_cast<Method *>(method));
    return true;
}

Expected<bool, std::string> LLVMIrtocCompiler::CanCompile(panda::compiler::Graph *graph)
{
    LLVM_LOG(DEBUG, INFRA) << "LLVM checking graph for method " << ark_interface_.GetUniqMethodName(graph->GetMethod());
    return LLVMIrConstructor::CanCompile(graph);
}

void LLVMIrtocCompiler::CompileAll()
{
    // Compile even if there are no methods because we have to produce an object file, even an empty one
    ASSERT_PRINT(!HasCompiledCode(), "Cannot compile twice");

    optimizer_->OptimizeModule(module_.get());
    debug_data_->Finalize();
    object_file_ = exit_on_err_(mir_compiler_->CompileModule(*module_));
}

void LLVMIrtocCompiler::InitializeSpecificLLVMOptions(Arch arch)
{
    if (arch == Arch::X86_64) {
        SetLLVMOption("x86-use-base-pointer", false);
    }
    if (arch == Arch::AARCH64) {
        SetLLVMOption("aarch64-enable-ptr32", true);
    }
}

void LLVMIrtocCompiler::InitializeModule()
{
    auto layout = target_machine_->createDataLayout();
    module_ = std::make_unique<llvm::Module>("irtoc empty module", *GetLLVMContext());
    module_->setDataLayout(layout);
    module_->setTargetTriple(GetTripleForArch(GetArch()).getTriple());
}

void LLVMIrtocCompiler::WriteObjectFile(std::string_view output)
{
    ASSERT_PRINT(HasCompiledCode(), "Call CompileAll first");
    object_file_->WriteTo(output);
}

size_t LLVMIrtocCompiler::GetObjectFileSize()
{
    return object_file_->Size();
}

CompiledCode LLVMIrtocCompiler::GetCompiledCode(std::string_view function_name)
{
    assert(object_file_ != nullptr);
    auto reference = object_file_->GetSectionByFunctionName(std::string {function_name});
    CompiledCode code {};
    code.size = reference.GetSize();
    code.code = reference.GetMemory();
    return code;
}
}  // namespace panda::llvmbackend
