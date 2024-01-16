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
#ifndef PANDA_RUNTIME_TESTS_PYGOTE_SPACE_ALLOCATOR_TEST_H_
#define PANDA_RUNTIME_TESTS_PYGOTE_SPACE_ALLOCATOR_TEST_H_

#include <sys/mman.h>
#include <gtest/gtest.h>

#include "libpandabase/os/mem.h"
#include "libpandabase/utils/logger.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/refstorage/global_object_storage.h"

namespace ark::mem {

class PygoteSpaceAllocatorTest : public testing::Test {
public:
    NO_COPY_SEMANTIC(PygoteSpaceAllocatorTest);
    NO_MOVE_SEMANTIC(PygoteSpaceAllocatorTest);

    using PygoteAllocator = PygoteSpaceAllocator<ObjectAllocConfig>;

    PygoteSpaceAllocatorTest() = default;
    ~PygoteSpaceAllocatorTest() override = default;

protected:
    PygoteAllocator *GetPygoteSpaceAllocator()
    {
        return thread_->GetVM()->GetGC()->GetObjectAllocator()->GetPygoteSpaceAllocator();
    }

    Class *GetObjectClass()
    {
        auto runtime = ark::Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    }

    void PygoteFork()
    {
        thread_->ManagedCodeEnd();
        auto runtime = ark::Runtime::GetCurrent();
        runtime->PreZygoteFork();
        runtime->PostZygoteFork();
        thread_->ManagedCodeBegin();
    }

    void TriggerGc()
    {
        auto gc = thread_->GetVM()->GetGC();
        auto task = GCTask(GCTaskCause::EXPLICIT_CAUSE);
        // trigger tenured gc
        gc->WaitForGCInManaged(task);
        gc->WaitForGCInManaged(task);
        gc->WaitForGCInManaged(task);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    RuntimeOptions options_;

    void InitAllocTest();

    void ForkedAllocTest();

    void NonMovableLiveObjectAllocTest();

    void NonMovableUnliveObjectAllocTest();

    void MovableLiveObjectAllocTest();

    void MovableUnliveObjectAllocTest();

    void MuchObjectAllocTest();

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_ {nullptr};
};

inline void PygoteSpaceAllocatorTest::InitAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));

    pygoteSpaceAllocator->Free(nonMovableHeader);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
}

inline void PygoteSpaceAllocatorTest::ForkedAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();

    PygoteFork();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(movableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));
}

inline void PygoteSpaceAllocatorTest::NonMovableLiveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(nonMovableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));

    pygoteSpaceAllocator->Free(nonMovableHeader);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
}

inline void PygoteSpaceAllocatorTest::NonMovableUnliveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(nonMovableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
    globalObjectStorage->Remove(ref);

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
}

inline void PygoteSpaceAllocatorTest::MovableLiveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(movableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(movableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    auto obj = globalObjectStorage->Get(ref);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
}

inline void PygoteSpaceAllocatorTest::MovableUnliveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(movableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(movableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    auto obj = globalObjectStorage->Get(ref);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
    globalObjectStorage->Remove(ref);

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
}

inline void PygoteSpaceAllocatorTest::MuchObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    static constexpr size_t OBJ_NUM = 1024;

    PandaVector<Reference *> movableRefs;
    PandaVector<Reference *> nonMovableRefs;
    for (size_t i = 0; i < OBJ_NUM; i++) {
        auto movable = ark::ObjectHeader::Create(cls);
        movableRefs.push_back(globalObjectStorage->Add(movable, ark::mem::Reference::ObjectType::GLOBAL));
        auto nonMovable = ark::ObjectHeader::CreateNonMovable(cls);
        nonMovableRefs.push_back(globalObjectStorage->Add(nonMovable, ark::mem::Reference::ObjectType::GLOBAL));
    }

    PygoteFork();

    PandaVector<ObjectHeader *> movableObjs;
    PandaVector<ObjectHeader *> nonMovableObjs;
    for (auto movalbeRef : movableRefs) {
        auto obj = globalObjectStorage->Get(movalbeRef);
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
        ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
        globalObjectStorage->Remove(movalbeRef);
        movableObjs.push_back(obj);
    }

    for (auto nonMovalbeRef : nonMovableRefs) {
        auto obj = globalObjectStorage->Get(nonMovalbeRef);
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
        ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
        globalObjectStorage->Remove(nonMovalbeRef);
        nonMovableObjs.push_back(obj);
    }

    TriggerGc();

    for (auto movalbeObj : movableObjs) {
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movalbeObj)));
        ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(movalbeObj)));
    }

    for (auto nonMovalbeObj : nonMovableObjs) {
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovalbeObj)));
        ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovalbeObj)));
    }
}

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_TESTS_PYGOTE_SPACE_ALLOCATOR_TEST_H_
