/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_VERIFIER_JOBS_SERVICE_H
#define PANDA_VERIFIER_JOBS_SERVICE_H

#include "libarkfile/include/source_lang_enum.h"
#include "runtime/include/mem/panda_containers.h"

#include "verification/type/type_system.h"

namespace ark::verifier {

using SourceLang = panda_file::SourceLang;
class TaskProcessor;

class VerifierService final {
public:
    VerifierService(mem::InternalAllocatorPtr allocator, ClassLinker *linker) : allocator_ {allocator}, linker_ {linker}
    {
    }
    ~VerifierService() = default;

    NO_COPY_SEMANTIC(VerifierService);
    NO_MOVE_SEMANTIC(VerifierService);

    void Init();
    void Destroy();

    ClassLinker *GetClassLinker()
    {
        return linker_;
    }

    TaskProcessor *GetProcessor(ClassLinkerContext *ctx);
    void ReleaseProcessor(TaskProcessor *processor);

private:
    struct LoadContextData {
        explicit LoadContextData(ClassLinkerContext *ctx) : loadContext {ctx} {}

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        ClassLinkerContext *loadContext;
        size_t totalProcessors = 0;
        PandaDeque<TaskProcessor *> queue;
        // NOLINTEND(misc-non-private-member-variables-in-classes)
    };

    mem::InternalAllocatorPtr allocator_;
    ClassLinker *linker_;

    ark::os::memory::Mutex lock_;
    ark::os::memory::ConditionVariable condVar_ GUARDED_BY(lock_);
    PandaUnorderedMap<ClassLinkerContext *, LoadContextData> processors_ GUARDED_BY(lock_);

    bool shutdown_ {false};
};

class TaskProcessor final {
public:
    explicit TaskProcessor(VerifierService *service, ClassLinkerContext *ctx)
        : service_ {service}, loadContext_ {ctx} {};

    TypeSystem *GetTypeSystem()
    {
        return &typeSystem_;
    }

    void ResetTypeSystem()
    {
        this->typeSystem_ = TypeSystem {service_, loadContext_->GetSourceLang()};
    }

    ClassLinkerContext *GetLoadContext()
    {
        return loadContext_;
    }

private:
    VerifierService *service_;
    ClassLinkerContext *const loadContext_;
    TypeSystem typeSystem_ {service_, loadContext_->GetSourceLang()};
};

}  // namespace ark::verifier

#endif  //  PANDA_VERIFIER_JOBS_SERVICE_H
