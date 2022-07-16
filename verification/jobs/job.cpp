/*
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

#include "job_fill_gen.h"
#include "verification/absint/absint.h"
#include "verification/cflow/cflow_check.h"
#include "verification/config/debug_breakpoint/breakpoint.h"
#include "verification/jobs/job.h"

namespace panda::verifier {
bool Job::UpdateTypes(PandaTypes &types) const
{
    bool result = true;
    auto has_type = [&](const LibCache::CachedClass &klass) {
        bool is_valid = types.TypeOf(klass).IsValid();
        if (!is_valid) {
            LOG(WARNING, VERIFIER) << "Cached class " << klass.GetName() << " doesn't have a valid type in typesystem";
        }
        return is_valid;
    };
    ForAllCachedClasses([&](const LibCache::CachedClass &klass) { result = result && has_type(klass); });
    ForAllCachedMethods([&types](const LibCache::CachedMethod &method) { types.NormalizedMethodSignature(method); });
    ForAllCachedFields([&](const LibCache::CachedField &field) {
        result = result && has_type(field.klass) && has_type(LibCache::GetRef(field.type));
    });
    return result;
}

bool Job::Verify(PandaTypes &types) const
{
    auto verif_context = PrepareVerificationContext(types, *this);
    auto result = VerifyMethod(verif_context);
    return result != VerificationStatus::ERROR;
}

bool Job::DoChecks(LibCache &cache, PandaTypes &types)
{
    const auto &check = Options().Check();

    if (check[MethodOption::CheckType::RESOLVE_ID]) {
        if (!ResolveIdentifiers(cache)) {
            LOG(WARNING, VERIFIER) << "Failed to resolve identifiers for method " << method_.GetFullName(true);
            return false;
        }
    }

    if (check[MethodOption::CheckType::CFLOW]) {
        auto cflow_info = CheckCflow(cached_method_, Options(), cache);
        if (!cflow_info) {
            LOG(WARNING, VERIFIER) << "Failed to check control flow for method " << method_.GetFullName(true);
            return false;
        }
        cflow_info_ = std::move(cflow_info);
    }

    DBG_MANAGED_BRK(cached_method_.id, 0xFFFF);

    if (check[MethodOption::CheckType::TYPING]) {
        if (!UpdateTypes(types)) {
            LOG(WARNING, VERIFIER) << "Cannot update types from cached classes for method "
                                   << method_.GetFullName(true);
            return false;
        }
    }

    if (check[MethodOption::CheckType::ABSINT]) {
        if (!Verify(types)) {
            LOG(WARNING, VERIFIER) << "Abstract interpretation failed for method " << method_.GetFullName(true);
            return false;
        }
    }

    return true;
}
}  // namespace panda::verifier
