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

#ifndef PANDA_VERIFIER_JOB_QUEUE_JOB_H_
#define PANDA_VERIFIER_JOB_QUEUE_JOB_H_

#include "verification/jobs/cache.h"
#include "verification/cflow/cflow_info.h"
#include "verification/verification_options.h"

#include "runtime/include/method.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace panda::verifier {
class Job {
public:
    Job(Method &method, const CachedMethod &cached_method, const MethodOptions &options)
        : method_ {method}, cached_method_ {cached_method}, options_ {options}
    {
    }

    ~Job() = default;

    NO_COPY_SEMANTIC(Job);

    bool IsFieldPresentForOffset(uint32_t offset) const
    {
        return fields_.count(offset) != 0;
    }

    bool IsMethodPresentForOffset(uint32_t offset) const
    {
        return methods_.count(offset) != 0;
    }

    bool IsClassPresentForOffset(uint32_t offset) const
    {
        return classes_.count(offset) != 0;
    }

    const CachedField &GetField(uint32_t offset) const
    {
        return fields_.at(offset);
    }

    const CachedMethod &GetMethod(uint32_t offset) const
    {
        return methods_.at(offset);
    }

    const CachedClass &GetClass(uint32_t offset) const
    {
        return classes_.at(offset);
    }

    const CachedMethod &JobCachedMethod() const
    {
        return cached_method_;
    }

    const CflowMethodInfo &JobMethodCflow() const
    {
        return *cflow_info_;
    }

    template <typename Handler>
    void ForAllCachedClasses(Handler &&handler) const
    {
        for (const auto &item : classes_) {
            handler(item.second.get());
        }
    }

    template <typename Handler>
    void ForAllCachedMethods(Handler &&handler) const
    {
        for (const auto &item : methods_) {
            handler(item.second.get());
        }
    }

    template <typename Handler>
    void ForAllCachedFields(Handler &&handler) const
    {
        for (const auto &item : fields_) {
            handler(item.second.get());
        }
    }

    const auto &Options() const
    {
        return options_;
    }

    bool DoChecks(LibCache &cache, PandaTypes &types);

private:
    Method &method_;
    const CachedMethod &cached_method_;
    const MethodOptions &options_;
    PandaUniquePtr<CflowMethodInfo> cflow_info_;

    // TODO(vdyadov): store file_id for double check during verification
    // offset -> cache item
    PandaUnorderedMap<uint32_t, std::reference_wrapper<const CachedField>> fields_;
    PandaUnorderedMap<uint32_t, std::reference_wrapper<const CachedMethod>> methods_;
    PandaUnorderedMap<uint32_t, std::reference_wrapper<const CachedClass>> classes_;

    bool ResolveIdentifiers(LibCache &cache);

    bool UpdateTypes(PandaTypes &types) const;

    bool Verify(PandaTypes &types) const;

    void AddField(uint32_t offset, const CachedField &cached_field)
    {
        fields_.emplace(offset, std::cref(cached_field));
    }

    void AddMethod(uint32_t offset, const CachedMethod &cached_method)
    {
        methods_.emplace(offset, std::cref(cached_method));
    }

    void AddClass(uint32_t offset, const CachedClass &cached_class)
    {
        classes_.emplace(offset, std::cref(cached_class));
    }
};
}  // namespace panda::verifier

#endif  //! PANDA_VERIFIER_JOB_QUEUE_JOB_H_
