/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_MEM_TEST_UTILS_H
#define PANDA_RUNTIME_MEM_TEST_UTILS_H

#include "libarkbase/test_utilities.h"
#include "runtime/include/runtime.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/class_root.h"

namespace ark::mem {
constexpr std::initializer_list<const char *> TESTED_GC = {"stw", "g1-gc"};

[[maybe_unused]] inline ObjectHeader *AllocateNullifiedPayloadString(size_t length)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    ASSERT(vm != nullptr);
    auto *stringClass = Runtime::GetCurrent()
                            ->GetClassLinker()
                            ->GetExtension(vm->GetLanguageContext())
                            ->GetClassRoot(ClassRoot::LINE_STRING);
    ASSERT(stringClass != nullptr);
    mem::HeapManager *heapManager = vm->GetHeapManager();
    ASSERT(heapManager != nullptr);
    return heapManager->AllocateObject(stringClass, ark::coretypes::String::ComputeSizeUtf16(length));
}

inline ObjectHeader *AllocNonMovableObject()
{
    uint16_t data = 0;
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    return coretypes::String::CreateFromUtf16(&data, 0, ctx, runtime->GetPandaVM(), false);
}

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,-warnings-as-errors)
class ObjectAllocator {
public:
    static coretypes::Array *AllocArray(size_t length, ClassRoot classRoot, bool nonmovable, bool pinned = false)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT;
        auto *klass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(classRoot);
        if (nonmovable) {
            spaceType = SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT;
        }
        return coretypes::Array::Create(klass, length, spaceType, pinned);
    }

    static coretypes::String *AllocString(size_t length, bool pinned = false, bool movable = true)
    {
        ASSERT(movable || !pinned);
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        PandaVector<uint8_t> data;
        data.resize(length);
        return coretypes::String::CreateFromMUtf8(data.data(), length, length, true, ctx, runtime->GetPandaVM(),
                                                  movable, pinned);
    }

    static ObjectHeader *AllocObjectInYoung()
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    }
};

template <GCPhase PHASE>
class OnGCPhaseStartListener : public GCListener {
public:
    explicit OnGCPhaseStartListener(std::function<void(void)> action) : fn_(std::move(action)) {}

private:
    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase == PHASE) {
            fn_();
        }
    }

    std::function<void(void)> fn_;
};

template <GCPhase PHASE>
class OnGCPhaseFinishListener : public GCListener {
public:
    explicit OnGCPhaseFinishListener(std::function<void(void)> action) : fn_(std::move(action)) {}

private:
    void GCPhaseFinished(GCPhase phase) override
    {
        if (phase == PHASE) {
            fn_();
        }
    }

    std::function<void(void)> fn_;
};

template <GCPhase PHASE>
class OnGCPhaseSFListener : public GCListener {
public:
    explicit OnGCPhaseSFListener(std::function<void(void)> onStart, std::function<void(void)> onFinish)
        : start_(std::move(onStart)), fin_(std::move(onFinish))
    {
    }

private:
    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase == PHASE) {
            start_();
        }
    }

    void GCPhaseFinished(GCPhase phase) override
    {
        if (phase == PHASE) {
            fin_();
        }
    }

    std::function<void(void)> start_, fin_;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_TEST_UTILS_H
