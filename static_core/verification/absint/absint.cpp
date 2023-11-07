/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "absint.h"
#include "exec_context.h"
#include "file_items.h"
#include "include/thread_scopes.h"
#include "plugins.h"
#include "verification_context.h"

#include "verification/jobs/job.h"

#include "value/abstract_typed_value.h"
#include "type/type_system.h"

#include "cflow/cflow_info.h"

#include "runtime/include/method.h"
#include "runtime/include/method-inl.h"

#include "macros.h"

#include <locale>
#include <optional>

#include "abs_int_inl.h"
#include "handle_gen.h"

#include "util/str.h"
#include "util/hash.h"

#include <utility>
#include <tuple>
#include <functional>

namespace panda::verifier {

#include "abs_int_inl_gen.h"

}  // namespace panda::verifier

namespace panda::verifier {

using TryBlock = panda_file::CodeDataAccessor::TryBlock;
using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

VerificationContext PrepareVerificationContext(TypeSystem *type_system, Job const *job)
{
    auto *method = job->JobMethod();

    auto *klass = method->GetClass();

    Type method_class_type {klass};

    VerificationContext verif_ctx {type_system, job, method_class_type};
    // NOTE(vdyadov): ASSERT(cflow_info corresponds method)

    auto &cflow_info = verif_ctx.CflowInfo();
    auto &exec_ctx = verif_ctx.ExecCtx();

    LOG_VERIFIER_DEBUG_METHOD_VERIFICATION(method->GetFullName());

    /*
    1. build initial reg_context for the method entry
    */
    RegContext &reg_ctx = verif_ctx.ExecCtx().CurrentRegContext();
    reg_ctx.Clear();

    auto num_vregs = method->GetNumVregs();
    reg_ctx[num_vregs] = AbstractTypedValue {};

    const auto &signature = type_system->GetMethodSignature(method);

    for (size_t idx = 0; idx < signature->args.size(); ++idx) {
        Type const &t = signature->args[idx];
        reg_ctx[num_vregs++] = AbstractTypedValue {t, verif_ctx.NewVar(), AbstractTypedValue::Start {}, idx};
    }
    LOG_VERIFIER_DEBUG_REGISTERS("registers =", reg_ctx.DumpRegs(type_system));

    verif_ctx.SetReturnType(&signature->result);

    LOG_VERIFIER_DEBUG_RESULT(signature->result.ToString(type_system));

    /*
    3. Add checkpoint for exc. handlers
    */
    method->EnumerateTryBlocks([&](TryBlock const &try_block) {
        uint8_t const *code = method->GetInstructions();
        uint8_t const *try_start_pc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(code) +
                                                                  static_cast<uintptr_t>(try_block.GetStartPc()));
        uint8_t const *try_end_pc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(try_start_pc) +
                                                                static_cast<uintptr_t>(try_block.GetLength()));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (uint8_t const *pc = try_start_pc; pc < try_end_pc; pc++) {
            if (cflow_info.IsFlagSet(pc, CflowMethodInfo::EXCEPTION_SOURCE)) {
                exec_ctx.SetCheckPoint(pc);
            }
        }
        return true;
    });

    /*
    3. add Start entry of method
    */

    const uint8_t *method_pc_start_ptr = method->GetInstructions();

    verif_ctx.ExecCtx().AddEntryPoint(method_pc_start_ptr, EntryPointType::METHOD_BODY);
    verif_ctx.ExecCtx().StoreCurrentRegContextForAddr(method_pc_start_ptr);

    return verif_ctx;
}

namespace {

VerificationStatus VerifyEntryPoints(VerificationContext &verif_ctx, ExecContext &exec_ctx)
{
    const auto &debug_opts = GetServiceConfig(verif_ctx.GetJob()->GetService())->opts.debug;

    /*
    Start main loop: get entry point with context, process, repeat
    */

    uint8_t const *entry_point = nullptr;
    EntryPointType entry_type;
    VerificationStatus worst_so_far = VerificationStatus::OK;

    while (exec_ctx.GetEntryPointForChecking(&entry_point, &entry_type) == ExecContext::Status::OK) {
#ifndef NDEBUG
        const void *code_start = verif_ctx.GetJob()->JobMethod()->GetInstructions();
        LOG_VERIFIER_DEBUG_CODE_BLOCK_VERIFICATION(
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry_point) - reinterpret_cast<uintptr_t>(code_start)),
            (entry_type == EntryPointType::METHOD_BODY ? "method body" : "exception handler"));
#endif
        auto result = AbstractInterpret(verif_ctx, entry_point, entry_type);
        if (debug_opts.allow.error_in_exception_handler && entry_type == EntryPointType::EXCEPTION_HANDLER &&
            result == VerificationStatus::ERROR) {
            result = VerificationStatus::WARNING;
        }
        if (result == VerificationStatus::ERROR) {
            return result;
        }
        worst_so_far = std::max(worst_so_far, result);
    }

    return worst_so_far;
}

bool ComputeRegContext(Method const *method, TryBlock const *try_block, VerificationContext &verif_ctx,
                       RegContext *reg_context)
{
    auto &cflow_info = verif_ctx.CflowInfo();
    auto &exec_ctx = verif_ctx.ExecCtx();
    auto *type_system = verif_ctx.GetTypeSystem();
    auto start = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                   try_block->GetStartPc());
    auto end = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                 try_block->GetStartPc() + try_block->GetLength());

    LOG_VERIFIER_DEBUG_TRY_BLOCK_COMMON_CONTEXT_COMPUTATION(try_block->GetStartPc(),
                                                            try_block->GetStartPc() + try_block->GetLength());

    bool first = true;
    exec_ctx.ForContextsOnCheckPointsInRange(start, end, [&](const uint8_t *pc, const RegContext &ctx) {
        if (cflow_info.IsFlagSet(pc, CflowMethodInfo::EXCEPTION_SOURCE)) {
            LOG_VERIFIER_DEBUG_REGISTERS("+", ctx.DumpRegs(type_system));
            if (first) {
                first = false;
                *reg_context = ctx;
            } else {
                reg_context->UnionWith(&ctx, type_system);
            }
        }
        return true;
    });
    LOG_VERIFIER_DEBUG_REGISTERS("=", reg_context->DumpRegs(type_system));

    if (first) {
        // return false when the try block does not have instructions to throw exception
        return false;
    }

    reg_context->RemoveInconsistentRegs();

#ifndef NDEBUG
    if (reg_context->HasInconsistentRegs()) {
        LOG_VERIFIER_COMMON_CONTEXT_INCONSISTENT_REGISTER_HEADER();
        for (int reg_num : reg_context->InconsistentRegsNums()) {
            LOG(DEBUG, VERIFIER) << AbsIntInstructionHandler::RegisterName(reg_num);
        }
    }
#endif

    return true;
}

VerificationStatus VerifyExcHandler([[maybe_unused]] TryBlock const *try_block, CatchBlock const *catch_block,
                                    VerificationContext *verif_ctx, RegContext *reg_context)
{
    Method const *method = verif_ctx->GetMethod();
    auto &exec_ctx = verif_ctx->ExecCtx();
    auto exception_idx = catch_block->GetTypeIdx();
    auto lang_ctx = LanguageContext(plugins::GetLanguageContextBase(method->GetClass()->GetSourceLang()));
    Class *exception = nullptr;
    if (exception_idx != panda_file::INVALID_INDEX) {
        ScopedChangeThreadStatus st {ManagedThread::GetCurrent(), ThreadStatus::RUNNING};
        exception = verif_ctx->GetJob()->GetService()->class_linker->GetExtension(lang_ctx)->GetClass(
            *method->GetPandaFile(), method->GetClass()->ResolveClassIndex(exception_idx));
    }

    LOG(DEBUG, VERIFIER) << "Exception handler at " << std::hex << "0x" << catch_block->GetHandlerPc()
                         << (exception != nullptr
                                 ? PandaString {", for exception '"} + PandaString {exception->GetName()} + "' "
                                 : PandaString {""})
                         << ", try block scope: [ "
                         << "0x" << try_block->GetStartPc() << ", "
                         << "0x" << (try_block->GetStartPc() + try_block->GetLength()) << " ]";

    Type exception_type {};
    if (exception != nullptr) {
        exception_type = Type {exception};
    } else {
        exception_type = verif_ctx->GetTypeSystem()->Throwable();
    }

    if (exception_type.IsConsistent()) {
        const int acc = -1;
        (*reg_context)[acc] = AbstractTypedValue {exception_type, verif_ctx->NewVar()};
    }

    auto start_pc = reinterpret_cast<uint8_t const *>(
        reinterpret_cast<uintptr_t>(verif_ctx->GetMethod()->GetInstructions()) + catch_block->GetHandlerPc());

    exec_ctx.CurrentRegContext() = *reg_context;
    exec_ctx.AddEntryPoint(start_pc, EntryPointType::EXCEPTION_HANDLER);
    exec_ctx.StoreCurrentRegContextForAddr(start_pc);

    return VerifyEntryPoints(*verif_ctx, exec_ctx);
}

}  // namespace

VerificationStatus VerifyMethod(VerificationContext &verif_ctx)
{
    VerificationStatus worst_so_far = VerificationStatus::OK;
    auto &exec_ctx = verif_ctx.ExecCtx();

    worst_so_far = std::max(worst_so_far, VerifyEntryPoints(verif_ctx, exec_ctx));
    if (worst_so_far == VerificationStatus::ERROR) {
        return worst_so_far;
    }

    // Need to have the try blocks sorted!
    verif_ctx.GetMethod()->EnumerateTryBlocks([&](TryBlock &try_block) {
        bool try_block_can_throw = true;
        RegContext reg_context;
        try_block_can_throw = ComputeRegContext(verif_ctx.GetMethod(), &try_block, verif_ctx, &reg_context);
        if (!try_block_can_throw) {
            // catch block is unreachable
        } else {
            try_block.EnumerateCatchBlocks([&](CatchBlock const &catch_block) {
                worst_so_far =
                    std::max(worst_so_far, VerifyExcHandler(&try_block, &catch_block, &verif_ctx, &reg_context));
                return (worst_so_far != VerificationStatus::ERROR);
            });
        }
        return true;
    });

    if (worst_so_far == VerificationStatus::ERROR) {
        return worst_so_far;
    }

    // NOTE(vdyadov): account for dead code
    const uint8_t *dummy_entry_point;
    EntryPointType dummy_entry_type;

    if (exec_ctx.GetEntryPointForChecking(&dummy_entry_point, &dummy_entry_type) ==
        ExecContext::Status::NO_ENTRY_POINTS_WITH_CONTEXT) {
        // NOTE(vdyadov): log remaining entry points as unreachable
        worst_so_far = std::max(worst_so_far, VerificationStatus::WARNING);
    }

    return worst_so_far;
}

}  // namespace panda::verifier
