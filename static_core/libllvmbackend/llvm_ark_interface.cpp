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

#include "compiler/optimizer/ir/runtime_interface.h"
#include "runtime/include/method.h"
#include "events/events.h"
#include "compiler_options.h"
#include "utils/cframe_layout.h"
#include "llvm_ark_interface.h"
#include "llvm_logger.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm_options.h"

using panda::compiler::RuntimeInterface;

namespace {
constexpr panda::Arch LLVMArchToArkArch(llvm::Triple::ArchType arch)
{
    switch (arch) {
        case llvm::Triple::ArchType::aarch64:
            return panda::Arch::AARCH64;
        case llvm::Triple::ArchType::x86_64:
            return panda::Arch::X86_64;
        case llvm::Triple::ArchType::x86:
            return panda::Arch::X86;
        case llvm::Triple::ArchType::arm:
            return panda::Arch::AARCH32;
        default:
            UNREACHABLE();
            return panda::Arch::NONE;
    }
}
#include <entrypoints_gen.inl>
#include <intrinsics_gen.inl>
#include <intrinsic_names_gen.inl>
#include <entrypoints_llvm_ark_interface_gen.inl>
}  // namespace

namespace panda::llvmbackend {

panda::llvmbackend::LLVMArkInterface::LLVMArkInterface(RuntimeInterface *runtime, llvm::Triple triple)
    : runtime_(runtime), triple_(std::move(triple))
{
    ASSERT(runtime != nullptr);
}

const char *LLVMArkInterface::GetThreadRegister() const
{
    switch (LLVMArchToArkArch(triple_.getArch())) {
        case Arch::AARCH64: {
            constexpr auto EXPECTED_REGISTER = 28;
            static_assert(ArchTraits<Arch::AARCH64>::THREAD_REG == EXPECTED_REGISTER);
            return "x28";
        }
        case Arch::X86_64: {
            constexpr auto EXPECTED_REGISTER = 15;
            static_assert(ArchTraits<Arch::X86_64>::THREAD_REG == EXPECTED_REGISTER);
            return "r15";
        }
        default:
            UNREACHABLE();
    }
}

const char *LLVMArkInterface::GetFramePointerRegister() const
{
    switch (LLVMArchToArkArch(triple_.getArch())) {
        case Arch::AARCH64:
            return "x29";
        case Arch::X86_64:
            return "rbp";
        default:
            UNREACHABLE();
    }
}

uintptr_t LLVMArkInterface::GetEntrypointTlsOffset(EntrypointId id) const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    using PandaEntrypointId = panda::compiler::RuntimeInterface::EntrypointId;
    return runtime_->GetEntrypointTlsOffset(arkArch, static_cast<PandaEntrypointId>(id));
}

size_t LLVMArkInterface::GetTlsPreWrbEntrypointOffset() const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    return runtime_->GetTlsPreWrbEntrypointOffset(arkArch);
}

uint32_t LLVMArkInterface::GetManagedThreadPostWrbOneObjectOffset() const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    return cross_values::GetManagedThreadPostWrbOneObjectOffset(arkArch);
}

llvm::StringRef LLVMArkInterface::GetRuntimeFunctionName(LLVMArkInterface::RuntimeCallType callType,
                                                         LLVMArkInterface::IntrinsicId id)
{
    if (callType == LLVMArkInterface::RuntimeCallType::INTRINSIC) {
        return llvm::StringRef(GetIntrinsicRuntimeFunctionName(id));
    }
    // sanity check
    ASSERT(callType == LLVMArkInterface::RuntimeCallType::ENTRYPOINT);
    return llvm::StringRef(GetEntrypointRuntimeFunctionName(id));
}

llvm::FunctionType *LLVMArkInterface::GetRuntimeFunctionType(llvm::StringRef name) const
{
    return runtimeFunctionTypes_.lookup(name);
}

llvm::FunctionType *LLVMArkInterface::GetOrCreateRuntimeFunctionType(llvm::LLVMContext &ctx, llvm::Module *module,
                                                                     LLVMArkInterface::RuntimeCallType callType,
                                                                     LLVMArkInterface::IntrinsicId id)
{
    auto rtFunctionName = GetRuntimeFunctionName(callType, id);
    auto rtFunctionTy = GetRuntimeFunctionType(rtFunctionName);
    if (rtFunctionTy != nullptr) {
        return rtFunctionTy;
    }

    if (callType == RuntimeCallType::INTRINSIC) {
        rtFunctionTy = GetIntrinsicDeclaration(ctx, static_cast<RuntimeInterface::IntrinsicId>(id));
    } else {
        // sanity check
        ASSERT(callType == RuntimeCallType::ENTRYPOINT);
        rtFunctionTy = GetEntrypointDeclaration(ctx, module, static_cast<RuntimeInterface::EntrypointId>(id));
    }

    ASSERT(rtFunctionTy != nullptr);
    runtimeFunctionTypes_.insert({rtFunctionName, rtFunctionTy});
    return rtFunctionTy;
}

void LLVMArkInterface::RememberFunctionOrigin(const llvm::Function *function, MethodPtr methodPtr)
{
    ASSERT(function != nullptr);
    ASSERT(methodPtr != nullptr);

    auto file = static_cast<panda_file::File *>(runtime_->GetBinaryFileForMethod(methodPtr));
    [[maybe_unused]] auto insertionResult = functionOrigins_.insert({function, file});
    ASSERT(insertionResult.second);
    LLVM_LOG(DEBUG, INFRA) << function->getName().data() << " defined in " << runtime_->GetFileName(methodPtr);
}

LLVMArkInterface::RuntimeCallee LLVMArkInterface::GetEntrypointCallee(EntrypointId id) const
{
    using PandaEntrypointId = panda::compiler::RuntimeInterface::EntrypointId;
    auto eid = static_cast<PandaEntrypointId>(id);
    auto functionName = GetEntrypointInternalName(eid);
    auto functionProto = GetRuntimeFunctionType(functionName);
    ASSERT(functionProto != nullptr);
    return {functionProto, functionName};
}

llvm::Function *LLVMArkInterface::GetFunctionByMethodPtr(LLVMArkInterface::MethodPtr method) const
{
    ASSERT(method != nullptr);

    return functions_.lookup(method);
}

void LLVMArkInterface::PutFunction(LLVMArkInterface::MethodPtr methodPtr, llvm::Function *function)
{
    ASSERT(function != nullptr);
    ASSERT(methodPtr != nullptr);
    // We are not expecting `tan` functions other than we are adding in LLVMIrConstructor::EmitTan()
    ASSERT(function->getName() != "tan");

    [[maybe_unused]] auto fInsertionResult = functions_.insert({methodPtr, function});
    ASSERT_PRINT(fInsertionResult.first->second == function,
                 std::string("Attempt to map '") + GetUniqMethodName(methodPtr) + "' to a function = '" +
                     function->getName().str() + "', but the method_ptr already mapped to '" +
                     fInsertionResult.first->second->getName().str() + "'");
    auto sourceLang = static_cast<uint8_t>(runtime_->GetMethodSourceLanguage(methodPtr));
    [[maybe_unused]] auto slInsertionResult = sourceLanguages_.insert({function, sourceLang});
    ASSERT(slInsertionResult.first->second == sourceLang);
}

const char *LLVMArkInterface::GetIntrinsicRuntimeFunctionName(LLVMArkInterface::IntrinsicId id) const
{
    ASSERT(id >= 0 && id < static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::COUNT));
    return GetIntrinsicInternalName(static_cast<RuntimeInterface::IntrinsicId>(id));
}

const char *LLVMArkInterface::GetEntrypointRuntimeFunctionName(LLVMArkInterface::EntrypointId id) const
{
    ASSERT(id >= 0 && id < static_cast<EntrypointId>(RuntimeInterface::EntrypointId::COUNT));
    return GetEntrypointInternalName(static_cast<RuntimeInterface::EntrypointId>(id));
}

std::string LLVMArkInterface::GetUniqMethodName(LLVMArkInterface::MethodPtr methodPtr) const
{
    ASSERT(methodPtr != nullptr);

    if (IsIrtocMode()) {
        return runtime_->GetMethodName(methodPtr);
    }
#ifndef NDEBUG
    auto uniqName = std::string(runtime_->GetMethodFullName(methodPtr, true));
    uniqName.append("_id_");
    uniqName.append(std::to_string(runtime_->GetUniqMethodId(methodPtr)));
#else
    std::stringstream ss_uniq_name;
    ss_uniq_name << "f_" << std::hex << method_ptr;
    auto uniq_name = ss_uniq_name.str();
#endif
    return uniqName;
}

std::string LLVMArkInterface::GetUniqMethodName(const Method *method) const
{
    ASSERT(method != nullptr);

    auto casted = const_cast<Method *>(method);
    return GetUniqMethodName(static_cast<MethodPtr>(casted));
}

std::string LLVMArkInterface::GetUniqueBasicBlockName(const std::string &bbName, const std::string &uniqueSuffix)
{
    std::stringstream uniqueName;
    const std::string nameDelimiter = "..";
    auto first = bbName.find(nameDelimiter);
    if (first == std::string::npos) {
        uniqueName << bbName << "_" << uniqueSuffix;
        return uniqueName.str();
    }

    auto second = bbName.rfind(nameDelimiter);
    ASSERT(second != std::string::npos);

    uniqueName << bbName.substr(0, first + 2U) << uniqueSuffix << bbName.substr(second, bbName.size());

    return uniqueName.str();
}

bool LLVMArkInterface::IsIrtocMode() const
{
    return true;
}

void LLVMArkInterface::AppendIrtocReturnHandler(llvm::StringRef returnHandler)
{
    irtocReturnHandlers_.push_back(returnHandler);
}

bool LLVMArkInterface::IsIrtocReturnHandler(const llvm::Function &function) const
{
    return std::find(irtocReturnHandlers_.cbegin(), irtocReturnHandlers_.cend(), function.getName()) !=
           irtocReturnHandlers_.cend();
}

}  // namespace panda::llvmbackend
