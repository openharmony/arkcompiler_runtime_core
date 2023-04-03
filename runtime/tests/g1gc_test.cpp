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

#include <gtest/gtest.h>

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

#include "test_utils.h"

namespace panda::mem {

class G1GCTest : public testing::Test {
public:
    explicit G1GCTest(size_t promotion_region_alive_rate = 100)
    {
        RuntimeOptions options;
        options.SetBootClassSpaces({"core"});
        options.SetRuntimeType("core");
        options.SetGcType("g1-gc");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetG1PromotionRegionAliveRate(promotion_region_alive_rate);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);

        Runtime::Create(options);
    }

    ~G1GCTest()
    {
        Runtime::Destroy();
    }

    static constexpr size_t GetHumongousStringLength()
    {
        // Total string size will be DEFAULT_REGION_SIZE + sizeof(String).
        // It is enought to make it humongous.
        return DEFAULT_REGION_SIZE;
    }

    size_t GetHumongousArrayLength(ClassRoot class_root)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *array_class = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(class_root);
        EXPECT_TRUE(array_class->IsArrayClass());
        if (!array_class->IsArrayClass()) {
            return 0;
        }
        // Total array size will be DEFAULT_REGION_SIZE * elem_size + sizeof(Array).
        // It is enought to make it humongous.
        size_t elem_size = array_class->GetComponentSize();
        return DEFAULT_REGION_SIZE / elem_size + 1;
    }

    coretypes::Array *AllocArray(size_t length, ClassRoot class_root, bool nonmovable)
    {
        ObjectAllocator object_allocator;
        return object_allocator.AllocArray(length, class_root, nonmovable);
    }

    coretypes::String *AllocString(size_t length)
    {
        ObjectAllocator object_allocator;
        return object_allocator.AllocString(length);
    }

    ObjectHeader *AllocObjectInYoung()
    {
        ObjectAllocator object_allocator;
        return object_allocator.AllocObjectInYoung();
    }

    ObjectAllocatorG1<> *GetAllocator()
    {
        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        return static_cast<ObjectAllocatorG1<> *>(gc->GetObjectAllocator());
    }
};

class RemSetChecker : public GCListener {
public:
    explicit RemSetChecker(ObjectHeader *obj, ObjectHeader *ref)
        : obj_(MTManagedThread::GetCurrent(), obj), ref_(MTManagedThread::GetCurrent(), ref)
    {
    }

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase == GCPhase::GC_PHASE_MARK_YOUNG) {
            Check();
        }
    }

    void GCPhaseFinished(GCPhase phase) override
    {
        if (phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE) {
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
        remset->VisitMarkedCards([object, &has_object](ObjectHeader *obj) { has_object |= obj == object; });
        ASSERT_TRUE(has_object);
    }

private:
    VMHandle<ObjectHeader> obj_;
    VMHandle<ObjectHeader> ref_;
};

TEST_F(G1GCTest, TestAddrToRegion)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    size_t humongous_len = GetHumongousArrayLength(ClassRoot::ARRAY_U8);
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<ObjectHeader> young(thread, AllocArray(0, ClassRoot::ARRAY_U8, false));
    ASSERT_NE(nullptr, young.GetPtr());
    VMHandle<ObjectHeader> nonmovable(thread, AllocArray(0, ClassRoot::ARRAY_U8, true));
    ASSERT_NE(nullptr, nonmovable.GetPtr());
    VMHandle<ObjectHeader> humongous(thread, AllocArray(humongous_len, ClassRoot::ARRAY_U8, false));
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
    ObjectHeader *obj = AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_U8), ClassRoot::ARRAY_U8, false);
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
    static constexpr size_t array_length = 100;
    coretypes::Array *non_movable_obj = nullptr;
    uintptr_t prev_young_addr = 0;
    Class *klass = class_linker->GetExtension(
        panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    non_movable_obj = coretypes::Array::Create(klass, array_length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    coretypes::String *young_obj = coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    non_movable_obj->Set(0, young_obj);
    prev_young_addr = ToUintPtr(young_obj);
    VMHandle<coretypes::Array> non_movable_obj_ptr(thread, non_movable_obj);

    // Trigger GC
    RemSetChecker listener(non_movable_obj, non_movable_obj->Get<ObjectHeader *>(0));
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
    VMHandle<coretypes::Array> humongous_obj(thread, AllocArray(array_length, ClassRoot::ARRAY_STRING, false));
    ObjectHeader *young_obj = AllocObjectInYoung();
    humongous_obj->Set(0, young_obj);
    prev_young_addr = ToUintPtr(young_obj);

    // Trigger GC
    RemSetChecker listener(humongous_obj.GetPtr(), humongous_obj->Get<ObjectHeader *>(0));
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

    humongous = VMHandle<coretypes::Array>(
        thread, AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_STRING), ClassRoot::ARRAY_STRING, false));
    nonmovable = VMHandle<coretypes::Array>(thread, AllocArray(1, ClassRoot::ARRAY_STRING, true));
    obj = AllocObjectInYoung();
    humongous->Set(0, obj);
    nonmovable->Set(0, obj);
    obj_addr = ToUintPtr(obj);

    RemSetChecker listener1(humongous.GetPtr(), obj);
    RemSetChecker listener2(nonmovable.GetPtr(), obj);
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
    static constexpr size_t humongous_array_length = 262144LU;
    static constexpr size_t young_array_length = ((DEFAULT_REGION_SIZE - Region::HeadSize()) / 4U) - 16U;
    coretypes::Array *humongous_obj;
    coretypes::Array *young_arr;

    auto *gc = runtime->GetPandaVM()->GetGC();
    auto card_table = gc->GetCardTable();

    Class *klass;

    klass = class_linker->GetExtension(
        panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);

    young_arr = coretypes::Array::Create(klass, young_array_length);
    ASSERT_NE(young_arr, nullptr);
    auto *region = ObjectToRegion(young_arr);
    ASSERT_NE(region, nullptr);

    VMHandle<coretypes::Array> young_obj_ptr(thread, young_arr);
    GCTask task(GCTaskCause::EXPLICIT_CAUSE);
    {
        [[maybe_unused]] HandleScope<ObjectHeader *> scope_for_humongous_obj(thread);

        humongous_obj = coretypes::Array::Create(klass, humongous_array_length);

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
            PandaVector<CardTable::CardPtr> cards;
            array_region->GetRemSet()->ProceedMarkedCards(
                [&cards](CardTable::CardPtr card, [[maybe_unused]] Region *region_unused) { cards.push_back(card); });
            ASSERT_EQ(1U, cards.size());  // we have reference only from 1 humongous space
            uintptr_t card_addr = card_table->GetCardStartAddress(cards[0]);
            ASSERT_EQ(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                      PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(ToVoidPtr(card_addr)));
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
    PandaVector<CardTable::CardPtr> cards;
    array_region->GetRemSet()->ProceedMarkedCards(
        [&cards](CardTable::CardPtr card, [[maybe_unused]] Region *region_unused) { cards.push_back(card); });
    ASSERT_EQ(0, cards.size());  // we have no references from the humongous space
}

class NewObjectsListener : public GCListener {
public:
    explicit NewObjectsListener(G1GCTest *test) : test_(test) {}

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        MTManagedThread *thread = MTManagedThread::GetCurrent();

        // Allocate quite large object to make allocator to create a separate region
        size_t nonmovable_len = 9 * DEFAULT_REGION_SIZE / 10;
        ObjectHeader *dummy = test_->AllocArray(nonmovable_len, ClassRoot::ARRAY_U8, true);
        Region *dummy_region = ObjectToRegion(dummy);
        EXPECT_TRUE(dummy_region->HasFlag(RegionFlag::IS_NONMOVABLE));
        nonmovable_ = VMHandle<ObjectHeader>(thread, test_->AllocArray(nonmovable_len, ClassRoot::ARRAY_U8, true));
        Region *nonmovable_region = ObjectToRegion(nonmovable_.GetPtr());
        EXPECT_TRUE(nonmovable_region->HasFlag(RegionFlag::IS_NONMOVABLE));
        EXPECT_NE(nonmovable_region, dummy_region);
        nonmovable_mark_bitmap_addr_ = ToUintPtr(nonmovable_region->GetMarkBitmap());

        size_t humongous_len = test_->GetHumongousArrayLength(ClassRoot::ARRAY_U8);
        humongous_ = VMHandle<ObjectHeader>(thread, test_->AllocArray(humongous_len, ClassRoot::ARRAY_U8, false));
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
    G1GCTest *test_;
    VMHandle<ObjectHeader> nonmovable_;
    uintptr_t nonmovable_mark_bitmap_addr_ {0};
    VMHandle<ObjectHeader> humongous_;
    uintptr_t humongous_mark_bitmap_addr_ {0};
};

// Test the new objects created during concurrent marking are alive
TEST_F(G1GCTest, TestNewObjectsSATB)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    NewObjectsListener listener(this);
    gc->AddListener(&listener);

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);  // threshold cause should trigger concurrent marking
        task.Run(*runtime->GetPandaVM()->GetGC());
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
    CollectionSetChecker(ObjectAllocatorG1<> *allocator) : allocator_(allocator) {}

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

        young1 = VMHandle<ObjectHeader>(thread, AllocArray(young_len, ClassRoot::ARRAY_U8, false));
        young2 = VMHandle<ObjectHeader>(thread, AllocArray(young_len, ClassRoot::ARRAY_U8, false));
        young3 = VMHandle<ObjectHeader>(thread, AllocArray(young_len, ClassRoot::ARRAY_U8, false));

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
    tenured = VMHandle<ObjectHeader>(thread, AllocObjectInYoung());

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young to tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    young = VMHandle<ObjectHeader>(thread, AllocObjectInYoung());
    humongous = VMHandle<ObjectHeader>(
        thread, AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_U8), ClassRoot::ARRAY_U8, false));

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
    size_t big_len = garbage_rate * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    size_t big_len1 = (garbage_rate + 1) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    size_t big_len2 = (garbage_rate + 2) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    size_t small_len = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::String);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocatorG1<> *allocator = GetAllocator();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> holder;
    VMHandle<ObjectHeader> young;

    // Allocate objects of different sizes.
    // Mixed regions should be choosen according to the largest garbage.
    holder = VMHandle<coretypes::Array>(thread, AllocArray(4, ClassRoot::ARRAY_STRING, false));
    holder->Set(0, AllocString(big_len));
    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(0))->HasFlag(RegionFlag::IS_EDEN));
    holder->Set(1, AllocString(big_len1));
    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(1))->HasFlag(RegionFlag::IS_EDEN));
    holder->Set(2, AllocString(big_len2));
    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(2))->HasFlag(RegionFlag::IS_EDEN));
    holder->Set(3, AllocString(small_len));
    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(3))->HasFlag(RegionFlag::IS_EDEN));

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
    current = VMHandle<ObjectHeader>(thread, AllocArray(small_len, ClassRoot::ARRAY_U8, false));

    // Propogate 'current' object -> tenured and prepare for mixed GC
    // Release 'big1', 'big2' and 'small' objects to make them garbage
    Region *region0 = ObjectToRegion(holder->Get<ObjectHeader *>(0));
    Region *region1 = ObjectToRegion(holder->Get<ObjectHeader *>(1));
    Region *region2 = ObjectToRegion(holder->Get<ObjectHeader *>(2));
    holder->Set(0, static_cast<ObjectHeader *>(nullptr));
    holder->Set(1, static_cast<ObjectHeader *>(nullptr));
    holder->Set(2, static_cast<ObjectHeader *>(nullptr));
    holder->Set(3, static_cast<ObjectHeader *>(nullptr));
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task1.Run(*gc);
    }

    // Now the region with 'current' is current and it will not be included into the collection set.

    young = VMHandle<ObjectHeader>(thread, AllocObjectInYoung());

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
    young = VMHandle<ObjectHeader>(thread, AllocObjectInYoung());
    yregion = ObjectToRegion(young.GetPtr());
    checker.SetExpectedRegions({region0, yregion});
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task3(GCTaskCause::YOUNG_GC_CAUSE);  // should run mixed GC
        task3.Run(*gc);
    }
}
class G1GCPromotionTest : public G1GCTest {
public:
    G1GCPromotionTest() : G1GCTest(PROMOTE_RATE) {}

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
    std::array<ObjectHeader *, FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT> first_region_object_links;
    std::array<ObjectHeader *, SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT> second_region_object_links;
    // Check Promotion for young region:

    first_holder = VMHandle<coretypes::Array>(thread, AllocArray(HUMONGOUS_STRING_LEN, ClassRoot::ARRAY_STRING, false));
    Region *first_region = ObjectToRegion(AllocObjectInYoung());
    ASSERT_TRUE(first_region->HasFlag(RegionFlag::IS_EDEN));
    for (size_t i = 0; i < FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        first_region_object_links[i] = AllocObjectInYoung();
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
        ASSERT_TRUE(ObjectToRegion(first_holder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_PROMOTED));
    }

    second_holder =
        VMHandle<coretypes::Array>(thread, AllocArray(HUMONGOUS_STRING_LEN, ClassRoot::ARRAY_STRING, false));
    Region *second_region = ObjectToRegion(AllocObjectInYoung());
    ASSERT_TRUE(second_region->HasFlag(RegionFlag::IS_EDEN));
    for (size_t i = 0; i < SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        second_region_object_links[i] = AllocObjectInYoung();
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

class InterruptGCListener : public GCListener {
public:
    InterruptGCListener(G1GCTest *test, VMHandle<coretypes::Array> *array) : test_(test), array_(array) {}

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        // Allocate an object to add it into SATB buffer
        test_->AllocObjectInYoung();
        // Set interrupt flag
        GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
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
    G1GCTest *test_;
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

    array = VMHandle<coretypes::Array>(thread, AllocArray(1, ClassRoot::ARRAY_STRING, false));
    array->Set(0, AllocString(1));

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young objects -> tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);

        // Clear live bytes to check that concurrent marking will not calculate them
        Region *region = ObjectToRegion(array->Get<ObjectHeader *>(0));
        ASSERT_TRUE(region != nullptr);
        region->SetLiveBytes(0);

        InterruptGCListener listener(this, &array);
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
    array = VMHandle<coretypes::Array>(thread, AllocArray(2, ClassRoot::ARRAY_STRING, false));
    ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(RegionFlag::IS_EDEN));
    // The same for string. The instance size must be 8-bytes aligned.
    array->Set(0, AllocString(8));
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

    AllocObjectInYoung();

    NullRefListener listener(&array);
    gc->AddListener(&listener);
    {
        ScopedNativeCodeThread sn(thread);
        // Prepare for mixed GC, start concurrent marking and calculate garbage for regions
        GCTask task2(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task2.Run(*gc);
    }

    Region *region = ObjectToRegion(array.GetPtr());
    ASSERT_EQ(array_size, region->GetLiveBytes());
    ASSERT_EQ(str_size, region->GetGarbageBytes());
}

TEST_F(G1GCTest, NonMovableClearingDuringConcurrentPhaseTest)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto obj_allocator =
        Runtime::GetCurrent()->GetPandaVM()->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    size_t array_length = GetHumongousArrayLength(ClassRoot::ARRAY_STRING) - 50;
    coretypes::Array *first_non_movable_obj = nullptr;
    coretypes::Array *second_non_movable_obj = nullptr;
    uintptr_t prev_young_addr = 0;

    Class *klass = class_linker->GetExtension(
        panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(ctx.GetStringArrayClassDescriptor());
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
    auto obj_allocator =
        Runtime::GetCurrent()->GetPandaVM()->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    size_t array_length = GetHumongousArrayLength(ClassRoot::ARRAY_STRING);
    coretypes::Array *first_humongous_obj = nullptr;
    coretypes::Array *second_humongous_obj = nullptr;
    uintptr_t prev_young_addr = 0;

    Class *klass = class_linker->GetExtension(
        panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(ctx.GetStringArrayClassDescriptor());
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

}  // namespace panda::mem
