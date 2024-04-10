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

#include "compiler/aot/compiled_method.h"
#include "compiler/code_info/code_info.h"
#include "events/events.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/checks_elimination.h"
#include "optimizer/optimizations/if_merging.h"
#include "optimizer/optimizations/inlining.h"
#include "optimizer/optimizations/memory_barriers.h"
#include "optimizer/optimizations/object_type_check_elimination.h"
#include "optimizer/optimizations/peepholes.h"
#include "optimizer/optimizations/simplify_string_builder.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/analysis/monitor_analysis.h"
#include "optimizer/optimizations/cleanup.h"
#include "runtime/include/method.h"
#include "runtime/include/thread.h"
#include "compiler_options.h"

#include "llvm_aot_compiler.h"
#include "llvm_logger.h"
#include "llvm_options.h"
#include "mir_compiler.h"
#include "target_machine_builder.h"

#include "lowering/llvm_ir_constructor.h"
#include "object_code/code_info_producer.h"
#include "transforms/passes/ark_frame_lowering/frame_lowering.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
// Suppress warning about forward Pass declaration defined in another namespace
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/ThreadPool.h>

/* Be careful this file also includes elf.h, which has a lot of defines */
#include "aot/aot_builder/llvm_aot_builder.h"

#define DEBUG_TYPE "llvm-aot-compiler"

static constexpr unsigned LIMIT_MULTIPLIER = 2U;

static constexpr int32_t MAX_DEPTH = 32;

using ark::compiler::LLVMIrConstructor;

namespace {

void MarkAsInlineObject(llvm::GlobalObject *globalObject)
{
    globalObject->addMetadata(ark::llvmbackend::LLVMArkInterface::FUNCTION_MD_INLINE_MODULE,
                              *llvm::MDNode::get(globalObject->getContext(), {}));
}

void UnmarkAsInlineObject(llvm::GlobalObject *globalObject)
{
    ASSERT_PRINT(globalObject->hasMetadata(ark::llvmbackend::LLVMArkInterface::FUNCTION_MD_INLINE_MODULE),
                 "Must be marked to unmark");

    globalObject->setMetadata(ark::llvmbackend::LLVMArkInterface::FUNCTION_MD_INLINE_MODULE, nullptr);
}

constexpr unsigned GetFrameSlotSize(ark::Arch arch)
{
    constexpr size_t SPILLS = 0;
    ark::CFrameLayout layout {arch, SPILLS};
    return layout.GetSlotSize();
}

}  // namespace

namespace ark::llvmbackend {

class FixedCountSpreader : public Spreader {
public:
    using MethodCount = uint64_t;
    using ModuleFactory = std::function<std::shared_ptr<WrappedModule>(uint32_t moduleId)>;

    explicit FixedCountSpreader(compiler::RuntimeInterface *runtime, MethodCount limit, ModuleFactory moduleFactory)
        : moduleFactory_(std::move(moduleFactory)), runtime_(runtime), limit_(limit)
    {
        // To avoid overflow in HardLimit()
        ASSERT(limit_ < std::numeric_limits<MethodCount>::max() / LIMIT_MULTIPLIER);
        ASSERT(limit_ > 0);
    }

    std::shared_ptr<WrappedModule> GetModuleForMethod(compiler::RuntimeInterface::MethodPtr method) override
    {
        auto &module = modules_[method];
        if (module != nullptr) {
            return module;
        }
        ASSERT(!IsOrderViolated(method));
        if (currentModule_ == nullptr) {
            currentModule_ = moduleFactory_(moduleCount_++);
        }
        if (currentModuleSize_ + 1 >= limit_) {
            bool sizeExceeds = currentModuleSize_ + 1 >= HardLimit();
            if (sizeExceeds || runtime_->GetClass(method) != lastClass_) {
                LLVM_LOG(DEBUG, INFRA) << "Creating new module, since "
                                       << (sizeExceeds ? "the module's size would exceed HardLimit" : "class changed")
                                       << ", class of current method = '" << runtime_->GetClassNameFromMethod(method)
                                       << "'"
                                       << ", lastClass_ = '"
                                       << (lastClass_ == nullptr ? "nullptr" : runtime_->GetClassName(lastClass_))
                                       << "', limit_ = " << limit_ << ", HardLimit = " << HardLimit()
                                       << ", currentModuleSize_ = " << currentModuleSize_;
                currentModuleSize_ = 0;
                currentModule_ = moduleFactory_(moduleCount_++);
                moduleCount_++;
            }
        }

        currentModuleSize_++;
        lastClass_ = runtime_->GetClass(method);
        module = currentModule_;
        LLVM_LOG(DEBUG, INFRA) << "Getting module for graph (" << runtime_->GetMethodFullName(method, true) << "), "
                               << currentModuleSize_ << " / " << limit_ << ", HardLimit = " << HardLimit();

        ASSERT(module != nullptr);
        ASSERT(!module->IsCompiled());
        ASSERT(currentModuleSize_ < HardLimit());
        return module;
    }

    std::unordered_set<std::shared_ptr<WrappedModule>> GetModules() override
    {
        std::unordered_set<std::shared_ptr<WrappedModule>> modules;
        for (auto &entry : modules_) {
            modules.insert(entry.second);
        }
        return modules;
    }

private:
    constexpr uint64_t HardLimit()
    {
        return limit_ * LIMIT_MULTIPLIER;
    }
#ifndef NDEBUG
    bool IsOrderViolated(compiler::RuntimeInterface::MethodPtr method)
    {
        auto clazz = runtime_->GetClass(method);
        auto notSeen = seenClasses_.insert(clazz).second;
        if (notSeen) {
            return false;
        }
        return clazz != lastClass_;
    }
#endif

private:
#ifndef NDEBUG
    std::unordered_set<compiler::RuntimeInterface::ClassPtr> seenClasses_;
#endif
    ModuleFactory moduleFactory_;
    compiler::RuntimeInterface *runtime_;
    const uint64_t limit_;
    uint64_t currentModuleSize_ {0};
    uint32_t moduleCount_ {0};
    compiler::RuntimeInterface::ClassPtr lastClass_ {nullptr};
    std::shared_ptr<WrappedModule> currentModule_ {nullptr};
    std::unordered_map<compiler::RuntimeInterface::MethodPtr, std::shared_ptr<WrappedModule>> modules_;
};

bool LLVMAotCompiler::AddGraph(compiler::Graph *graph)
{
    ASSERT(graph != nullptr);
    ASSERT(graph->GetArch() == GetArch());
    ASSERT(graph->GetAotData()->GetUseCha());
    ASSERT(graph->GetRuntime()->IsCompressedStringsEnabled());
    ASSERT(graph->SupportManagedCode());
    auto method = static_cast<Method *>(graph->GetMethod());
    auto module = spreader_->GetModuleForMethod(method);
    bool result = AddGraphToModule(graph, *module, AddGraphMode::PRIMARY_FUNCTION);
    module->AddMethod(method);
    methods_.push_back(method);
    AddInlineFunctionsByDepth(*module, graph, 0);
    return result;
}

void LLVMAotCompiler::RunArkPasses(ark::compiler::Graph *graph)
{
    graph->RunPass<compiler::Cleanup>(false);
    // ==== Partial Ark Compiler Pipeline =======
    graph->RunPass<compiler::Peepholes>();
    graph->RunPass<compiler::BranchElimination>();
    graph->RunPass<compiler::SimplifyStringBuilder>();
    graph->RunPass<compiler::Cleanup>();
    graph->RunPass<compiler::CatchInputs>();
    graph->RunPass<compiler::TryCatchResolving>();
    graph->RunPass<compiler::Peepholes>();
    graph->RunPass<compiler::BranchElimination>();
    graph->RunPass<compiler::ValNum>();
    graph->RunPass<compiler::IfMerging>();
    graph->RunPass<compiler::Cleanup>(false);
    graph->RunPass<compiler::Peepholes>();
    // ==========================================
    graph->RunPass<compiler::ChecksElimination>();
    graph->RunPass<compiler::Inlining>(true);
    graph->RunPass<compiler::Cleanup>(false);
#ifndef NDEBUG
    if (ark::compiler::g_options.IsCompilerCheckGraph() && ark::compiler::g_options.IsCompilerCheckFinal()) {
        ark::compiler::GraphChecker(graph, "LLVM").Check();
    }
#endif
}

bool LLVMAotCompiler::AddGraphToModule(ark::compiler::Graph *graph, WrappedModule &module, AddGraphMode addGraphMode)
{
    auto method = graph->GetMethod();
    if (module.HasFunctionDefinition(method) && addGraphMode == AddGraphMode::PRIMARY_FUNCTION) {
        auto function = module.GetFunctionByMethodPtr(method);
        UnmarkAsInlineObject(function);
        return true;
    }
    LLVM_LOG(DEBUG, INFRA) << "Adding graph for method = '" << runtime_->GetMethodFullName(graph->GetMethod()) << "'";
    RunArkPasses(graph);
    LLVMIrConstructor ctor(graph, module.GetModule().get(), module.GetLLVMContext().get(),
                           module.GetLLVMArkInterface().get(), module.GetDebugData());
    auto llvmFunction = ctor.GetFunc();
    bool noInline = IsInliningDisabled(graph);

    bool builtIr = ctor.BuildIr(noInline);
    if (!builtIr) {
        if (addGraphMode == AddGraphMode::PRIMARY_FUNCTION) {
            irFailed_ = true;
            LLVM_LOG(ERROR, INFRA) << "LLVM AOT failed to build IR for method '"
                                   << module.GetLLVMArkInterface()->GetUniqMethodName(method) << "'";
        } else {
            ASSERT(addGraphMode == AddGraphMode::INLINE_FUNCTION);
            LLVM_LOG(WARNING, INFRA) << "LLVM AOT failed to build IR for inline function '"
                                     << module.GetLLVMArkInterface()->GetUniqMethodName(method) << "'";
        }
        llvmFunction->deleteBody();
        return false;
    }

    LLVM_LOG(DEBUG, INFRA) << "LLVM AOT built LLVM IR for method  "
                           << module.GetLLVMArkInterface()->GetUniqMethodName(method);
    return true;
}

void LLVMAotCompiler::PrepareAotGot(WrappedModule *wrappedModule)
{
    static constexpr size_t ARRAY_LENGTH = 0;
    auto array64 = llvm::ArrayType::get(llvm::Type::getInt64Ty(*wrappedModule->GetLLVMContext()), ARRAY_LENGTH);

    auto aotGot =
        new llvm::GlobalVariable(*wrappedModule->GetModule(), array64, false, llvm::GlobalValue::ExternalLinkage,
                                 llvm::ConstantAggregateZero::get(array64), "__aot_got");

    aotGot->setSection(AOT_GOT_SECTION);
    aotGot->setVisibility(llvm::GlobalVariable::ProtectedVisibility);
}

WrappedModule LLVMAotCompiler::CreateModule(uint32_t moduleId)
{
    auto ttriple = GetTripleForArch(GetArch());
    auto llvmContext = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("panda-llvmaot-module", *llvmContext);
    auto options = InitializeLLVMCompilerOptions();
    // clang-format off
    auto targetMachine = cantFail(ark::llvmbackend::TargetMachineBuilder {}
                                .SetCPU(GetCPUForArch(GetArch()))
                                .SetOptLevel(static_cast<llvm::CodeGenOpt::Level>(options.optlevel))
                                .SetFeatures(GetFeaturesForArch(GetArch()))
                                .SetTriple(ttriple)
                                .Build());
    // clang-format on
    auto layout = targetMachine->createDataLayout();
    module->setDataLayout(layout);
    module->setTargetTriple(ttriple.getTriple());
    auto debugData = std::make_unique<DebugDataBuilder>(module.get(), filename_);
    auto arkInterface = std::make_unique<LLVMArkInterface>(runtime_, ttriple, aotBuilder_, &lock_);
    arkInterface.get()->CreateRequiredIntrinsicFunctionTypes(*llvmContext.get());
    LLVMIrConstructor::InsertArkFrameInfo(module.get(), GetArch());
    LLVMIrConstructor::ProvideSafepointPoll(module.get(), arkInterface.get(), GetArch());
    WrappedModule wrappedModule {
        std::move(llvmContext),    //
        std::move(module),         //
        std::move(targetMachine),  //
        std::move(arkInterface),   //
        std::move(debugData)       //
    };
    wrappedModule.SetId(moduleId);

    PrepareAotGot(&wrappedModule);
    return wrappedModule;
}

Expected<bool, std::string> LLVMAotCompiler::CanCompile(ark::compiler::Graph *graph)
{
    LLVM_LOG(DEBUG, INFRA) << "LLVM AOT checking graph for method " << runtime_->GetMethodFullName(graph->GetMethod());
    return LLVMIrConstructor::CanCompile(graph);
}

std::unique_ptr<ark::llvmbackend::CompilerInterface> CreateLLVMAotCompiler(ark::compiler::RuntimeInterface *runtime,
                                                                           ark::ArenaAllocator *allocator,
                                                                           ark::compiler::LLVMAotBuilder *aotBuilder,
                                                                           const std::string &cmdline,
                                                                           const std::string &filename)
{
    return std::make_unique<LLVMAotCompiler>(runtime, allocator, aotBuilder, cmdline, filename);
}

LLVMAotCompiler::LLVMAotCompiler(ark::compiler::RuntimeInterface *runtime, ark::ArenaAllocator *allocator,
                                 ark::compiler::LLVMAotBuilder *aotBuilder, std::string cmdline, std::string filename)
    : LLVMCompiler(aotBuilder->GetArch()),
      methods_(allocator->Adapter()),
      aotBuilder_(aotBuilder),
      cmdline_(std::move(cmdline)),
      filename_(std::move(filename)),
      runtime_(runtime)
{
    auto arch = aotBuilder->GetArch();
    llvm::Triple ttriple = GetTripleForArch(arch);
    auto llvmCompilerOptions = InitializeLLVMCompilerOptions();
    SetLLVMOption("spill-slot-min-size-bytes", GetFrameSlotSize(arch));
    // Disable some options of PlaceSafepoints pass
    SetLLVMOption("spp-no-entry", true);
    SetLLVMOption("spp-no-call", true);
    // Set limit to skip Safepoints on entry for small functions
    SetLLVMOption("isp-on-entry-limit", ark::compiler::g_options.GetCompilerSafepointEliminationLimit());
    SetLLVMOption("enable-implicit-null-checks", ark::compiler::g_options.IsCompilerImplicitNullCheck());
    SetLLVMOption("imp-null-check-page-size", static_cast<int>(runtime->GetProtectedMemorySize()));
    if (arch == Arch::AARCH64) {
        SetLLVMOption("aarch64-enable-ptr32", true);
    }
    spreader_ = std::make_unique<FixedCountSpreader>(
        runtime_, g_options.GetLlvmaotMethodsPerModule(),
        [this](uint32_t moduleId) { return std::make_shared<WrappedModule>(CreateModule(moduleId)); });
}

/* static */
std::vector<std::string> LLVMAotCompiler::GetFeaturesForArch(Arch arch)
{
    std::vector<std::string> features;
    features.reserve(2U);
    switch (arch) {
        case Arch::AARCH64: {
            features.emplace_back(std::string("+reserve-x") + std::to_string(ark::GetThreadReg(arch)));
            if (ark::compiler::g_options.IsCpuFeatureEnabled(compiler::CRC32)) {
                features.emplace_back(std::string("+crc"));
            }
            return features;
        }
        case Arch::X86_64:
            features.emplace_back(std::string("+fixed-r") + std::to_string(ark::GetThreadReg(arch)));
            if (ark::compiler::g_options.IsCpuFeatureEnabled(compiler::SSE42)) {
                features.emplace_back("+sse4.2");
            }
            return features;
        default:
            return {};
    }
}

AotBuilderOffsets LLVMAotCompiler::CollectAotBuilderOffsets(
    const std::unordered_set<std::shared_ptr<WrappedModule>> &modules)
{
    using StackMapSymbol = ark::llvmbackend::CreatedObjectFile::StackMapSymbol;

    const auto &methodPtrs = aotBuilder_->GetMethods();
    const auto &headers = aotBuilder_->GetMethodHeaders();
    auto methodsIt = std::begin(methods_);
    for (size_t i = 0; i < headers.size(); ++i) {
        if (methodPtrs[i].GetMethod() != *methodsIt) {
            continue;
        }

        auto module = spreader_->GetModuleForMethod(*methodsIt);
        auto fullMethodName = module->GetLLVMArkInterface()->GetUniqMethodName(*methodsIt);
        std::unordered_map<std::string, StackMapSymbol> stackmapInfo;

        auto section = module->GetObjectFile()->GetSectionByFunctionName(fullMethodName);
        auto compiledMethod = AdaptCode(*methodsIt, {section.GetMemory(), section.GetSize()});
        aotBuilder_->AdjustMethodHeader(compiledMethod, i);
        EVENT_PAOC("Compiling " + runtime_->GetMethodFullName(*methodsIt) + " using llvm");
        methodsIt++;
    }
    ASSERT(methodsIt == std::end(methods_));

    std::vector<ark::compiler::RoData> roDatas;
    for (auto &module : modules) {
        auto roDataSections = module->GetObjectFile()->GetRoDataSections();
        for (auto &item : roDataSections) {
            roDatas.push_back(ark::compiler::RoData {
                item.ContentToVector(), item.GetName() + std::to_string(module->GetModuleId()), item.GetAlignment()});
        }
    }
    std::sort(roDatas.begin(), roDatas.end(),
              [](const compiler::RoData &a, const compiler::RoData &b) -> bool { return a.name < b.name; });
    aotBuilder_->SetRoDataSections(roDatas);

    // All necessary information is supplied to the aotBuilder.
    // And it can return the offsets of sections and methods where they will be located.
    auto sectionAddresses = aotBuilder_->GetSectionsAddresses(cmdline_, filename_);
    for (const auto &item : sectionAddresses) {
        LLVM_LOG(DEBUG, INFRA) << item.first << " starts at " << item.second;
    }

    std::unordered_map<std::string, size_t> methodOffsets;
    for (size_t i = 0; i < headers.size(); ++i) {
        auto method =
            const_cast<compiler::RuntimeInterface::MethodPtr>(static_cast<const void *>(methodPtrs[i].GetMethod()));
        std::string methodName =
            spreader_->GetModuleForMethod(method)->GetLLVMArkInterface()->GetUniqMethodName(methodPtrs[i].GetMethod());
        methodOffsets[methodName] = headers[i].codeOffset;
    }
    return AotBuilderOffsets {std::move(sectionAddresses), std::move(methodOffsets)};
}

ark::compiler::CompiledMethod LLVMAotCompiler::AdaptCode(ark::Method *method, Span<const uint8_t> machineCode)
{
    ASSERT(method != nullptr);
    ASSERT(!machineCode.empty());

    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};

    ArenaVector<uint8_t> code(allocator.Adapter());
    code.insert(code.begin(), machineCode.cbegin(), machineCode.cend());

    auto compiledMethod = ark::compiler::CompiledMethod(GetArch(), method, 0);
    compiledMethod.SetCode(Span<const uint8_t>(code));

    ark::compiler::CodeInfoBuilder codeInfoBuilder(GetArch(), &allocator);
    spreader_->GetModuleForMethod(method)->GetCodeInfoProducer()->Produce(method, &codeInfoBuilder);

    ArenaVector<uint8_t> codeInfo(allocator.Adapter());
    codeInfoBuilder.Encode(&codeInfo);
    ASSERT(!codeInfo.empty());
    compiledMethod.SetCodeInfo(Span<const uint8_t>(codeInfo));

    return compiledMethod;
}

ArkAotLinker::RoDataSections LLVMAotCompiler::LinkModule(WrappedModule *wrappedModule, ArkAotLinker *linker,
                                                         AotBuilderOffsets *offsets)
{
    exitOnErr_(linker->LoadObject(wrappedModule->TakeObjectFile()));
    exitOnErr_(linker->RelocateSections(offsets->GetSectionAddresses(), offsets->GetMethodOffsets(),
                                        wrappedModule->GetModuleId()));
    const auto &methodPtrs = aotBuilder_->GetMethods();
    const auto &headers = aotBuilder_->GetMethodHeaders();
    ASSERT((methodPtrs.size() == headers.size()) && (methodPtrs.size() >= methods_.size()));
    if (wrappedModule->GetMethods().empty()) {
        return {};
    }
    auto methodsIt = wrappedModule->GetMethods().begin();

    for (size_t i = 0; i < headers.size(); ++i) {
        if (methodsIt == wrappedModule->GetMethods().end()) {
            break;
        }
        if (methodPtrs[i].GetMethod() != *methodsIt) {
            continue;
        }
        auto methodName = wrappedModule->GetLLVMArkInterface()->GetUniqMethodName(*methodsIt);
        auto functionSection = linker->GetLinkedFunctionSection(methodName);
        Span<const uint8_t> code(functionSection.GetMemory(), functionSection.GetSize());
        auto compiledMethod = AdaptCode(static_cast<Method *>(*methodsIt), code);
        if (g_options.IsLlvmDumpCodeinfo()) {
            DumpCodeInfo(compiledMethod);
        }
        aotBuilder_->AdjustMethod(compiledMethod, i);
        methodsIt++;
    }
    return linker->GetLinkedRoDataSections();
}

void LLVMAotCompiler::CompileAll()
{
    // No need to do anything if we have no methods
    if (methods_.empty()) {
        return;
    }
    ASSERT_PRINT(!compiled_, "Cannot compile twice");

    auto modules = spreader_->GetModules();
    uint32_t numThreads = g_options.GetLlvmaotThreads();
    if (numThreads == 1U || modules.size() <= 1U) {
        std::for_each(modules.begin(), modules.end(), [this](auto module) -> void { CompileModule(*module); });
    } else {
        llvm::ThreadPool threadPool {llvm::hardware_concurrency(numThreads)};
        auto thread = runtime_->GetCurrentThread();
        ASSERT(thread != nullptr);
        for (auto &module : modules) {
            threadPool.async(
                [this, thread](auto it) -> void {
                    auto oldThread = runtime_->GetCurrentThread();
                    runtime_->SetCurrentThread(thread);
                    CompileModule(*it);
                    runtime_->SetCurrentThread(oldThread);
                },
                module);
        }
    }

    std::vector<ark::compiler::RoData> roDatas;
    auto offsets = CollectAotBuilderOffsets(modules);
    for (auto &wrappedModule : modules) {
        size_t functionHeaderSize = compiler::CodeInfo::GetCodeOffset(GetArch());
        ArkAotLinker linker {functionHeaderSize};
        auto newRodatas = LinkModule(wrappedModule.get(), &linker, &offsets);
        for (auto &item : newRodatas) {
            roDatas.push_back(ark::compiler::RoData {item.ContentToVector(),
                                                     item.GetName() + std::to_string(wrappedModule->GetModuleId()),
                                                     item.GetAlignment()});
        }
    }
    compiled_ = true;

    std::sort(roDatas.begin(), roDatas.end(),
              [](const compiler::RoData &a, const compiler::RoData &b) -> bool { return a.name < b.name; });
    aotBuilder_->SetRoDataSections(std::move(roDatas));
}

void LLVMAotCompiler::AddInlineMethodByDepth(WrappedModule &module, ark::compiler::Graph *caller,
                                             compiler::RuntimeInterface::MethodPtr method, int32_t depth)
{
    if (!runtime_->IsMethodCanBeInlined(method) || IsInliningDisabled(runtime_, method) ||
        caller->GetRuntime()->IsMethodExternal(caller->GetMethod(), method)) {
        LLVM_LOG(DEBUG, INFRA) << "Will not add inline function = '" << runtime_->GetMethodFullName(method)
                               << "' for caller = '" << runtime_->GetMethodFullName(caller->GetMethod())
                               << "' because !IsMethodCanBeInlined or IsInliningDisabled or the inline "
                                  "function is external for the caller";
        return;
    }

    auto function = module.GetFunctionByMethodPtr(method);
    if (function != nullptr && !function->isDeclarationForLinker()) {
        return;
    }
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator localAllocator {SpaceType::SPACE_TYPE_COMPILER};

    auto graph = CreateGraph(allocator, localAllocator, *static_cast<ark::Method *>(method));
    if (!graph) {
        [[maybe_unused]] auto message = llvm::toString(graph.takeError());
        LLVM_LOG(WARNING, INFRA) << "Could not add inline function = '" << runtime_->GetMethodFullName(method)
                                 << "' for caller = '" << runtime_->GetMethodFullName(caller->GetMethod())
                                 << "' because '" << message << "'";
        return;
    }
    auto callee = llvm::cantFail(std::move(graph));
    if (!AddGraphToModule(callee, module, AddGraphMode::INLINE_FUNCTION)) {
        LLVM_LOG(WARNING, INFRA) << "Could not add inline function = '" << runtime_->GetMethodFullName(method)
                                 << "' for caller = '" << runtime_->GetMethodFullName(caller->GetMethod())
                                 << "' because AddGraphToModule failed";
        return;
    }
    MarkAsInlineObject(module.GetFunctionByMethodPtr(method));

    LLVM_LOG(DEBUG, INFRA) << "Added inline function = '" << runtime_->GetMethodFullName(callee->GetMethod(), true)
                           << "because '" << runtime_->GetMethodFullName(caller->GetMethod(), true) << "' calls it";
    AddInlineFunctionsByDepth(module, callee, depth + 1);
}

void LLVMAotCompiler::AddInlineFunctionsByDepth(WrappedModule &module, ark::compiler::Graph *caller, int32_t depth)
{
    ASSERT(depth >= 0);
    LLVM_LOG(DEBUG, INFRA) << "Adding inline functions, depth = " << depth << ", caller = '"
                           << caller->GetRuntime()->GetMethodFullName(caller->GetMethod()) << "'";

    // Max depth for framework.abc is 26.
    // Limit the depth to avoid excessively large call depth
    if (depth >= MAX_DEPTH) {
        LLVM_LOG(DEBUG, INFRA) << "Exiting, inlining depth exceeded";
        return;
    }

    for (auto basicBlock : caller->GetBlocksRPO()) {
        for (auto instruction : basicBlock->AllInsts()) {
            if (instruction->GetOpcode() != compiler::Opcode::CallStatic) {
                continue;
            }

            auto method = instruction->CastToCallStatic()->GetCallMethod();
            ASSERT(method != nullptr);
            AddInlineMethodByDepth(module, caller, method, depth);
        }
    }
}

llvm::Expected<ark::compiler::Graph *> LLVMAotCompiler::CreateGraph(ArenaAllocator &allocator,
                                                                    ArenaAllocator &localAllocator, Method &method)
{
    // Part of Paoc::CompileAot
    auto sourceLang = method.GetClass()->GetSourceLang();
    bool isDynamic = ark::panda_file::IsDynamicLanguage(sourceLang);

    auto graph = allocator.New<ark::compiler::Graph>(&allocator, &localAllocator, aotBuilder_->GetArch(), &method,
                                                     runtime_, false, nullptr, isDynamic);
    if (graph == nullptr) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(), "Couldn't create graph");
    }
    graph->SetLanguage(sourceLang);

    uintptr_t codeAddress = aotBuilder_->GetCurrentCodeAddress();
    auto aotData = graph->GetAllocator()->New<compiler::AotData>(
        method.GetPandaFile(), graph, codeAddress, aotBuilder_->GetIntfInlineCacheIndex(), aotBuilder_->GetGotPlt(),
        aotBuilder_->GetGotVirtIndexes(), aotBuilder_->GetGotClass(), aotBuilder_->GetGotString(),
        aotBuilder_->GetGotIntfInlineCache(), aotBuilder_->GetGotCommon(), nullptr);

    aotData->SetUseCha(true);
    graph->SetAotData(aotData);

    if (!graph->RunPass<ark::compiler::IrBuilder>()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(), "IrBuilder failed");
    }
    graph->GetAnalysis<ark::compiler::MonitorAnalysis>().SetCheckNonCatchOnly(true);
    bool monitorsCorrect = graph->RunPass<ark::compiler::MonitorAnalysis>();
    graph->InvalidateAnalysis<ark::compiler::MonitorAnalysis>();
    graph->GetAnalysis<ark::compiler::MonitorAnalysis>().SetCheckNonCatchOnly(false);
    if (!monitorsCorrect) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(), "Monitors incorrect");
    }
    auto canCompile = CanCompile(graph);
    if (!canCompile.HasValue()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "CanCompile returned Unexpected with message = '" + canCompile.Error() + "'");
    }
    if (!canCompile.Value() && g_options.IsLlvmSafemode()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "CanCompile returned false and --llvm-safemode enabled");
    }
    return graph;
}

void LLVMAotCompiler::CompileModule(WrappedModule &module)
{
    auto compilerOptions = InitializeLLVMCompilerOptions();
    module.GetDebugData()->Finalize();
    auto arkInterface = module.GetLLVMArkInterface().get();
    auto llvmIrModule = module.GetModule().get();

    auto &targetMachine = module.GetTargetMachine();
    LLVMOptimizer llvmOptimizer {compilerOptions, arkInterface, targetMachine};

    // Dumps require lock to be not messed up between different modules compiled in parallel.
    // NB! When ark is built with TSAN there are warnings coming from libLLVM.so.
    // They do not fire in tests because we do not run parallel compilation in tests.
    // We have to use suppression file, to run parallel compilation under tsan

    {  // Dump before optimization
        llvm::sys::ScopedLock scopedLock {lock_};
        llvmOptimizer.DumpModuleBefore(llvmIrModule);
    }

    // Optimize IR
    llvmOptimizer.OptimizeModule(llvmIrModule);

    {  // Dump after optimization
        llvm::sys::ScopedLock scopedLock {lock_};
        llvmOptimizer.DumpModuleAfter(llvmIrModule);
    }

    // clang-format off
    MIRCompiler mirCompiler {targetMachine, [&arkInterface](InsertingPassManager *manager) -> void {
                                manager->InsertBefore(&llvm::FEntryInserterID, CreateFrameLoweringPass(arkInterface));
                            }};
    // clang-format on

    // Create machine code
    auto file = exitOnErr_(mirCompiler.CompileModule(*llvmIrModule));
    if (file->HasSection(".llvm_stackmaps")) {
        auto section = file->GetSection(".llvm_stackmaps");
        module.GetCodeInfoProducer()->SetStackMap(section.GetMemory(), section.GetSize());

        auto stackmapInfo = file->GetStackMapInfo();

        for (auto method : module.GetMethods()) {
            auto fullMethodName = module.GetLLVMArkInterface()->GetUniqMethodName(method);
            if (stackmapInfo.find(fullMethodName) != stackmapInfo.end()) {
                module.GetCodeInfoProducer()->AddSymbol(static_cast<Method *>(method), stackmapInfo.at(fullMethodName));
            }
        }
    }
    if (file->HasSection(".llvm_faultmaps")) {
        auto section = file->GetSection(".llvm_faultmaps");
        module.GetCodeInfoProducer()->SetFaultMaps(section.GetMemory(), section.GetSize());

        auto faultMapInfo = file->GetFaultMapInfo();

        for (auto method : module.GetMethods()) {
            auto fullMethodName = module.GetLLVMArkInterface()->GetUniqMethodName(method);
            if (faultMapInfo.find(fullMethodName) != faultMapInfo.end()) {
                module.GetCodeInfoProducer()->AddFaultMapSymbol(static_cast<Method *>(method),
                                                                faultMapInfo.at(fullMethodName));
            }
        }
    }

    if (g_options.IsLlvmDumpObj()) {
        int32_t moduleNumber = compiledModules_++;
        std::string fileName = "llvm-output-" + std::to_string(moduleNumber) + ".o";
        file->WriteTo(fileName);
    }
    module.SetCompiled(std::move(file));
}

void LLVMAotCompiler::DumpCodeInfo(compiler::CompiledMethod &method) const
{
    LLVM_LOG(DEBUG, INFRA) << "Dumping code info for method: " << runtime_->GetMethodFullName(method.GetMethod());
    std::stringstream ss;
    compiler::CodeInfo info;
    info.Decode(method.GetCodeInfo());
    info.Dump(ss);
    LLVM_LOG(DEBUG, INFRA) << ss.str();
}

}  // namespace ark::llvmbackend
