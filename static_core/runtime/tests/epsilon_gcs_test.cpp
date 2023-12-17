/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "runtime/include/thread_scopes.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/gc/epsilon-g1/epsilon-g1.h"

#include "test_utils.h"

namespace panda::mem {

// NOLINTBEGIN(readability-magic-numbers)
class EpsilonGCTest : public testing::Test {
public:
    explicit EpsilonGCTest() : EpsilonGCTest(CreateDefaultOptions()) {}

    explicit EpsilonGCTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~EpsilonGCTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EpsilonGCTest);
    NO_MOVE_SEMANTIC(EpsilonGCTest);

    static RuntimeOptions CreateDefaultOptions(GCType gc_type = GCType::EPSILON_GC)
    {
        constexpr size_t HEAP_SIZE_LIMIT_FOR_TEST = 16_MB;
        RuntimeOptions options;
        switch (gc_type) {
            case GCType::EPSILON_GC:
                options.SetGcType("epsilon");
                break;
            case GCType::EPSILON_G1_GC:
                options.SetGcType("epsilon-g1");
                options.SetGcWorkersCount(0);
                options.SetG1PromotionRegionAliveRate(100);
                break;
            default:
                UNREACHABLE();
        }
        options.SetHeapSizeLimit(HEAP_SIZE_LIMIT_FOR_TEST);
        options.SetLoadRuntimes({"core"});
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        return options;
    }

    template <typename T>
    T *GetAllocator()
    {
        static_assert(std::is_same<T, ObjectAllocatorNoGen<>>::value || std::is_same<T, ObjectAllocatorG1<>>::value);
        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        return static_cast<T *>(gc->GetObjectAllocator());
    }

    template <typename T>
    std::vector<ObjectHeader *> AllocObjectsForTest()
    {
        std::vector<ObjectHeader *> obj_vector;

        obj_vector.emplace_back(ObjectAllocator::AllocArray(GetAllocator<T>()->GetRegularObjectMaxSize() * 0.8,
                                                            ClassRoot::ARRAY_U8, false));
        obj_vector.emplace_back(
            ObjectAllocator::AllocArray(GetAllocator<T>()->GetLargeObjectMaxSize(), ClassRoot::ARRAY_U8, false));
        obj_vector.emplace_back(
            ObjectAllocator::AllocArray(GetAllocator<T>()->GetRegularObjectMaxSize() * 0.5, ClassRoot::ARRAY_U8, true));
        obj_vector.emplace_back(ObjectAllocator::AllocString(GetAllocator<T>()->GetRegularObjectMaxSize() * 0.8));
        obj_vector.emplace_back(ObjectAllocator::AllocString(GetAllocator<T>()->GetLargeObjectMaxSize()));

        return obj_vector;
    }

    static constexpr size_t NUM_OF_ELEMS_CHECKED = 5;
};

class EpsilonG1GCTest : public EpsilonGCTest {
public:
    explicit EpsilonG1GCTest() : EpsilonGCTest(CreateDefaultOptions(GCType::EPSILON_G1_GC)) {}

    NO_COPY_SEMANTIC(EpsilonG1GCTest);
    NO_MOVE_SEMANTIC(EpsilonG1GCTest);

    ~EpsilonG1GCTest() override = default;

    static constexpr size_t GetHumongousStringLength()
    {
        // Total string size will be G1_REGION_SIZE + sizeof(String).
        // It is enough to make it humongous.
        return G1_REGION_SIZE;
    }

    static constexpr size_t YOUNG_OBJECT_SIZE =
        AlignUp(static_cast<size_t>(G1_REGION_SIZE * 0.8), DEFAULT_ALIGNMENT_IN_BYTES);
};

TEST_F(EpsilonGCTest, TestObjectsAllocation)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ASSERT_EQ(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetType(), GCType::EPSILON_GC);

    // Allocation of objects of different sizes in movable/non-movable spaces and checking that they are accessible
    std::vector<ObjectHeader *> allocated_objects = AllocObjectsForTest<ObjectAllocatorNoGen<>>();
    for (size_t i = 0; i < allocated_objects.size(); ++i) {
        ASSERT_NE(allocated_objects[i], nullptr);
        if (i < 3) {  // First 3 elements are coretypes::Array
            ASSERT_NE(static_cast<coretypes::Array *>(allocated_objects[i])->GetLength(), 0);
        } else {
            ASSERT_NE(static_cast<coretypes::String *>(allocated_objects[i])->GetLength(), 0);
        }
    }
}

TEST_F(EpsilonGCTest, TestOOMAndGCTriggering)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);
    VMHandle<coretypes::Array> nonmovable = VMHandle<coretypes::Array>(
        thread, ObjectAllocator::AllocArray(NUM_OF_ELEMS_CHECKED, ClassRoot::ARRAY_STRING, true));
    std::vector<VMHandle<coretypes::String>> strings;
    coretypes::String *obj_string;

    // Alloc objects until OOM
    // First NUM_OF_ELEMS_CHECKED objects are added to nonmovable array to check their addresses after triggered GC
    do {
        obj_string =
            ObjectAllocator::AllocString(GetAllocator<ObjectAllocatorNoGen<>>()->GetRegularObjectMaxSize() * 0.8);
        if (strings.size() < NUM_OF_ELEMS_CHECKED) {
            strings.emplace_back(thread, obj_string);
            size_t last_elem_indx = strings.size() - 1;
            nonmovable->Set<ObjectHeader *>(last_elem_indx, strings[last_elem_indx].GetPtr());
        }
    } while (obj_string != nullptr);
    VMHandle<coretypes::String> obj_after_oom = VMHandle<coretypes::String>(
        thread, ObjectAllocator::AllocString(GetAllocator<ObjectAllocatorNoGen<>>()->GetRegularObjectMaxSize() * 0.8));
    ASSERT_EQ(obj_after_oom.GetPtr(), nullptr) << "Expected OOM";
    ASSERT_EQ(strings.size(), NUM_OF_ELEMS_CHECKED);

    // Checking if the addresses are correct before triggering GC
    for (size_t i = 0; i < strings.size(); ++i) {
        ASSERT_EQ(strings[i].GetPtr(), nonmovable->Get<ObjectHeader *>(i));
    }

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::OOM_CAUSE);
        task.Run(*gc);
    }
    // Checking if the addresses are correct after triggering GC
    for (size_t i = 0; i < strings.size(); ++i) {
        ASSERT_EQ(strings[i].GetPtr(), nonmovable->Get<ObjectHeader *>(i));
    }

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);
    }
    // Checking if the addresses are correct after triggering GC
    for (size_t i = 0; i < strings.size(); ++i) {
        ASSERT_EQ(strings[i].GetPtr(), nonmovable->Get<ObjectHeader *>(i));
    }

    // Trying to alloc after triggering GC
    VMHandle<coretypes::String> obj_after_triggered_gc = VMHandle<coretypes::String>(
        thread, ObjectAllocator::AllocString(GetAllocator<ObjectAllocatorNoGen<>>()->GetRegularObjectMaxSize() * 0.8));
    ASSERT_EQ(obj_after_oom.GetPtr(), nullptr) << "Expected OOM";
}

TEST_F(EpsilonG1GCTest, TestObjectsAllocation)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ASSERT_EQ(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetType(), GCType::EPSILON_G1_GC);

    // Allocation of objects of different sizes in movable/non-movable spaces and checking that they are accessible and
    // are in correct regions
    std::vector<ObjectHeader *> allocated_objects = AllocObjectsForTest<ObjectAllocatorG1<>>();
    ASSERT_EQ(allocated_objects.size(), NUM_OF_ELEMS_CHECKED);
    for (auto &allocated_object : allocated_objects) {
        ASSERT_NE(allocated_object, nullptr);
    }

    ASSERT_TRUE(ObjectToRegion(allocated_objects[0])->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_FALSE(ObjectToRegion(allocated_objects[0])->HasFlag(RegionFlag::IS_NONMOVABLE));
    ASSERT_TRUE(ObjectToRegion(allocated_objects[1])->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ASSERT_TRUE(ObjectToRegion(allocated_objects[2])->HasFlag(RegionFlag::IS_NONMOVABLE));
    ASSERT_TRUE(ObjectToRegion(allocated_objects[3])->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(allocated_objects[4])->HasFlag(RegionFlag::IS_LARGE_OBJECT));
}

TEST_F(EpsilonG1GCTest, TestOOM)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);
    auto object_allocator_g1 = thread->GetVM()->GetGC()->GetObjectAllocator();
    VMHandle<coretypes::String> obj_string;
    VMHandle<coretypes::String> obj_string_huge;

    while (reinterpret_cast<GenerationalSpaces *>(object_allocator_g1->GetHeapSpace())->GetCurrentFreeYoungSize() >=
           YOUNG_OBJECT_SIZE) {
        obj_string = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(YOUNG_OBJECT_SIZE));
        ASSERT_NE(obj_string.GetPtr(), nullptr) << "Cannot allocate an object in young space when heap is not full";
    }
    obj_string = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(YOUNG_OBJECT_SIZE));
    ASSERT_EQ(obj_string.GetPtr(), nullptr) << "Expected OOM in young space";

    size_t huge_string_size = GetHumongousStringLength();
    size_t new_region_for_huge_string_size =
        Region::RegionSize(AlignUp(huge_string_size, GetAlignmentInBytes(DEFAULT_ALIGNMENT)), G1_REGION_SIZE);
    while (reinterpret_cast<GenerationalSpaces *>(object_allocator_g1->GetHeapSpace())->GetCurrentFreeTenuredSize() >
           new_region_for_huge_string_size) {
        obj_string_huge = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(huge_string_size));
        ASSERT_NE(obj_string_huge.GetPtr(), nullptr)
            << "Cannot allocate an object in tenured space when heap is not full";
    }
    obj_string_huge = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(huge_string_size));
    ASSERT_EQ(obj_string_huge.GetPtr(), nullptr) << "Expected OOM in tenured space";
}

TEST_F(EpsilonG1GCTest, TestGCTriggering)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);
    ASSERT_EQ(gc->GetType(), GCType::EPSILON_G1_GC);

    VMHandle<coretypes::Array> nonmovable;
    ObjectHeader *obj;
    ObjectHeader *huge_obj;
    uintptr_t obj_addr;
    uintptr_t huge_obj_addr;

    nonmovable = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(2, ClassRoot::ARRAY_STRING, true));
    obj = ObjectAllocator::AllocObjectInYoung();
    huge_obj = ObjectAllocator::AllocString(GetHumongousStringLength());
    nonmovable->Set(0, obj);
    nonmovable->Set(1, huge_obj);
    obj_addr = ToUintPtr(obj);
    huge_obj_addr = ToUintPtr(huge_obj);
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(huge_obj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(huge_obj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // Check obj was not propagated to tenured and huge_obj was not moved to any other region
    obj = nonmovable->Get<ObjectHeader *>(0);
    huge_obj = nonmovable->Get<ObjectHeader *>(1);
    ASSERT_EQ(obj_addr, ToUintPtr(obj));
    ASSERT_EQ(huge_obj_addr, ToUintPtr(huge_obj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(huge_obj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(huge_obj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);  // run full GC
        task.Run(*gc);
    }
    // Check obj was not propagated to tenured and huge_obj was not propagated to any other region after full gc
    obj = nonmovable->Get<ObjectHeader *>(0);
    huge_obj = nonmovable->Get<ObjectHeader *>(1);
    ASSERT_EQ(obj_addr, ToUintPtr(obj));
    ASSERT_EQ(huge_obj_addr, ToUintPtr(huge_obj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(huge_obj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(huge_obj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));

    // Check the objects are accessible
    ASSERT_NE(nullptr, obj->ClassAddr<Class>());
    ASSERT_NE(nullptr, huge_obj->ClassAddr<Class>());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::mem
