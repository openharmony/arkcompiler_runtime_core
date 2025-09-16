/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "job_fill_gen.h"
#include "runtime/include/thread_scopes.h"
#include "verification/absint/absint.h"
#include "verification/cflow/cflow_check.h"
#include "verification/config/debug_breakpoint/breakpoint.h"
#include "verification/jobs/job.h"
#include "verification/public_internal.h"

namespace ark::verifier {
bool Job::UpdateTypes(TypeSystem *types) const
{
    bool result = true;
    Job::ErrorHandler handler;

    auto hasType = [&](Class const *klass) {
        types->MentionClass(klass);
        return true;
    };
    ForAllCachedTypes([&](Type type) {
        if (type.IsClass()) {
            result = result && hasType(type.GetClass());
        }
    });
    ForAllCachedMethods([&](Method const *method) { types->GetMethodSignature(method); });
    ForAllCachedFields([&](Field const *field) {
        result = result && hasType(field->GetClass());
        if (field->GetType().IsReference()) {
            ScopedChangeThreadStatus st(ManagedThread::GetCurrent(), ThreadStatus::RUNNING);
            auto typeId = panda_file::FieldDataAccessor::GetTypeId(*field->GetPandaFile(), field->GetFileId());
            auto *klass = GetClass(typeId, field->GetClass()->GetLoadContext(), field->GetPandaFile(), &handler);
            result = result && hasType(klass);
        }
    });
    return result;
}

bool Job::Verify(TypeSystem *types) const
{
    auto verifContext = PrepareVerificationContext(types, this);
    auto result = VerifyMethod(verifContext);
    return result != VerificationStatus::ERROR;
}

bool Job::DoChecks(TypeSystem *types)
{
    const auto &check = Options().Check();

    if (check[MethodOption::CheckType::RESOLVE_ID]) {
        if (!ResolveIdentifiers()) {
            LOG(WARNING, VERIFIER) << "Failed to resolve identifiers for method " << method_->GetFullName(true);
            return false;
        }
    }

    if (check[MethodOption::CheckType::CFLOW]) {
        auto cflowInfo = CheckCflow(method_);
        if (!cflowInfo) {
            LOG(WARNING, VERIFIER) << "Failed to check control flow for method " << method_->GetFullName(true);
            return false;
        }
        cflowInfo_ = std::move(cflowInfo);
    }

    DBG_MANAGED_BRK(&service_->debugCtx, method_->GetUniqId(), 0xFFFF);

    if (check[MethodOption::CheckType::TYPING]) {
        if (!UpdateTypes(types)) {
            LOG(WARNING, VERIFIER) << "Cannot update types from cached classes for method "
                                   << method_->GetFullName(true);
            return false;
        }
    }

    if (check[MethodOption::CheckType::ABSINT]) {
        if (!Verify(types)) {
            LOG(WARNING, VERIFIER) << "Abstract interpretation failed for method " << method_->GetFullName(true);
            return false;
        }
    }

    // Clean up temporary types
    types->ResetTypeSpans();

    return true;
}

void Job::ErrorHandler::OnError([[maybe_unused]] ClassLinker::Error error, PandaString const &message)
{
    LOG(ERROR, VERIFIER) << "Cannot link class: " << message;
}

Class *Job::GetClass(panda_file::File::EntityId classId, ClassLinkerContext *ctx, const panda_file::File *pfile,
                     ErrorHandler *errorHandler) const
{
    const auto *desc = pfile->GetStringData(classId).data;
    if (ClassHelper::IsUnionOrArrayUnionDescriptor(desc)) {
        return ClassHelper::GetUnionLUBClass(desc, classLinker_, ctx, classLinker_->GetExtension(langContext_),
                                             errorHandler);
    }
    return classLinker_->GetClass(desc, true, ctx, errorHandler);
}

Class *Job::GetClass(panda_file::File::EntityId classId, ErrorHandler *errorHandler) const
{
    return GetClass(classId, method_->GetClass()->GetLoadContext(), method_->GetPandaFile(), errorHandler);
}

Field *Job::GetField(panda_file::File::EntityId fieldIdx, bool isStatic, ErrorHandler *errorHandler) const
{
    Field *field = method_->GetPandaFile()->GetPandaCache()->GetFieldFromCache(fieldIdx);
    if (field != nullptr) {
        return field;
    }
    panda_file::FieldDataAccessor const fda(*method_->GetPandaFile(), fieldIdx);
    auto *klass =
        GetClass(fda.GetClassId(), method_->GetClass()->GetLoadContext(), method_->GetPandaFile(), errorHandler);
    return classLinker_->GetField(klass, fda, isStatic, errorHandler);
}

}  // namespace ark::verifier
