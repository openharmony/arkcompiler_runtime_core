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
#include "verification_context.h"

#include "verification/jobs/job.h"
#include "verification/jobs/cache.h"

#include "value/abstract_typed_value.h"
#include "type/type_system.h"

#include "cflow/cflow_info.h"

#include "runtime/include/method.h"

#include "macros.h"

#include <optional>

#include "abs_int_inl.h"

#include "util/str.h"
#include "util/hash.h"

#include <utility>
#include <tuple>
#include <functional>

namespace panda::verifier {

#include "abs_int_inl_gen.h"

}  // namespace panda::verifier

namespace panda::verifier {

VerificationContext PrepareVerificationContext(PandaTypes &pandaTypes, const Job &job)
{
    auto &cached_method = job.JobCachedMethod();

    auto &klass = cached_method.klass;

    Type method_class_type = pandaTypes.TypeOf(klass);

    VerificationContext verifCtx {pandaTypes, job, method_class_type};
    // TODO(vdyadov): ASSERT(cflow_info corresponds method)

    auto &cflow_info = verifCtx.CflowInfo();
    auto &exec_ctx = verifCtx.ExecCtx();

    LOG_VERIFIER_DEBUG_METHOD_VERIFICATION(cached_method.GetName());

    /*
    1. build initial reg_context for the method entry
    */
    RegContext &reg_ctx = verifCtx.ExecCtx().CurrentRegContext();
    reg_ctx.Clear();

    auto num_vregs = cached_method.num_vregs;

    const auto &signature = verifCtx.Types().MethodSignature(cached_method);

    for (size_t idx = 0; idx < signature.size() - 1; ++idx) {
        const Type &t = pandaTypes.TypeOf(signature[idx]);
        reg_ctx[num_vregs++] = AbstractTypedValue {t, verifCtx.NewVar(), AbstractTypedValue::Start {}, idx};
    }
    LOG_VERIFIER_DEBUG_REGISTERS("registers =",
                                 reg_ctx.DumpRegs([&pandaTypes](const auto &t) { return pandaTypes.ImageOf(t); }));

    verifCtx.SetReturnType(pandaTypes.TypeOf(signature.back()));

    LOG_VERIFIER_DEBUG_RESULT(pandaTypes.ImageOf(verifCtx.ReturnType()));

    /*
    3. Add checkpoint for exc. handlers
    */

    for (const auto &exc_handler : cflow_info.ExcHandlers()) {
        auto &&handler = [&exec_ctx](const uint8_t *pc) {
            exec_ctx.SetCheckPoint(pc);
            return true;
        };
        cflow_info.ExcSrcMap().ForSourcesInRange(exc_handler.TryBlock.Start, exc_handler.TryBlock.End, handler);
    }

    /*
    3. add start entry of method
    */

    const uint8_t *method_pc_start_ptr = cached_method.bytecode;

    verifCtx.ExecCtx().AddEntryPoint(method_pc_start_ptr, EntryPointType::METHOD_BODY);
    verifCtx.ExecCtx().StoreCurrentRegContextForAddr(method_pc_start_ptr);

    return verifCtx;
}

namespace {

VerificationStatus VerifyEntryPoints(VerificationContext &verifCtx, ExecContext &exec_ctx)
{
    const auto &debug_opts = Runtime::GetCurrent()->GetVerificationOptions().Debug;

    /*
    start main loop: get entry point with context, process, repeat
    */

    uint8_t const *entry_point = nullptr;
    EntryPointType entry_type;
    VerificationStatus worstSoFar = VerificationStatus::OK;

    while (exec_ctx.GetEntryPointForChecking(&entry_point, &entry_type) == ExecContext::Status::OK) {
#ifndef NDEBUG
        auto &cflow_info = verifCtx.CflowInfo();
        const void *codeStart = cflow_info.InstMap().AddrStart<const void *>();
        LOG_VERIFIER_DEBUG_CODE_BLOCK_VERIFICATION(
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry_point) - reinterpret_cast<uintptr_t>(codeStart)),
            (entry_type == EntryPointType::METHOD_BODY ? "method body" : "exception handler"));
#endif
        auto result = AbstractInterpret(verifCtx, entry_point, entry_type);
        if (debug_opts.Allow.ErrorInExceptionHandler && entry_type == EntryPointType::EXCEPTION_HANDLER &&
            result == VerificationStatus::ERROR) {
            result = VerificationStatus::WARNING;
        }
        if (result == VerificationStatus::ERROR) {
            return result;
        }
        worstSoFar = std::max(worstSoFar, result);
    }

    return worstSoFar;
}

void ComputeRegContext(CflowExcHandlerInfo const &exc_handler, VerificationContext &verifCtx, RegContext *reg_context)
{
    auto &cflow_info = verifCtx.CflowInfo();
    auto &exec_ctx = verifCtx.ExecCtx();

#ifndef NDEBUG
    const void *codeStart = cflow_info.InstMap().AddrStart<const void *>();
    auto takeAddress = [&](const void *ptr) {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(codeStart));
    };

    LOG_VERIFIER_DEBUG_EXCEPTION_HANDLER_COMMON_CONTEXT_COMPUTATION(takeAddress(exc_handler.Start), "",
                                                                    takeAddress(exc_handler.TryBlock.Start),
                                                                    takeAddress(exc_handler.TryBlock.End));
#endif

#ifndef NDEBUG
    auto image_of = [&verifCtx](const auto &t) { return verifCtx.Types().ImageOf(t); };
#endif

    bool first = true;
    exec_ctx.ForContextsOnCheckPointsInRange(
#ifndef NDEBUG
        exc_handler.TryBlock.Start, exc_handler.TryBlock.End,
        [&cflow_info, &reg_context, &image_of, &first](const uint8_t *pc, const RegContext &ctx) {
#else
        exc_handler.TryBlock.Start, exc_handler.TryBlock.End,
        [&cflow_info, &reg_context, &first](const uint8_t *pc, const RegContext &ctx) {
#endif
            if (cflow_info.ExcSrcMap().IsExceptionSource(pc)) {
#ifndef NDEBUG
                LOG_VERIFIER_DEBUG_REGISTERS("+", ctx.DumpRegs(image_of));
#endif
                if (first) {
                    first = false;
                    *reg_context = ctx;
                } else {
                    *reg_context &= ctx;
                }
            }
            return true;
        });
#ifndef NDEBUG
    LOG_VERIFIER_DEBUG_REGISTERS("=", reg_context->DumpRegs(image_of));
#endif

    reg_context->RemoveInconsistentRegs();

#ifndef NDEBUG
    if (reg_context->HasInconsistentRegs()) {
        LOG_VERIFIER_COMMON_CONTEXT_INCONSISTENT_REGISTER_HEADER();
        for (int reg_num : reg_context->InconsistentRegsNums()) {
            LOG(DEBUG, VERIFIER) << AbsIntInstructionHandler::RegisterName(reg_num);
        }
    }
#endif
}

VerificationStatus VerifyExcHandler(CflowExcHandlerInfo const &exc_handler, VerificationContext &verifCtx,
                                    RegContext &reg_context)
{
    auto &exec_ctx = verifCtx.ExecCtx();
    auto exception = exc_handler.CachedException;

#ifndef NDEBUG
    auto &cflow_info = verifCtx.CflowInfo();
    const void *codeStart = cflow_info.InstMap().AddrStart<const void *>();
    auto takeAddress = [&](const void *ptr) {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(codeStart));
    };

    LOG(DEBUG, VERIFIER) << "Exception handler at " << std::hex << "0x" << takeAddress(exc_handler.Start)
                         << (exception.HasRef() ? PandaString {", for exception '"} + exception->GetName() + "' "
                                                : PandaString {""})
                         << ", try block scope: [ "
                         << "0x" << takeAddress(exc_handler.TryBlock.Start) << ", "
                         << "0x" << takeAddress(exc_handler.TryBlock.End) << " ]";
#endif

    Type exception_type;
    if (exception.HasRef()) {
        exception_type = verifCtx.Types().TypeOf(exception.Get());
    } else {
        auto lang = verifCtx.GetJob().JobCachedMethod().GetSourceLang();
        exception_type = verifCtx.Types().Throwable(lang);
    }

    if (exception_type.IsValid()) {
        const int ACC = -1;
        reg_context[ACC] = AbstractTypedValue {exception_type, verifCtx.NewVar()};
    }

    exec_ctx.CurrentRegContext() = reg_context;
    exec_ctx.AddEntryPoint(exc_handler.Start, EntryPointType::EXCEPTION_HANDLER);
    exec_ctx.StoreCurrentRegContextForAddr(exc_handler.Start);

    return VerifyEntryPoints(verifCtx, exec_ctx);
}

}  // namespace

VerificationStatus VerifyMethod(VerificationContext &verifCtx)
{
    VerificationStatus worstSoFar = VerificationStatus::OK;
    auto &cflow_info = verifCtx.CflowInfo();
    auto &exec_ctx = verifCtx.ExecCtx();

    worstSoFar = std::max(worstSoFar, VerifyEntryPoints(verifCtx, exec_ctx));
    if (worstSoFar == VerificationStatus::ERROR) {
        return worstSoFar;
    }

    std::vector<CflowExcHandlerInfo const *> sorted_handlers;
    for (const auto &exc_handler : cflow_info.ExcHandlers()) {
        sorted_handlers.push_back(&exc_handler);
    }
    std::sort(sorted_handlers.begin(), sorted_handlers.end(), [](auto h1, auto h2) { return h1->Start < h2->Start; });

    PandaUnorderedMap<std::pair<const uint8_t *, const uint8_t *>, RegContext> scope_reg_context;

    for (const auto exc_handler : sorted_handlers) {
        // calculate contexts for exception handlers scopes
        auto scope = std::make_pair(exc_handler->TryBlock.Start, exc_handler->TryBlock.End);
        if (scope_reg_context.count(scope) == 0) {
            RegContext reg_context;
            ComputeRegContext(*exc_handler, verifCtx, &reg_context);
            scope_reg_context[scope] = reg_context;
        }

        RegContext &reg_context = scope_reg_context[scope];

        worstSoFar = std::max(worstSoFar, VerifyExcHandler(*exc_handler, verifCtx, reg_context));
        if (worstSoFar == VerificationStatus::ERROR) {
            return worstSoFar;
        }
    }

    // TODO(vdyadov): account for dead code
    // add marking at Sync() and calc unmarked blocks at the end
    const uint8_t *dummy_entry_point;
    EntryPointType dummy_entry_type;

    if (exec_ctx.GetEntryPointForChecking(&dummy_entry_point, &dummy_entry_type) ==
        ExecContext::Status::NO_ENTRY_POINTS_WITH_CONTEXT) {
        // TODO(vdyadov): log remaining entry points as unreachable
        worstSoFar = std::max(worstSoFar, VerificationStatus::WARNING);
    }

    return worstSoFar;
}

}  // namespace panda::verifier
