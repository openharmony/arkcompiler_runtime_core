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

#include <gtest/gtest.h>
#include <array>

#include "runtime/include/object_header.h"
#include "runtime/mem/tlab.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/region_space.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/gc/g1/g1-gc.h"

#include "test_utils.h"

namespace panda::mem {

class G1GCTest : public testing::Test {
public:
    explicit G1GCTest() : G1GCTest(CreateDefaultOptions()) {}

    explicit G1GCTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~G1GCTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(G1GCTest);
    NO_MOVE_SEMANTIC(G1GCTest);

    static RuntimeOptions CreateDefaultOptions()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("g1-gc");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetG1PromotionRegionAliveRate(100);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(false);
        options.SetG1NumberOfTenuredRegionsAtMixedCollection(2);
        return options;
    }

    static constexpr size_t GetHumongousStringLength()
    {
        // Total string size will be DEFAULT_REGION_SIZE + sizeof(String).
        // It is enough to make it humongous.
        return DEFAULT_REGION_SIZE;
    }

    static constexpr size_t StringLengthFitIntoRegion(size_t num_regions)
    {
        return num_regions * DEFAULT_REGION_SIZE - sizeof(coretypes::String) - Region::HeadSize();
    }

    static size_t GetHumongousArrayLength(ClassRoot class_root)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *array_class = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(class_root);
        EXPECT_TRUE(array_class->IsArrayClass());
        if (!array_class->IsArrayClass()) {
            return 0;
        }
        // Total array size will be DEFAULT_REGION_SIZE * elem_size + sizeof(Array).
        // It is enough to make it humongous.
        size_t elem_size = array_class->GetComponentSize();
        return DEFAULT_REGION_SIZE / elem_size + 1;
    }

    ObjectAllocatorG1<> *GetAllocator()
    {
        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        return static_cast<ObjectAllocatorG1<> *>(gc->GetObjectAllocator());
    }

    void ProcessDirtyCards(G1GC<PandaAssemblyLanguageConfig> *gc)
    {
        gc->EndConcurrentScopeRoutine();
        gc->ProcessDirtyCards();
        gc->StartConcurrentScopeRoutine();
    }
};

class RemSetChecker : public GCListener {
public:
    explicit RemSetChecker(GC *gc, ObjectHeader *obj, ObjectHeader *ref)
        : gc_(static_cast<G1GC<PandaAssemblyLanguageConfig> *>(gc)),
          obj_(MTManagedThread::GetCurrent(), obj),
          ref_(MTManagedThread::GetCurrent(), ref)
    {
    }

    void GCPhaseStarted([[maybe_unused]] GCPhase phase) override {}

    void GCPhaseFinished(GCPhase phase) override
    {
        if (phase == GCPhase::GC_PHASE_MARK_YOUNG) {
            // We don't do it in phase started because refs from remset will be collected at marking stage
            Check();
        }
        if (phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE) {
            // remset is not fully updated during mixed collection
            gc_->ProcessDirtyCards();
            Check();
        }
    }

private:
    void Check()
    {
        RemSet<> *remset = ObjectToRegion(ref_.GetPtr())->GetRemSet();
        ASSERT_NE(nullptr, remset);
        bool has_object = false;
        ObjectHeader *object = obj_.GetPtr();
        remset->IterateOverObjects([object, &has_object](ObjectHeader *obj) { has_object |= object == obj; });
        // remset is not fully updated during mixed collection, check set of dirty objects
        ASSERT_TRUE(has_object || gc_->HasRefFromRemset(object));
    }

private:
    G1GC<PandaAssemblyLanguageConfig> *gc_;
    VMHandle<ObjectHeader> obj_;
    VMHandle<ObjectHeader> ref_;
};

TEST_F(G1GCTest, TestAddrToRegion)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    size_t humongous_len = GetHumongousArrayLength(ClassRoot::ARRAY_U8);
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<ObjectHeader> young(thread, ObjectAllocator::AllocArray(0, ClassRoot::ARRAY_U8, false));
    ASSERT_NE(nullptr, young.GetPtr());
    VMHandle<ObjectHeader> nonmovable(thread, ObjectAllocator::AllocArray(0, ClassRoot::ARRAY_U8, true));
    ASSERT_NE(nullptr, nonmovable.GetPtr());
    VMHandle<ObjectHeader> humongous(thread, ObjectAllocator::AllocArray(humongous_len, ClassRoot::ARRAY_U8, false));
    ASSERT_NE(nullptr, humongous.GetPtr());

    Region *young_region = ObjectToRegion(young.GetPtr());
    ASSERT_NE(nullptr, young_region);
    ASSERT_EQ(young_region, AddrToRegion(young.GetPtr()));
    bool has_young_obj = false;
    young_region->IterateOverObjects(
        [&has_young_obj, &young](ObjectHeader *obj) { has_young_obj |= obj == young.GetPtr(); });
    ASSERT_TRUE(has_young_obj);

    Region *nonmovable_region = ObjectToRegion(nonmovable.GetPtr());
    ASSERT_NE(nullptr, nonmovable_region);
    ASSERT_EQ(nonmovable_region, AddrToRegion(nonmovable.GetPtr()));
    ASSERT_TRUE(nonmovable_region->GetLiveBitmap()->Test(nonmovable.GetPtr()));

    Region *humongous_region = ObjectToRegion(humongous.GetPtr());
    ASSERT_NE(nullptr, humongous_region);
    ASSERT_EQ(humongous_region, AddrToRegion(humongous.GetPtr()));
    ASSERT_EQ(humongous_region, AddrToRegion(ToVoidPtr(ToUintPtr(humongous.GetPtr()) + DEFAULT_REGION_SIZE)));
    bool has_humongous_obj = false;
    humongous_region->IterateOverObjects(
        [&has_humongous_obj, &humongous](ObjectHeader *obj) { has_humongous_obj |= obj == humongous.GetPtr(); });
    ASSERT_TRUE(has_humongous_obj);
}

TEST_F(G1GCTest, TestAllocHumongousArray)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ObjectHeader *obj =
        ObjectAllocator::AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_U8), ClassRoot::ARRAY_U8, false);
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
}

TEST_F(G1GCTest, NonMovable2YoungRef)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    static constexpr size_t ARRAY_LENGTH = 100;
    coretypes::Array *non_movable_obj = nullptr;
    uintptr_t prev_young_addr = 0;
    Class *klass = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    non_movable_obj = coretypes::Array::Create(klass, ARRAY_LENGTH, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    coretypes::String *young_obj = coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    non_movable_obj->Set(0, young_obj);
    prev_young_addr = ToUintPtr(young_obj);
    VMHandle<coretypes::Array> non_movable_obj_ptr(thread, non_movable_obj);

    // Trigger GC
    RemSetChecker listener(gc, non_movable_obj, non_movable_obj->Get<ObjectHeader *>(0));
    gc->AddListener(&listener);

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    auto young_obj_2 = static_cast<coretypes::String *>(non_movable_obj_ptr->Get<ObjectHeader *>(0));
    // Check GC has moved the young obj
    ASSERT_NE(prev_young_addr, ToUintPtr(young_obj_2));
    // Check young object is accessible
    ASSERT_EQ(0, young_obj_2->GetLength());
}

TEST_F(G1GCTest, Humongous2YoungRef)
{
    Runtime *runtime = Runtime::GetCurrent();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    uintptr_t prev_young_addr = 0;
    size_t array_length = GetHumongousArrayLength(ClassRoot::ARRAY_STRING);
    VMHandle<coretypes::Array> humongous_obj(thread,
                                             ObjectAllocator::AllocArray(array_length, ClassRoot::ARRAY_STRING, false));
    ObjectHeader *young_obj = ObjectAllocator::AllocObjectInYoung();
    humongous_obj->Set(0, young_obj);
    prev_young_addr = ToUintPtr(young_obj);

    // Trigger GC
    RemSetChecker listener(gc, humongous_obj.GetPtr(), humongous_obj->Get<ObjectHeader *>(0));
    gc->AddListener(&listener);

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    young_obj = static_cast<ObjectHeader *>(humongous_obj->Get<ObjectHeader *>(0));
    // Check GC has moved the young obj
    ASSERT_NE(prev_young_addr, ToUintPtr(young_obj));
    // Check the young object is accessible
    ASSERT_NE(nullptr, young_obj->ClassAddr<Class>());
}

TEST_F(G1GCTest, TestCollectTenured)
{
    Runtime *runtime = Runtime::GetCurrent();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);

    VMHandle<coretypes::Array> humongous;
    VMHandle<coretypes::Array> nonmovable;
    ObjectHeader *obj;
    uintptr_t obj_addr;

    humongous =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_STRING),
                                                                       ClassRoot::ARRAY_STRING, false));
    nonmovable = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(1, ClassRoot::ARRAY_STRING, true));
    obj = ObjectAllocator::AllocObjectInYoung();
    humongous->Set(0, obj);
    nonmovable->Set(0, obj);
    obj_addr = ToUintPtr(obj);

    RemSetChecker listener1(gc, humongous.GetPtr(), obj);
    RemSetChecker listener2(gc, nonmovable.GetPtr(), obj);
    gc->AddListener(&listener1);
    gc->AddListener(&listener2);
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // Check the obj obj was propagated to tenured
    obj = humongous->Get<ObjectHeader *>(0);
    ASSERT_NE(obj_addr, ToUintPtr(obj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_OLD));

    obj_addr = ToUintPtr(obj);
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::EXPLICIT_CAUSE);  // run full GC to collect all regions
        task1.Run(*gc);
    }

    // Check the tenured obj was propagated to another tenured region
    obj = humongous->Get<ObjectHeader *>(0);
    ASSERT_NE(obj_addr, ToUintPtr(obj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_OLD));

    // Check the objet is accessible
    ASSERT_NE(nullptr, obj->ClassAddr<Class>());
}

// test that we don't have remset from humongous space after we reclaim humongous object
TEST_F(G1GCTest, CheckRemsetToHumongousAfterReclaimHumongousObject)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope_for_young_obj(thread);

    // 1MB array
    static constexpr size_t HUMONGOUS_ARRAY_LENGTH = 262144LU;
    static constexpr size_t YOUNG_ARRAY_LENGTH = ((DEFAULT_REGION_SIZE - Region::HeadSize()) / 4U) - 16U;
    coretypes::Array *humongous_obj;
    coretypes::Array *young_arr;

    auto *gc = runtime->GetPandaVM()->GetGC();
    auto region_pred = []([[maybe_unused]] Region *r) { return true; };

    Class *klass;

    klass = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);

    young_arr = coretypes::Array::Create(klass, YOUNG_ARRAY_LENGTH);
    ASSERT_NE(young_arr, nullptr);
    auto *region = ObjectToRegion(young_arr);
    ASSERT_NE(region, nullptr);

    VMHandle<coretypes::Array> young_obj_ptr(thread, young_arr);
    GCTask task(GCTaskCause::EXPLICIT_CAUSE);
    {
        [[maybe_unused]] HandleScope<ObjectHeader *> scope_for_humongous_obj(thread);

        humongous_obj = coretypes::Array::Create(klass, HUMONGOUS_ARRAY_LENGTH);

        ASSERT_NE(humongous_obj, nullptr);
        // add humongous object to our remset
        humongous_obj->Set(0, young_obj_ptr.GetPtr());

        ASSERT_EQ(gc->GetType(), GCType::G1_GC);
        {
            VMHandle<coretypes::Array> humongous_obj_ptr(thread, humongous_obj);
            {
                ScopedNativeCodeThread sn(thread);
                task.Run(*gc);
            }

            auto array_region = ObjectToRegion(young_obj_ptr.GetPtr());
            PandaVector<Region *> regions;
            array_region->GetRemSet()->Iterate(
                region_pred, [&regions](Region *r, [[maybe_unused]] const MemRange &range) { regions.push_back(r); });
            ASSERT_EQ(1U, regions.size());  // we have reference only from 1 humongous space
            ASSERT_TRUE(regions[0]->HasFlag(IS_LARGE_OBJECT));
            ASSERT_EQ(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                      PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(regions[0]));
        }
    }
    /*
     * humongous object is dead now
     * need one fake GC because we marked humongous in concurrent in the first GC before we removed Scoped, need to
     * unmark it
     */
    {
        ScopedNativeCodeThread sn(thread);
        task.Run(*gc);
        task.Run(*gc);  // humongous object should be reclaimed
    }

    auto array_region = ObjectToRegion(young_obj_ptr.GetPtr());
    PandaVector<Region *> regions;
    array_region->GetRemSet()->Iterate(
        region_pred, [&regions](Region *r, [[maybe_unused]] const MemRange &range) { regions.push_back(r); });
    ASSERT_EQ(0U, regions.size());  // we have no references from the humongous space
}

class NewObjectsListener : public GCListener {
public:
    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        MTManagedThread *thread = MTManagedThread::GetCurrent();
        ScopedManagedCodeThread s(thread);

        // Allocate quite large object to make allocator to create a separate region
        // NOLINTNEXTLINE(readability-magic-numbers)
        size_t nonmovable_len = 9 * DEFAULT_REGION_SIZE / 10;
        ObjectHeader *dummy = ObjectAllocator::AllocArray(nonmovable_len, ClassRoot::ARRAY_U8, true);
        Region *dummy_region = ObjectToRegion(dummy);
        EXPECT_TRUE(dummy_region->HasFlag(RegionFlag::IS_NONMOVABLE));
        nonmovable_ =
            VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(nonmovable_len, ClassRoot::ARRAY_U8, true));
        Region *nonmovable_region = ObjectToRegion(nonmovable_.GetPtr());
        EXPECT_TRUE(nonmovable_region->HasFlag(RegionFlag::IS_NONMOVABLE));
        EXPECT_NE(nonmovable_region, dummy_region);
        nonmovable_mark_bitmap_addr_ = ToUintPtr(nonmovable_region->GetMarkBitmap());

        size_t humongous_len = G1GCTest::GetHumongousArrayLength(ClassRoot::ARRAY_U8);
        humongous_ =
            VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(humongous_len, ClassRoot::ARRAY_U8, false));
        Region *humongous_region = ObjectToRegion(humongous_.GetPtr());
        humongous_mark_bitmap_addr_ = ToUintPtr(humongous_region->GetMarkBitmap());
    }

    ObjectHeader *GetNonMovable()
    {
        ASSERT(nonmovable_.GetPtr() != nullptr);
        return nonmovable_.GetPtr();
    }

    uintptr_t GetNonMovableMarkBitmapAddr()
    {
        return nonmovable_mark_bitmap_addr_;
    }

    ObjectHeader *GetHumongous()
    {
        return humongous_.GetPtr();
    }

    uintptr_t GetHumongousMarkBitmapAddr()
    {
        return humongous_mark_bitmap_addr_;
    }

private:
    VMHandle<ObjectHeader> nonmovable_;
    uintptr_t nonmovable_mark_bitmap_addr_ {};
    VMHandle<ObjectHeader> humongous_;
    uintptr_t humongous_mark_bitmap_addr_ {};
};

// Test the new objects created during concurrent marking are alive
TEST_F(G1GCTest, TestNewObjectsSATB)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    NewObjectsListener listener;
    gc->AddListener(&listener);

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);  // threshold cause should trigger concurrent marking
        task.Run(*gc);
    }
    // nullptr means we cannot allocate an object or concurrent phase wasn't triggered or
    // the listener wasn't called.
    ASSERT_NE(nullptr, listener.GetNonMovable());
    ASSERT_NE(nullptr, listener.GetHumongous());

    // Check the objects are alive
    Region *nonmovable_region = ObjectToRegion(listener.GetNonMovable());
    ASSERT_NE(nullptr, nonmovable_region->GetLiveBitmap());
    ASSERT_TRUE(nonmovable_region->GetLiveBitmap()->Test(listener.GetNonMovable()));
    ASSERT_FALSE(listener.GetNonMovable()->IsMarkedForGC());  // mark should be done using mark bitmap
    Region *humongous_region = ObjectToRegion(listener.GetHumongous());
    ASSERT_NE(nullptr, humongous_region->GetLiveBitmap());
    ASSERT_TRUE(humongous_region->GetLiveBitmap()->Test(listener.GetHumongous()));
    ASSERT_FALSE(listener.GetHumongous()->IsMarkedForGC());  // mark should be done using mark bitmap
}

class CollectionSetChecker : public GCListener {
public:
    explicit CollectionSetChecker(ObjectAllocatorG1<> *allocator) : allocator_(allocator) {}

    void SetExpectedRegions(const std::initializer_list<Region *> &expected_regions)
    {
        expected_regions_ = expected_regions;
    }

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase == GCPhase::GC_PHASE_MARK_YOUNG) {
            EXPECT_EQ(expected_regions_, GetCollectionSet());
            expected_regions_.clear();
        }
    }

private:
    PandaSet<Region *> GetCollectionSet()
    {
        PandaSet<Region *> collection_set;
        for (Region *region : allocator_->GetAllRegions()) {
            if (region->HasFlag(RegionFlag::IS_COLLECTION_SET)) {
                collection_set.insert(region);
            }
        }
        return collection_set;
    }

private:
    ObjectAllocatorG1<> *allocator_;
    PandaSet<Region *> expected_regions_;
};

TEST_F(G1GCTest, TestGetCollectibleRegionsHasAllYoungRegions)
{
    // The object will occupy more than half of region.
    // So expect the allocator allocates a separate young region for each object.
    size_t young_len = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::Array);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocatorG1<> *allocator = GetAllocator();
    MTManagedThread *thread = MTManagedThread::GetCurrent();

    CollectionSetChecker checker(allocator);
    gc->AddListener(&checker);
    {
        ScopedManagedCodeThread s(thread);
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> young1;
        VMHandle<ObjectHeader> young2;
        VMHandle<ObjectHeader> young3;

        young1 = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(young_len, ClassRoot::ARRAY_U8, false));
        young2 = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(young_len, ClassRoot::ARRAY_U8, false));
        young3 = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(young_len, ClassRoot::ARRAY_U8, false));

        Region *yregion1 = ObjectToRegion(young1.GetPtr());
        Region *yregion2 = ObjectToRegion(young2.GetPtr());
        Region *yregion3 = ObjectToRegion(young3.GetPtr());
        // Check all 3 objects are in different regions
        ASSERT_NE(yregion1, yregion2);
        ASSERT_NE(yregion2, yregion3);
        ASSERT_NE(yregion1, yregion3);
        checker.SetExpectedRegions({yregion1, yregion2, yregion3});
    }
    GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
    task.Run(*gc);
}

TEST_F(G1GCTest, TestGetCollectibleRegionsHasAllRegionsInCaseOfFull)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocatorG1<> *allocator = GetAllocator();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<ObjectHeader> young;
    VMHandle<ObjectHeader> tenured;
    VMHandle<ObjectHeader> humongous;
    tenured = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young to tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    young = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());
    humongous = VMHandle<ObjectHeader>(
        thread, ObjectAllocator::AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_U8), ClassRoot::ARRAY_U8, false));

    Region *yregion = ObjectToRegion(young.GetPtr());
    [[maybe_unused]] Region *tregion = ObjectToRegion(tenured.GetPtr());
    [[maybe_unused]] Region *hregion = ObjectToRegion(humongous.GetPtr());

    CollectionSetChecker checker(allocator);
    gc->AddListener(&checker);
    // Even thou it's full, currently we split it into two parts, the 1st one is young-only collection.
    // And the tenured collection part doesn't use GC_PHASE_MARK_YOUNG
    checker.SetExpectedRegions({yregion});
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::EXPLICIT_CAUSE);
        task1.Run(*gc);
    }
}

TEST_F(G1GCTest, TestMixedCollections)
{
    uint32_t garbage_rate = Runtime::GetOptions().GetG1RegionGarbageRateThreshold();
    // The object will occupy more than half of region.
    // So expect the allocator allocates a separate young region for each object.
    static constexpr size_t ARRAY_SIZE = 4;
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t big_len = garbage_rate * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t big_len1 = (garbage_rate + 1) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t big_len2 = (garbage_rate + 2) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    size_t small_len = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::String);
    std::array<size_t, ARRAY_SIZE> lenths_array {big_len, big_len1, big_len2, small_len};
    size_t mini_obj_len = PANDA_TLAB_SIZE + 1;  // To allocate not in TLAB

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocatorG1<> *allocator = GetAllocator();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> small_object_holder;
    VMHandle<coretypes::Array> big_object_holder;
    VMHandle<ObjectHeader> young;

    // Allocate objects of different sizes.
    // Allocate mini object after each of them for prevent clearing after concurrent
    // Mixed regions should be choosen according to the largest garbage.
    big_object_holder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(4, ClassRoot::ARRAY_STRING, false));
    small_object_holder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(4, ClassRoot::ARRAY_STRING, false));
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        big_object_holder->Set(i, ObjectAllocator::AllocString(lenths_array[i]));
        small_object_holder->Set(i, ObjectAllocator::AllocString(mini_obj_len));
        Region *first_region = ObjectToRegion(big_object_holder->Get<ObjectHeader *>(i));
        Region *second_region = ObjectToRegion(small_object_holder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(first_region->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(second_region->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(first_region == second_region);
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young objects -> tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // GC doesn't include current tenured region to the collection set.
    // Now we don't know which tenured region is current.
    // So propagate one big young object to tenured to make the latter current.
    VMHandle<ObjectHeader> current;
    current = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(small_len, ClassRoot::ARRAY_U8, false));

    // Propogate 'current' object -> tenured and prepare for mixed GC
    // Release 'big1', 'big2' and 'small' objects to make them garbage
    Region *region0 = ObjectToRegion(big_object_holder->Get<ObjectHeader *>(0));
    Region *region1 = ObjectToRegion(big_object_holder->Get<ObjectHeader *>(1));
    Region *region2 = ObjectToRegion(big_object_holder->Get<ObjectHeader *>(2));
    Region *region3 = ObjectToRegion(big_object_holder->Get<ObjectHeader *>(3));
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        big_object_holder->Set(i, static_cast<ObjectHeader *>(nullptr));
    }
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task1.Run(*gc);
    }

    // Now the region with 'current' is current and it will not be included into the collection set.

    young = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());

    Region *yregion = ObjectToRegion(young.GetPtr());
    CollectionSetChecker checker(allocator);
    gc->AddListener(&checker);
    checker.SetExpectedRegions({region1, region2, yregion});
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task2(GCTaskCause::YOUNG_GC_CAUSE);  // should run mixed GC
        task2.Run(*gc);
    }

    // Run GC one more time because we still have garbage regions.
    // Check we collect them.
    young = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());
    yregion = ObjectToRegion(young.GetPtr());
    checker.SetExpectedRegions({region0, yregion, region3});
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task3(GCTaskCause::YOUNG_GC_CAUSE);  // should run mixed GC
        task3.Run(*gc);
    }
}

TEST_F(G1GCTest, TestHandlePendingCards)
{
    auto thread = MTManagedThread::GetCurrent();
    auto runtime = Runtime::GetCurrent();
    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto array_class = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
    auto gc = runtime->GetPandaVM()->GetGC();
    size_t elem_size = array_class->GetComponentSize();
    size_t array_size = DEFAULT_REGION_SIZE / 2;
    // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
    size_t array_length = array_size / elem_size + 1;
    ScopedManagedCodeThread s(thread);
    HandleScope<ObjectHeader *> scope(thread);

    constexpr size_t REGION_NUM = 16;
    std::vector<VMHandle<coretypes::Array>> arrays;

    for (size_t i = 0; i < REGION_NUM; i++) {
        arrays.emplace_back(thread, ObjectAllocator::AllocArray(array_length, ClassRoot::ARRAY_STRING, false));
    }

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    for (auto &array : arrays) {
        ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(IS_OLD));
    }

    std::vector<VMHandle<coretypes::String>> strings;
    std::vector<void *> string_orig_ptrs;

    for (auto &array : arrays) {
        auto str = ObjectAllocator::AllocString(StringLengthFitIntoRegion(1));
        strings.emplace_back(thread, str);
        string_orig_ptrs.push_back(str);
        array->Set(0, str);  // create dirty card
    }

    // With high probability update_remset_worker could not process all dirty cards before GC
    // so GC drains them from update_remset_worker and handles separately
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    for (size_t i = 0; i < REGION_NUM; i++) {
        auto &array = arrays[i];
        auto &str = strings[i];
        auto str_orig_ptr = string_orig_ptrs[i];
        ASSERT_NE(str_orig_ptr, str.GetPtr());                   // string was moved
        ASSERT_EQ(array->Get<ObjectHeader *>(0), str.GetPtr());  // refs were correctly updated
    }

    // dirty cards corresponding to dirty_regions_objects should be reenqueued
    ProcessDirtyCards(static_cast<G1GC<PandaAssemblyLanguageConfig> *>(gc));
    for (size_t i = 0; i < REGION_NUM; i++) {
        auto &array = arrays[i];
        auto &str = strings[i];
        bool found = false;
        ObjectToRegion(str.GetPtr())->GetRemSet()->IterateOverObjects([&found, &array](ObjectHeader *obj) {
            if (obj == array.GetPtr()) {
                found = true;
            }
        });
        ASSERT_TRUE(found);
    }
}

class G1GCPromotionTest : public G1GCTest {
public:
    G1GCPromotionTest() : G1GCTest(CreateOptions()) {}

    static RuntimeOptions CreateOptions()
    {
        RuntimeOptions options = CreateDefaultOptions();
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetG1PromotionRegionAliveRate(50);
        return options;
    }

    static constexpr size_t PROMOTE_RATE = 50;
};

TEST_F(G1GCPromotionTest, TestCorrectPromotionYoungRegion)
{
    // We will create a humongous object with a links to two young regions
    // and check promotion workflow
    static constexpr size_t HUMONGOUS_STRING_LEN = G1GCPromotionTest::GetHumongousStringLength();
    // Consume more than 50% of region size
    static constexpr size_t FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT =
        DEFAULT_REGION_SIZE / sizeof(coretypes::String) * 2 / 3 + 1;
    // Consume less than 50% of region size
    static constexpr size_t SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT = 1;
    ASSERT(FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT <= HUMONGOUS_STRING_LEN);
    ASSERT((FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT * sizeof(coretypes::String) * 100 / DEFAULT_REGION_SIZE) >
           G1GCPromotionTest::PROMOTE_RATE);
    ASSERT((SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT * sizeof(coretypes::String) * 100 / DEFAULT_REGION_SIZE) <
           G1GCPromotionTest::PROMOTE_RATE);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    // Run Full GC to compact all existed young regions:
    GCTask task0(GCTaskCause::EXPLICIT_CAUSE);
    task0.Run(*gc);

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> first_holder;
    VMHandle<coretypes::Array> second_holder;
    VMHandle<ObjectHeader> young;
    std::array<ObjectHeader *, FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT> first_region_object_links {};
    std::array<ObjectHeader *, SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT> second_region_object_links {};
    // Check Promotion for young region:

    first_holder = VMHandle<coretypes::Array>(
        thread, ObjectAllocator::AllocArray(HUMONGOUS_STRING_LEN, ClassRoot::ARRAY_STRING, false));
    Region *first_region = ObjectToRegion(ObjectAllocator::AllocObjectInYoung());
    ASSERT_TRUE(first_region->HasFlag(RegionFlag::IS_EDEN));
    for (size_t i = 0; i < FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        first_region_object_links[i] = ObjectAllocator::AllocObjectInYoung();
        ASSERT_TRUE(first_region_object_links[i] != nullptr);
        first_holder->Set(i, first_region_object_links[i]);
        ASSERT_TRUE(ObjectToRegion(first_region_object_links[i]) == first_region);
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Promote young objects in one region -> tenured
        GCTask task1(GCTaskCause::YOUNG_GC_CAUSE);
        task1.Run(*gc);
    }
    // Check that we didn't change the links for young objects from the first region:
    for (size_t i = 0; i < FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        ASSERT_EQ(first_region_object_links[i], first_holder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(ObjectToRegion(first_holder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(first_holder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_PROMOTED));
    }

    second_holder = VMHandle<coretypes::Array>(
        thread, ObjectAllocator::AllocArray(HUMONGOUS_STRING_LEN, ClassRoot::ARRAY_STRING, false));
    Region *second_region = ObjectToRegion(ObjectAllocator::AllocObjectInYoung());
    ASSERT_TRUE(second_region->HasFlag(RegionFlag::IS_EDEN));
    for (size_t i = 0; i < SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        second_region_object_links[i] = ObjectAllocator::AllocObjectInYoung();
        ASSERT_TRUE(second_region_object_links[i] != nullptr);
        second_holder->Set(i, second_region_object_links[i]);
        ASSERT_TRUE(ObjectToRegion(second_region_object_links[i]) == second_region);
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Compact young objects in one region -> tenured
        GCTask task2(GCTaskCause::YOUNG_GC_CAUSE);
        task2.Run(*gc);
    }
    // Check that we changed the links for young objects from the second region:
    for (size_t i = 0; i < SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        ASSERT_NE(second_region_object_links[i], second_holder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(ObjectToRegion(second_holder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(second_holder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_PROMOTED));
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Run Full GC to compact all tenured regions:
        GCTask task3(GCTaskCause::EXPLICIT_CAUSE);
        task3.Run(*gc);
    }
    // Now we should have updated links in the humongous object to first region objects:
    for (size_t i = 0; i < FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        ASSERT_NE(first_region_object_links[i], first_holder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(ObjectToRegion(first_holder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(first_holder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_PROMOTED));
    }
}

class PromotionRemSetChecker : public GCListener {
public:
    PromotionRemSetChecker(VMHandle<coretypes::Array> *array, VMHandle<coretypes::String> *string)
        : array_(array), string_(string)
    {
    }

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK_YOUNG) {
            return;
        }
        // Before marking young all remsets must by actual
        CheckRemSets();
    }

    bool CheckRemSets()
    {
        Region *ref_region = ObjectToRegion(string_->GetPtr());
        found_ = false;
        ref_region->GetRemSet()->IterateOverObjects([this](ObjectHeader *obj) {
            if (obj == array_->GetPtr()) {
                found_ = true;
            }
        });
        return found_;
    }

    bool IsFound() const
    {
        return found_;
    }

private:
    VMHandle<coretypes::Array> *array_;
    VMHandle<coretypes::String> *string_;
    bool found_ = false;
};

TEST_F(G1GCPromotionTest, TestPromotedRegionHasValidRemSets)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    GC *gc = runtime->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::String> string(thread, ObjectAllocator::AllocString(1));
    ASSERT_TRUE(ObjectToRegion(string.GetPtr())->IsYoung());
    {
        ScopedNativeCodeThread sn(thread);
        // Move string to tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    ASSERT_TRUE(ObjectToRegion(string.GetPtr())->HasFlag(IS_OLD));

    // Allocate an array which ocuppies more than half region.
    // This array will be promoted.
    auto *array_class = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
    size_t elem_size = array_class->GetComponentSize();
    size_t array_size = DEFAULT_REGION_SIZE / 2;
    size_t array_length = array_size / elem_size + 1;
    VMHandle<coretypes::Array> array(thread, ObjectAllocator::AllocArray(array_length, ClassRoot::ARRAY_STRING, false));
    ASSERT_FALSE(array->IsForwarded());
    Region *array_region = ObjectToRegion(array.GetPtr());
    ASSERT_TRUE(array_region->IsYoung());
    array->Set(0, string.GetPtr());

    PromotionRemSetChecker listener(&array, &string);
    gc->AddListener(&listener);
    {
        ScopedNativeCodeThread sn(thread);
        // Promote array's regions to tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
        ASSERT_FALSE(listener.IsFound());
    }
    // Check the array was promoted.
    ASSERT_TRUE(array_region == ObjectToRegion(array.GetPtr()));

    // remset is not fully updated during mixed collection
    ProcessDirtyCards(static_cast<G1GC<PandaAssemblyLanguageConfig> *>(gc));
    // Check remsets
    ASSERT_TRUE(listener.CheckRemSets());
}

class InterruptGCListener : public GCListener {
public:
    explicit InterruptGCListener(VMHandle<coretypes::Array> *array) : array_(array) {}

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
        {
            ScopedManagedCodeThread s(ManagedThread::GetCurrent());
            // Allocate an object to add it into SATB buffer
            ObjectAllocator::AllocObjectInYoung();
        }
        // Set interrupt flag
        gc->OnWaitForIdleFail();
    }

    void GCPhaseFinished(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        Region *region = ObjectToRegion((*array_)->Get<ObjectHeader *>(0));
        // Check the object array[0] is not marked
        EXPECT_FALSE(region->GetMarkBitmap()->Test((*array_)->Get<ObjectHeader *>(0)));
        // Check GC haven't calculated live bytes for the region
        EXPECT_EQ(0, region->GetLiveBytes());
        // Check GC has cleared SATB buffer
        MTManagedThread *thread = MTManagedThread::GetCurrent();
        EXPECT_NE(nullptr, thread->GetPreBuff());
        EXPECT_EQ(0, thread->GetPreBuff()->size());
    }

private:
    VMHandle<coretypes::Array> *array_;
};

TEST_F(G1GCTest, TestInterruptConcurrentMarking)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> array;

    array = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(1, ClassRoot::ARRAY_STRING, false));
    array->Set(0, ObjectAllocator::AllocString(1));

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young objects -> tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);

        // Clear live bytes to check that concurrent marking will not calculate them
        Region *region = ObjectToRegion(array->Get<ObjectHeader *>(0));
        ASSERT_TRUE(region != nullptr);
        region->SetLiveBytes(0);

        InterruptGCListener listener(&array);
        gc->AddListener(&listener);
        // Trigger concurrent marking
        GCTask task1(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task1.Run(*gc);
    }
}

class NullRefListener : public GCListener {
public:
    explicit NullRefListener(VMHandle<coretypes::Array> *array) : array_(array) {}

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        (*array_)->Set(0, static_cast<ObjectHeader *>(nullptr));
    }

private:
    VMHandle<coretypes::Array> *array_;
};

TEST_F(G1GCTest, TestGarbageBytesCalculation)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> array;

    // Allocate objects of different sizes.
    // Mixed regions should be choosen according to the largest garbage.
    // Allocate an array of length 2. 2 because the array's size must be 8 bytes aligned
    array = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(2, ClassRoot::ARRAY_STRING, false));
    ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(RegionFlag::IS_EDEN));
    // The same for string. The instance size must be 8-bytes aligned.
    array->Set(0, ObjectAllocator::AllocString(8));
    array->Set(1, ObjectAllocator::AllocString(8));
    ASSERT_TRUE(ObjectToRegion(array->Get<ObjectHeader *>(0))->HasFlag(RegionFlag::IS_EDEN));

    size_t array_size = GetObjectSize(array.GetPtr());
    size_t str_size = GetObjectSize(array->Get<ObjectHeader *>(0));

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young objects -> tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // check the array and the string are in the same tenured region
    ASSERT_EQ(ObjectToRegion(array.GetPtr()), ObjectToRegion(array->Get<ObjectHeader *>(0)));
    ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(RegionFlag::IS_OLD));

    ObjectAllocator::AllocObjectInYoung();
    array->Set(1, static_cast<ObjectHeader *>(nullptr));

    NullRefListener listener(&array);
    gc->AddListener(&listener);
    {
        ScopedNativeCodeThread sn(thread);
        // Prepare for mixed GC, start concurrent marking and calculate garbage for regions
        GCTask task2(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task2.Run(*gc);
    }

    Region *region = ObjectToRegion(array.GetPtr());
    ASSERT_EQ(array_size + str_size, region->GetLiveBytes());
    ASSERT_EQ(str_size, region->GetGarbageBytes());
}

TEST_F(G1GCTest, NonMovableClearingDuringConcurrentPhaseTest)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto obj_allocator = Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator();
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    // NOLINTBEGIN(readability-magic-numbers)
    size_t array_length = GetHumongousArrayLength(ClassRoot::ARRAY_STRING) - 50;
    coretypes::Array *first_non_movable_obj = nullptr;
    coretypes::Array *second_non_movable_obj = nullptr;
    uintptr_t prev_young_addr = 0;

    Class *klass = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    first_non_movable_obj = coretypes::Array::Create(klass, array_length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    second_non_movable_obj = coretypes::Array::Create(klass, array_length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ASSERT_EQ(true, ObjectToRegion(first_non_movable_obj)->HasFlag(RegionFlag::IS_NONMOVABLE));
    ASSERT_EQ(true, ObjectToRegion(second_non_movable_obj)->HasFlag(RegionFlag::IS_NONMOVABLE));
    coretypes::String *young_obj = coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    first_non_movable_obj->Set(0, young_obj);
    prev_young_addr = ToUintPtr(young_obj);

    VMHandle<coretypes::Array> second_non_movable_obj_ptr(thread, second_non_movable_obj);

    {
        [[maybe_unused]] HandleScope<ObjectHeader *> first_scope(thread);
        VMHandle<coretypes::Array> first_non_movable_obj_ptr(thread, first_non_movable_obj);
        {
            ScopedNativeCodeThread sn(thread);
            GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
            task.Run(*gc);
        }

        auto young_obj_2 = static_cast<coretypes::String *>(first_non_movable_obj_ptr->Get<ObjectHeader *>(0));
        // Check GC has moved the young obj
        ASSERT_NE(prev_young_addr, ToUintPtr(young_obj_2));
        // Check young object is accessible
        ASSERT_EQ(0, young_obj_2->GetLength());
    }

    // Check that all objects are alive
    ASSERT_EQ(true, obj_allocator->ContainObject(first_non_movable_obj));
    ASSERT_EQ(true, obj_allocator->ContainObject(second_non_movable_obj));
    ASSERT_EQ(true, obj_allocator->IsLive(first_non_movable_obj));
    ASSERT_EQ(true, obj_allocator->IsLive(second_non_movable_obj));
    // Check that the first object is accessible
    bool found_first_object = false;
    obj_allocator->IterateOverObjects([&first_non_movable_obj, &found_first_object](ObjectHeader *object) {
        if (first_non_movable_obj == object) {
            found_first_object = true;
        }
    });
    ASSERT_EQ(true, found_first_object);

    // So, try to remove the first non movable object:
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task.Run(*gc);
    }

    // Check that the second object is still alive
    ASSERT_EQ(true, obj_allocator->ContainObject(second_non_movable_obj));
    ASSERT_EQ(true, obj_allocator->IsLive(second_non_movable_obj));
    // Check that the first object is dead
    obj_allocator->IterateOverObjects(
        [&first_non_movable_obj](ObjectHeader *object) { ASSERT_NE(first_non_movable_obj, object); });
}

TEST_F(G1GCTest, HumongousClearingDuringConcurrentPhaseTest)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto obj_allocator = Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator();
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    size_t array_length = GetHumongousArrayLength(ClassRoot::ARRAY_STRING);
    coretypes::Array *first_humongous_obj = nullptr;
    coretypes::Array *second_humongous_obj = nullptr;
    uintptr_t prev_young_addr = 0;

    Class *klass = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    first_humongous_obj = coretypes::Array::Create(klass, array_length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    second_humongous_obj = coretypes::Array::Create(klass, array_length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ASSERT_EQ(true, ObjectToRegion(first_humongous_obj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ASSERT_EQ(true, ObjectToRegion(second_humongous_obj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    coretypes::String *young_obj = coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    first_humongous_obj->Set(0, young_obj);
    prev_young_addr = ToUintPtr(young_obj);

    VMHandle<coretypes::Array> second_humongous_obj_ptr(thread, second_humongous_obj);

    {
        HandleScope<ObjectHeader *> first_scope(thread);
        VMHandle<coretypes::Array> first_humongous_obj_ptr(thread, first_humongous_obj);
        {
            ScopedNativeCodeThread sn(thread);
            GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
            task.Run(*gc);
        }

        auto young_obj_2 = static_cast<coretypes::String *>(first_humongous_obj_ptr->Get<ObjectHeader *>(0));
        // Check GC has moved the young obj
        ASSERT_NE(prev_young_addr, ToUintPtr(young_obj_2));
        // Check young object is accessible
        ASSERT_EQ(0, young_obj_2->GetLength());
    }

    // Check that all objects are alive
    ASSERT_EQ(true, obj_allocator->ContainObject(first_humongous_obj));
    ASSERT_EQ(true, obj_allocator->ContainObject(second_humongous_obj));
    ASSERT_EQ(true, obj_allocator->IsLive(first_humongous_obj));
    ASSERT_EQ(true, obj_allocator->IsLive(second_humongous_obj));
    // Check that the first object is accessible
    bool found_first_object = false;
    obj_allocator->IterateOverObjects([&first_humongous_obj, &found_first_object](ObjectHeader *object) {
        if (first_humongous_obj == object) {
            found_first_object = true;
        }
    });
    ASSERT_EQ(true, found_first_object);

    {
        ScopedNativeCodeThread sn(thread);
        // So, try to remove the first non movable object:
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task.Run(*gc);
    }

    // Check that the second object is still alive
    ASSERT_EQ(true, obj_allocator->ContainObject(second_humongous_obj));
    ASSERT_EQ(true, obj_allocator->IsLive(second_humongous_obj));
    // Check that the first object is dead
    obj_allocator->IterateOverObjects(
        [&first_humongous_obj](ObjectHeader *object) { ASSERT_NE(first_humongous_obj, object); });
}

class G1FullGCTest : public G1GCTest {
public:
    explicit G1FullGCTest(uint32_t full_gc_region_fragmentation_rate = 0)
        : G1GCTest(CreateOptions(full_gc_region_fragmentation_rate))
    {
    }

    static RuntimeOptions CreateOptions(uint32_t full_gc_region_fragmentation_rate)
    {
        RuntimeOptions options = CreateDefaultOptions();
        options.SetInitYoungSpaceSize(YOUNG_SIZE);
        options.SetYoungSpaceSize(YOUNG_SIZE);
        options.SetHeapSizeLimit(HEAP_SIZE);
        options.SetG1FullGcRegionFragmentationRate(full_gc_region_fragmentation_rate);
        return options;
    }

    static constexpr size_t NumYoungRegions()
    {
        return YOUNG_SIZE / DEFAULT_REGION_SIZE;
    }

    static constexpr size_t NumRegions()
    {
        // Region count without reserved region for Full GC
        return HEAP_SIZE / DEFAULT_REGION_SIZE - 1U;
    }

    size_t RefArrayLengthFitIntoRegion(size_t num_regions)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *klass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
        size_t elem_size = klass->GetComponentSize();
        // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
        return (num_regions * DEFAULT_REGION_SIZE - sizeof(coretypes::Array) - Region::HeadSize()) / elem_size;
    }

    void FillHeap(size_t num_regions, VMHandle<coretypes::Array> &holder, size_t start_index)
    {
        constexpr size_t STRING_LENGTH = StringLengthFitIntoRegion(1);
        EXPECT_LE(num_regions, holder->GetLength());
        for (size_t i = 0; i < num_regions; ++i) {
            ObjectHeader *obj = ObjectAllocator::AllocString(STRING_LENGTH);
            EXPECT_NE(nullptr, obj);
            holder->Set(start_index + i, obj);
        }
    }

    static constexpr size_t YOUNG_SIZE = 1_MB;
    static constexpr size_t HEAP_SIZE = 4_MB;
    static constexpr size_t NUM_NONMOVABLE_REGIONS_FOR_RUNTIME = 1;
};

TEST_F(G1FullGCTest, TestFullGCCollectsNonRegularObjects)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ObjectHeader *humongous_obj = ObjectAllocator::AllocString(GetHumongousStringLength());
    ObjectHeader *nonmovable_obj = AllocNonMovableObject();

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);
    }

    PandaVector<Region *> nonregular_regions = GetAllocator()->GetNonRegularRegions();
    for (Region *region : nonregular_regions) {
        if (region->HasFlag(IS_LARGE_OBJECT)) {
            ASSERT_NE(humongous_obj, region->GetLargeObject());
        } else if (region->HasFlag(IS_NONMOVABLE)) {
            if (region->Begin() <= ToUintPtr(nonmovable_obj) && ToUintPtr(nonmovable_obj) < region->End()) {
                ASSERT_FALSE(region->GetLiveBitmap()->Test(nonmovable_obj));
            }
        } else {
            FAIL() << "Unknown region type";
        }
    }
}

TEST_F(G1FullGCTest, TestFullGCFreeHumongousBeforeTenuredCollection)
{
    constexpr size_t NUM_REGIONS_FOR_HUMONGOUS = 4;

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    coretypes::String *humongous_obj =
        ObjectAllocator::AllocString(StringLengthFitIntoRegion(NUM_REGIONS_FOR_HUMONGOUS));
    holder->Set(0, humongous_obj);
    size_t num_free_regions =
        NumRegions() - NumYoungRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME - NUM_REGIONS_FOR_HUMONGOUS;
    FillHeap(num_free_regions, holder, 1);  // occupy 4 tenured regions and 3 young regions
    // move 3 young regions to tenured space.
    gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
    // now tenured space is full. Fill young space.
    FillHeap(NumYoungRegions(), holder, num_free_regions + 1);
    // At this point we have filled 4 tenured regions and 4 young regions and 3 free tenured regions.
    // We cannot do young GC because there are not enough free regions in tenured to move 4 young regions.
    // Check we are OOM
    ASSERT_EQ(nullptr, ObjectAllocator::AllocObjectInYoung());
    // Forget humongous_obj
    holder->Set(0, static_cast<ObjectHeader *>(nullptr));
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // We should have 2 free regions but during allocation we reserve 1 region for full GC.
    // So we can allocate only one region.
    ASSERT_NE(nullptr, ObjectAllocator::AllocObjectInYoung());
}

TEST_F(G1FullGCTest, TestRemSetsAndYoungCardsAfterFailedFullGC)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *card_table = gc->GetCardTable();
    ASSERT_NE(nullptr, card_table);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    size_t num_free_regions = NumRegions() - NumYoungRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME + 1U;
    FillHeap(num_free_regions, holder, 0);  // occupy 8 tenured regions and 3 young regions
    VMHandle<coretypes::Array> young_array(
        thread, ObjectAllocator::AllocArray(RefArrayLengthFitIntoRegion(1), ClassRoot::ARRAY_STRING, false));
    ASSERT(ObjectToRegion(young_array.GetPtr())->IsEden());
    young_array->Set(0, holder->Get<ObjectHeader *>(0));
    uintptr_t tenured_addr_before_gc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Trigger FullGC by allocating an object in full young. It should fail because there is no tenured space to move 4
    // young regions.
    ASSERT_EQ(nullptr, ObjectAllocator::AllocObjectInYoung());
    uintptr_t tenured_addr_after_gc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Check GC moved tenured regions
    ASSERT_NE(tenured_addr_before_gc, tenured_addr_after_gc);
    // Check FullGC updates refs in young correctly in case it cannot collect young.
    ASSERT_EQ(holder->Get<ObjectHeader *>(0), young_array->Get<ObjectHeader *>(0));
    // Check remsets.
    Region *young_region = ObjectToRegion(young_array.GetPtr());
    ASSERT_TRUE(young_region->IsEden());
    Region *tenured_region = ObjectToRegion(holder->Get<ObjectHeader *>(0));
    ASSERT_TRUE(tenured_region->HasFlag(IS_OLD));
    bool has_object = false;
    tenured_region->GetRemSet()->IterateOverObjects(
        [&has_object, &young_array](ObjectHeader *obj) { has_object |= obj == young_array.GetPtr(); });
    ASSERT_FALSE(has_object);
    // Check young cards
    ASSERT_EQ(NumYoungRegions(), GetAllocator()->GetYoungRegions().size());
    for (Region *region : GetAllocator()->GetYoungRegions()) {
        uintptr_t begin = ToUintPtr(region);
        uintptr_t end = region->End();
        while (begin < end) {
            ASSERT_TRUE(card_table->GetCardPtr(begin)->IsYoung());
            begin += CardTable::GetCardSize();
        }
    }
}

TEST_F(G1FullGCTest, TestFullGCGenericFlow)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *card_table = gc->GetCardTable();
    ASSERT_NE(nullptr, card_table);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    size_t num_free_regions = NumRegions() - NumYoungRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME + 1U;
    FillHeap(num_free_regions, holder, 0);  // occupy 8 tenured regions and 3 young regions
    VMHandle<coretypes::Array> young_array(
        thread, ObjectAllocator::AllocArray(RefArrayLengthFitIntoRegion(1), ClassRoot::ARRAY_STRING, false));
    ASSERT(ObjectToRegion(young_array.GetPtr())->IsEden());
    young_array->Set(0, holder->Get<ObjectHeader *>(0));
    // Check we are OOM
    ASSERT_EQ(nullptr, ObjectAllocator::AllocObjectInYoung());
    uintptr_t tenured_addr_before_gc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Forget two tenured regions
    holder->Set(1, static_cast<ObjectHeader *>(nullptr));
    holder->Set(2, static_cast<ObjectHeader *>(nullptr));
    // Now there should be enough space in tenured to move young
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    ASSERT_NE(nullptr, ObjectAllocator::AllocObjectInYoung());
    uintptr_t tenured_addr_after_gc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Check GC moved tenured regions
    ASSERT_NE(tenured_addr_before_gc, tenured_addr_after_gc);
    // Check FullGC updates refs in young correctly in case it cannot collect young.
    ASSERT_EQ(holder->Get<ObjectHeader *>(0), young_array->Get<ObjectHeader *>(0));
}

TEST_F(G1FullGCTest, TestFullGCResetTenuredRegions)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *card_table = gc->GetCardTable();
    ASSERT_NE(nullptr, card_table);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    // Fill almost all regions (3 tenured regions will be free)
    size_t num_free_regions = NumRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME;
    size_t num_filled_regions = RoundDown(num_free_regions, NumYoungRegions());
    FillHeap(num_filled_regions, holder, 0);
    // Foreget all objects
    for (size_t i = 0; i < num_filled_regions; ++i) {
        holder->Set(i, static_cast<ObjectHeader *>(nullptr));
    }
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Fill almost all regions (3 tenured regions will be free)
    // We should be able to allocate all objects because FullGC should reset old tenured regions.
    FillHeap(num_filled_regions, holder, 0);
}

template <uint32_t REGION_FRAGMENTATION_RATE>
class G1FullGCWithRegionFragmentationRate : public G1FullGCTest {
public:
    G1FullGCWithRegionFragmentationRate() : G1FullGCTest(REGION_FRAGMENTATION_RATE) {}
};

class FullGcRegionFragmentationRateOptionNever : public G1FullGCWithRegionFragmentationRate<100> {};

TEST_F(FullGcRegionFragmentationRateOptionNever, TestG1FullGcRegionFragmentationRateOptionNever)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *card_table = gc->GetCardTable();
    ASSERT_NE(nullptr, card_table);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    // Fill one region
    FillHeap(1, holder, 0);
    // Save ref to young object
    auto object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we moved this young object
    ASSERT_NE(holder->Get<ObjectHeader *>(0), object);
    // Save ref to tenured object
    object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we don't move this object
    ASSERT_EQ(holder->Get<ObjectHeader *>(0), object);
}

class FullGcRegionFragmentationRateOptionAlways : public G1FullGCWithRegionFragmentationRate<0> {};

TEST_F(FullGcRegionFragmentationRateOptionAlways, TestG1FullGcRegionFragmentationRateOptionAlways)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *card_table = gc->GetCardTable();
    ASSERT_NE(nullptr, card_table);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    // Fill one region
    FillHeap(1, holder, 0);
    // Save ref to young object
    auto object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we moved this young object
    ASSERT_NE(holder->Get<ObjectHeader *>(0), object);
    // Save ref to tenured object
    object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we moved this object
    ASSERT_NE(holder->Get<ObjectHeader *>(0), object);
}

class G1FullGCOOMTest : public G1GCTest {
public:
    G1FullGCOOMTest() : G1GCTest(CreateOOMOptions())
    {
        thread_ = MTManagedThread::GetCurrent();
        ASSERT(thread_ != nullptr);
        thread_->ManagedCodeBegin();
    }

    NO_COPY_SEMANTIC(G1FullGCOOMTest);
    NO_MOVE_SEMANTIC(G1FullGCOOMTest);

    static RuntimeOptions CreateOOMOptions()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetCompilerEnableJit(false);
        options.SetShouldInitializeIntrinsics(false);
        // GC options
        constexpr size_t HEAP_SIZE_LIMIT_TEST = 16_MB;
        options.SetRunGcInPlace(true);
        options.SetGcType("g1-gc");
        options.SetHeapSizeLimit(HEAP_SIZE_LIMIT_TEST);
        options.SetGcTriggerType("debug-never");
        options.SetG1NumberOfTenuredRegionsAtMixedCollection(0);
        return options;
    }

    ~G1FullGCOOMTest() override
    {
        thread_->ManagedCodeEnd();
    }

protected:
    MTManagedThread *thread_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(G1FullGCOOMTest, AllocateBy1Region)
{
    constexpr size_t OBJECT_SIZE = AlignUp(static_cast<size_t>(DEFAULT_REGION_SIZE * 0.8), DEFAULT_ALIGNMENT_IN_BYTES);
    {
        [[maybe_unused]] HandleScope<panda::ObjectHeader *> scope(thread_);
        auto *g1_allocator =
            static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
        // Fill tenured space by garbage
        do {
            VMHandle<ObjectHeader> handle(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
            ASSERT_NE(handle.GetPtr(), nullptr) << "Must be correctly allocated object in non-full heap";
            // Move new object to tenured
            Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        } while (g1_allocator->HaveTenuredSize(2));
        ASSERT_TRUE(g1_allocator->HaveTenuredSize(1));
        // Allocate one young region
        VMHandle<ObjectHeader> handle_1(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
        ASSERT_NE(handle_1.GetPtr(), nullptr) << "Must be correctly allocated object. Heap has a lot of garbage";
        // Try to move alone young region to last tenured region
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        // Fully fill young space
        while (g1_allocator->GetHeapSpace()->GetCurrentFreeYoungSize() > 0) {
            auto *young_obj = ObjectAllocator::AllocString(OBJECT_SIZE);
            ASSERT_NE(young_obj, nullptr) << "Must allocate in free young space";
        }
    }
    ASSERT_NE(ObjectAllocator::AllocString(OBJECT_SIZE), nullptr)
        << "We must correctly allocate object in non-full heap";
}

TEST_F(G1FullGCOOMTest, PinUnpinObject)
{
    constexpr size_t OBJECT_SIZE = AlignUp(static_cast<size_t>(DEFAULT_REGION_SIZE * 0.8), DEFAULT_ALIGNMENT_IN_BYTES);
    auto *g1_allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    {
        [[maybe_unused]] HandleScope<panda::ObjectHeader *> scope(thread_);
        // Fill tenured space by garbage
        do {
            VMHandle<ObjectHeader> handle(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
            ASSERT_NE(handle.GetPtr(), nullptr) << "Must be correctly allocated object in non-full heap";
            // Move new object to tenured
            Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        } while (g1_allocator->HaveTenuredSize(2));
        ASSERT_TRUE(g1_allocator->HaveTenuredSize(1));
        // Allocate one young region
        VMHandle<ObjectHeader> handle_1(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
        ASSERT_NE(handle_1.GetPtr(), nullptr) << "Must be correctly allocated object in young";
        ASSERT_TRUE(ObjectToRegion(handle_1.GetPtr())->IsYoung());
        // Pin object in young region
        g1_allocator->PinObject(handle_1.GetPtr());
        // Try to move young region to last tenured region
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        ASSERT_TRUE(ObjectToRegion(handle_1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
        // Just allocate one object in young
        auto *young_obj = ObjectAllocator::AllocString(OBJECT_SIZE);
        ASSERT_NE(young_obj, nullptr) << "Must allocate in free young space";
        // Run Full GC
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));
        // Check "pinned" region
        ASSERT_TRUE(ObjectToRegion(handle_1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
        ASSERT_TRUE(ObjectToRegion(handle_1.GetPtr())->HasPinnedObjects());
        // Unpin object
        g1_allocator->UnpinObject(handle_1.GetPtr());
        // Check "unpinned" region
        ASSERT_TRUE(ObjectToRegion(handle_1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(handle_1.GetPtr())->HasPinnedObjects());
    }
    // Run yet FullGC after unpinning
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));
    g1_allocator->IterateOverObjects([](ObjectHeader *obj) {
        ASSERT_FALSE(ObjectToRegion(obj)->HasPinnedObjects()) << "Along pinned object was unpinned before GC";
    });
}

static void PinUnpinTest(SpaceType requested_space_type, size_t object_size = 1_KB)
{
    ASSERT_TRUE(IsHeapSpace(requested_space_type));
    Runtime::Create(G1FullGCOOMTest::CreateOOMOptions());
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT_NE(thread, nullptr);
    thread->ManagedCodeBegin();
    auto *g1_allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    {
        [[maybe_unused]] HandleScope<panda::ObjectHeader *> scope(thread);
        constexpr size_t OBJ_ELEMENT_SIZE = 64;
        auto *address_before_gc =
            ObjectAllocator::AllocArray(object_size / OBJ_ELEMENT_SIZE, ClassRoot::ARRAY_I64,
                                        requested_space_type == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
        ASSERT_NE(address_before_gc, nullptr);
        VMHandle<ObjectHeader> handle(thread, address_before_gc);
        SpaceType obj_space_type =
            PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(static_cast<void *>(handle.GetPtr()));
        ASSERT_EQ(obj_space_type, requested_space_type);
        g1_allocator->PinObject(handle.GetPtr());
        // Run GC - try to move objects
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
        ASSERT_EQ(address_before_gc, handle.GetPtr()) << "Pinned object must not moved";
        g1_allocator->UnpinObject(handle.GetPtr());
    }
    thread->ManagedCodeEnd();
    Runtime::Destroy();
}

TEST(G1GCPinnigTest, PinUnpinRegularObjectTest)
{
    PinUnpinTest(SpaceType::SPACE_TYPE_OBJECT);
}

TEST(G1GCPinnigTest, PinUnpinHumongousObjectTest)
{
    constexpr size_t HUMONGOUS_OBJECT_FOR_PINNING_SIZE = 4_MB;
    PinUnpinTest(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, HUMONGOUS_OBJECT_FOR_PINNING_SIZE);
}

TEST(G1GCPinnigTest, PinUnpinNonMovableObjectTest)
{
    PinUnpinTest(SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace panda::mem
