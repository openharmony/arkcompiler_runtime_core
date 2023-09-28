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

#ifndef PANDA_VERIFICATION_VERIFICATION_CONTEXT_HPP_
#define PANDA_VERIFICATION_VERIFICATION_CONTEXT_HPP_

#include "libpandabase/macros.h"
#include "runtime/include/method.h"
#include "runtime/include/method-inl.h"

#include "verification/absint/exec_context.h"
#include "verification/cflow/cflow_info.h"
#include "verification/jobs/job.h"
#include "verification/plugins.h"
#include "verification/type/type_system.h"
#include "verification/util/lazy.h"
#include "verification/util/callable.h"
#include "verification/value/variables.h"

#include <functional>

namespace panda::verifier {
using CallIntoRuntimeHandler = callable<void(callable<void()>)>;

class VerificationContext {
public:
    using Var = Variables::Var;

    VerificationContext(TypeSystem *type_system, Job const *job, Type method_class_type)
        : types_ {type_system},
          job_ {job},
          method_class_type_ {method_class_type},
          exec_ctx_ {CflowInfo().GetAddrStart(), CflowInfo().GetAddrEnd(), type_system},
          plugin_ {plugin::GetLanguagePlugin(job->JobMethod()->GetClass()->GetSourceLang())}
    {
        Method const *method = job->JobMethod();
        // set checkpoints for reg_context storage
        // start of method is checkpoint too
        ExecCtx().SetCheckPoint(CflowInfo().GetAddrStart());
        uint8_t const *start = CflowInfo().GetAddrStart();
        uint8_t const *end = CflowInfo().GetAddrEnd();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (uint8_t const *pc = start; pc < end; pc++) {
            if (CflowInfo().IsFlagSet(pc, CflowMethodInfo::JUMP_TARGET)) {
                ExecCtx().SetCheckPoint(pc);
            }
        }
        method->EnumerateCatchBlocks([&](uint8_t const *try_start, uint8_t const *try_end,
                                         panda_file::CodeDataAccessor::CatchBlock const &catch_block) {
            auto catch_start =
                reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                  static_cast<uintptr_t>(catch_block.GetHandlerPc()));
            ExecCtx().SetCheckPoint(try_start);
            ExecCtx().SetCheckPoint(try_end);
            ExecCtx().SetCheckPoint(catch_start);
            return true;
        });
    }

    ~VerificationContext() = default;
    DEFAULT_MOVE_SEMANTIC(VerificationContext);
    DEFAULT_COPY_SEMANTIC(VerificationContext);

    Job const *GetJob() const
    {
        return job_;
    }

    const CflowMethodInfo &CflowInfo() const
    {
        return job_->JobMethodCflow();
    }

    Method const *GetMethod() const
    {
        return job_->JobMethod();
    }

    Type GetMethodClass() const
    {
        return method_class_type_;
    }

    ExecContext &ExecCtx()
    {
        return exec_ctx_;
    }

    const ExecContext &ExecCtx() const
    {
        return exec_ctx_;
    }

    TypeSystem *GetTypeSystem()
    {
        return types_;
    }

    Var NewVar()
    {
        return types_->NewVar();
    }

    Type ReturnType() const
    {
        return return_type_;
    }

    void SetReturnType(Type const *type)
    {
        return_type_ = *type;
    }

    plugin::Plugin const *GetPlugin()
    {
        return plugin_;
    }

private:
    TypeSystem *types_;
    Job const *job_;
    Type return_type_;
    Type method_class_type_;
    ExecContext exec_ctx_;
    plugin::Plugin const *plugin_;
};
}  // namespace panda::verifier

#endif  // !PANDA_VERIFICATION_VERIFICATION_CONTEXT_HPP_
