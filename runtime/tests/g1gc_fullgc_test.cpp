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
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <thread>

#include "gtest/gtest.h"
#include "iostream"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/gc/generational-gc-base.h"
#include "runtime/mem/malloc-proxy-allocator-inl.h"
#include "runtime/mem/mem_stats.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/mem/runslots_allocator-inl.h"

namespace panda::mem::test {

class G1GCFullGCTest : public testing::Test {
public:
    using ObjVec = PandaVector<ObjectHeader *>;
    using HanVec = PandaVector<VMHandle<ObjectHeader *> *>;
    static constexpr size_t ROOT_MAX_SIZE = 100000U;

    static constexpr GCTaskCause MIXED_G1_GC_CAUSE = GCTaskCause::YOUNG_GC_CAUSE;
    static constexpr GCTaskCause FULL_GC_CAUSE = GCTaskCause::EXPLICIT_CAUSE;

    enum class TargetSpace { YOUNG, TENURED, LARGE, HUMONG };

    class GCCounter : public GCListener {
    public:
        void GCStarted([[maybe_unused]] size_t heap_size) override
        {
            count++;
        }

        void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                        [[maybe_unused]] size_t heap_size) override
        {
        }

        int count = 0;
    };

    template <typename F>
    class GCHook : public GCListener {
    public:
        explicit GCHook(F hook) : hook_(hook) {};

        void GCStarted([[maybe_unused]] size_t heap_size) override {}

        void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                        [[maybe_unused]] size_t heap_size) override
        {
            hook_();
        }

        F hook_;
    };

    void SetupRuntime(const std::string &gc_type)
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetUseTlabForAllocations(false);
        options.SetGcType(gc_type);
        options.SetGcTriggerType("debug");
        options.SetGcDebugTriggerStart(std::numeric_limits<int>::max());
        options.SetCompilerEnableJit(false);
        options.SetHeapSizeLimit(33554432);
        options.SetYoungSpaceSize(4194304);
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);

        thread_ = panda::MTManagedThread::GetCurrent();
        gc_type_ = Runtime::GetGCType(options, plugins::RuntimeTypeToLang(options.GetRuntimeType()));
        [[maybe_unused]] auto gc = thread_->GetVM()->GetGC();
        ASSERT(gc->GetType() == panda::mem::GCTypeFromString(gc_type));
        ASSERT(gc->IsGenerational());
        thread_->ManagedCodeBegin();
    }

    void ResetRuntime()
    {
        DeleteHandles();
        internal_allocator_->Delete(gccnt_);
        thread_->ManagedCodeEnd();
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }

    template <typename F, size_t multi>
    ObjVec MakeAllocations(size_t min_size, size_t max_size, size_t count, size_t *allocated, size_t *requested,
                           F space_checker, bool check_oom_in_tenured = false);

    void InitRoot();
    void MakeObjectsAlive(ObjVec objects, int every = 1);
    void MakeObjectsPermAlive(ObjVec objects, int every = 1);
    void MakeObjectsGarbage(size_t start_idx, size_t after_end_idx, int every = 1);
    void DumpHandles();
    void DumpAliveObjects();
    void DeleteHandles();
    bool IsInYoung(uintptr_t addr);

    template <class LanguageConfig>
    void PrepareTest();

    void TearDown() override {}

    panda::MTManagedThread *thread_;
    GCType gc_type_;

    LanguageContext ctx_ {nullptr};
    ObjectAllocatorBase *object_allocator_;
    mem::InternalAllocatorPtr internal_allocator_;
    PandaVM *vm_;
    GC *gc_;
    std::vector<HanVec> handles_;
    MemStatsType *ms_;
    GCStats *gc_ms_;
    coretypes::Array *root_ = nullptr;
    size_t root_size_ = 0;
    GCCounter *gccnt_;
};

template <typename F, size_t multi>
G1GCFullGCTest::ObjVec G1GCFullGCTest::MakeAllocations(size_t min_size, size_t max_size, size_t count,
                                                       size_t *allocated, size_t *requested,
                                                       [[maybe_unused]] F space_checker, bool check_oom_in_tenured)
{
    ASSERT(min_size <= max_size);
    *allocated = 0;
    *requested = 0;
    // Create array of object templates based on count and max size
    PandaVector<PandaString> obj_templates(count);
    size_t obj_size = sizeof(coretypes::String) + min_size;
    for (size_t i = 0; i < count; ++i) {
        PandaString simple_string;
        simple_string.resize(obj_size - sizeof(coretypes::String));
        obj_templates[i] = std::move(simple_string);
        obj_size += (max_size / count + i);  // +i to mess with the alignment
        if (obj_size > max_size) {
            obj_size = max_size;
        }
    }
    ObjVec result;
    result.reserve(count * multi);
    for (size_t j = 0; j < count; ++j) {
        size_t size = obj_templates[j].length() + sizeof(coretypes::String);
        if (check_oom_in_tenured) {
            // Leaving 5MB in tenured seems OK
            auto free =
                reinterpret_cast<GenerationalSpaces *>(object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
            if (size + 5000000 > free) {
                return result;
            }
        }
        for (size_t i = 0; i < multi; ++i) {
            coretypes::String *string_obj = coretypes::String::CreateFromMUtf8(
                reinterpret_cast<const uint8_t *>(&obj_templates[j][0]), obj_templates[j].length(), ctx_, vm_);
            ASSERT(string_obj != nullptr);
            ASSERT(space_checker(ToUintPtr(string_obj)));
            *allocated += GetAlignedObjectSize(size);
            *requested += size;
            result.push_back(string_obj);
        }
    }
    return result;
}

void G1GCFullGCTest::InitRoot()
{
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    Class *klass = class_linker->GetExtension(
        panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(ctx_.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    root_ = coretypes::Array::Create(klass, ROOT_MAX_SIZE);
    root_size_ = 0;
    MakeObjectsPermAlive({root_});
}

void G1GCFullGCTest::MakeObjectsAlive(ObjVec objects, int every)
{
    int cnt = every;
    for (auto *obj : objects) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        root_->Set(root_size_, obj);
        root_size_++;
        ASSERT(root_size_ < ROOT_MAX_SIZE);
        cnt = every;
    }
}

void G1GCFullGCTest::MakeObjectsGarbage(size_t start_idx, size_t after_end_idx, int every)
{
    int cnt = every;
    for (size_t i = start_idx; i < after_end_idx; ++i) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        root_->Set(i, 0);
        root_size_++;
        cnt = every;
    }
}

void G1GCFullGCTest::MakeObjectsPermAlive(ObjVec objects, int every)
{
    HanVec result;
    result.reserve(objects.size() / every);
    int cnt = every;
    for (auto *obj : objects) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        result.push_back(internal_allocator_->New<VMHandle<ObjectHeader *>>(thread_, obj));
        cnt = every;
    }
    handles_.push_back(result);
}

void G1GCFullGCTest::DumpHandles()
{
    for (auto &hv : handles_) {
        for (auto *handle : hv) {
            std::cout << "vector " << (void *)&hv << " handle " << (void *)handle << " obj " << handle->GetPtr()
                      << std::endl;
        }
    }
}

void G1GCFullGCTest::DumpAliveObjects()
{
    std::cout << "Alive root array : " << handles_[0][0]->GetPtr() << std::endl;
    for (size_t i = 0; i < root_size_; ++i) {
        if (root_->Get<ObjectHeader *>(i) != nullptr) {
            std::cout << "Alive idx " << i << " : " << root_->Get<ObjectHeader *>(i) << std::endl;
        }
    }
}

void G1GCFullGCTest::DeleteHandles()
{
    for (auto &hv : handles_) {
        for (auto *handle : hv) {
            internal_allocator_->Delete(handle);
        }
    }
    handles_.clear();
}

template <class LanguageConfig>
void G1GCFullGCTest::PrepareTest()
{
    if constexpr (std::is_same<LanguageConfig, panda::PandaAssemblyLanguageConfig>::value) {
        DeleteHandles();
        ctx_ = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        object_allocator_ = thread_->GetVM()->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
        vm_ = Runtime::GetCurrent()->GetPandaVM();
        internal_allocator_ = Runtime::GetCurrent()->GetClassLinker()->GetAllocator();
        gc_ = vm_->GetGC();
        ms_ = vm_->GetMemStats();
        gc_ms_ = vm_->GetGCStats();
        gccnt_ = internal_allocator_->New<GCCounter>();
        gc_->AddListener(gccnt_);
        InitRoot();
    } else {
        UNREACHABLE();  // NYI
    }
}

bool G1GCFullGCTest::IsInYoung(uintptr_t addr)
{
    switch (gc_type_) {
        case GCType::GEN_GC: {
            return object_allocator_->IsAddressInYoungSpace(addr);
        }
        case GCType::G1_GC: {
            auto mem_pool = PoolManager::GetMmapMemPool();
            if (mem_pool->GetSpaceTypeForAddr(reinterpret_cast<ObjectHeader *>(addr)) != SpaceType::SPACE_TYPE_OBJECT) {
                return false;
            }
            return Region::AddrToRegion<false>(ToVoidPtr(addr))->HasFlag(RegionFlag::IS_EDEN);
        }
        default:
            UNREACHABLE();  // NYI
    }
    return false;
}

TEST_F(G1GCFullGCTest, TestIntensiveAlloc)
{
    std::string gctype = static_cast<std::string>(GCStringFromType(GCType::G1_GC));
    SetupRuntime(gctype);
    {
        HandleScope<ObjectHeader *> scope(thread_);
        PrepareTest<panda::PandaAssemblyLanguageConfig>();
        [[maybe_unused]] size_t bytes, raw_objects_size;

        [[maybe_unused]] size_t young_size =
            reinterpret_cast<GenerationalSpaces *>(reinterpret_cast<ObjectAllocatorGenBase *>(
                object_allocator_)->GetHeapSpace())->GetCurrentMaxYoungSize();
        [[maybe_unused]] size_t heap_size = mem::MemConfig::GetHeapSizeLimit();
        [[maybe_unused]] auto g1_alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(object_allocator_);
        [[maybe_unused]] size_t max_y_size = g1_alloc->GetYoungAllocMaxSize();

        [[maybe_unused]] auto y_space_check = [](uintptr_t addr) -> bool { return IsInYoung(addr); };
        [[maybe_unused]] auto h_space_check = [](uintptr_t addr) -> bool { return !IsInYoung(addr); };
        [[maybe_unused]] auto t_free =
            reinterpret_cast<GenerationalSpaces *>(object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
        const size_t y_obj_size = max_y_size / 10;
        gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        size_t initial_heap = ms_->GetFootprintHeap();

        {
            // Ordinary young shall not be broken when intermixed with explicits
            int i = 0;
            size_t allocated = 0;
            while (allocated < 2 * heap_size) {
                ObjVec ov1 = MakeAllocations<decltype(y_space_check), 1>(y_obj_size, y_obj_size, 1, &bytes,
                                                                         &raw_objects_size, y_space_check);
                allocated += bytes;
                if (i++ % 100 == 0) {
                    gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                }
            }
            gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
            ASSERT_EQ(initial_heap, ms_->GetFootprintHeap());
        }

        {
            // Intensive allocations surviving 1 young
            auto old_root_size = root_size_;
            size_t allocated = 0;
            bool gc_happened = false;
            GCHook gchook([&old_root_size, this, &gc_happened]() {
                MakeObjectsGarbage(old_root_size, this->root_size_);
                old_root_size = this->root_size_;
                gc_happened = 1;
            });
            gc_->AddListener(&gchook);
            while (allocated < 4 * heap_size) {
                ObjVec ov1 = MakeAllocations<decltype(y_space_check), 1>(y_obj_size, y_obj_size, 1, &bytes,
                                                                         &raw_objects_size, y_space_check);
                MakeObjectsAlive(ov1, 1);
                t_free = reinterpret_cast<GenerationalSpaces *>(
                    object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
                allocated += bytes;
            }
            MakeObjectsGarbage(old_root_size, root_size_);
            gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
            ASSERT_EQ(initial_heap, ms_->GetFootprintHeap());
            gc_->RemoveListener(&gchook);
        }

        MakeObjectsGarbage(0, root_size_);
        gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        ASSERT_EQ(initial_heap, ms_->GetFootprintHeap());
    }
    ResetRuntime();
}

TEST_F(G1GCFullGCTest, TestExplicitFullNearLimit)
{
    std::string gctype = static_cast<std::string>(GCStringFromType(GCType::G1_GC));
    SetupRuntime(gctype);
    {
        HandleScope<ObjectHeader *> scope(thread_);
        PrepareTest<panda::PandaAssemblyLanguageConfig>();
        [[maybe_unused]] size_t bytes, raw_objects_size;

        [[maybe_unused]] size_t young_size =
            reinterpret_cast<GenerationalSpaces *>(reinterpret_cast<ObjectAllocatorGenBase *>(
                object_allocator_)->GetHeapSpace())->GetCurrentMaxYoungSize();
        [[maybe_unused]] size_t heap_size = mem::MemConfig::GetHeapSizeLimit();
        [[maybe_unused]] auto g1_alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(object_allocator_);
        [[maybe_unused]] size_t max_y_size = g1_alloc->GetYoungAllocMaxSize();

        [[maybe_unused]] auto y_space_check = [](uintptr_t addr) -> bool { return IsInYoung(addr); };
        [[maybe_unused]] auto h_space_check = [](uintptr_t addr) -> bool { return !IsInYoung(addr); };
        [[maybe_unused]] auto t_free =
            reinterpret_cast<GenerationalSpaces *>(object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
        const size_t y_obj_size = max_y_size / 10;
        gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        size_t initial_heap = ms_->GetFootprintHeap();

        {
            // Allocating until we are close to OOM, then do release this mem,
            // do explicit full and allocate the same size once again
            auto old_root_size = root_size_;
            int i = 0;
            while (t_free > 2.2 * young_size) {
                ObjVec ov1 = MakeAllocations<decltype(y_space_check), 1>(y_obj_size, y_obj_size, 1, &bytes,
                                                                         &raw_objects_size, y_space_check);
                MakeObjectsAlive(ov1, 1);
                t_free = reinterpret_cast<GenerationalSpaces *>(
                    object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
                i++;
            }
            gc_->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
            MakeObjectsGarbage(old_root_size, root_size_);

            old_root_size = root_size_;
            while (--i > 0) {
                ObjVec ov1 = MakeAllocations<decltype(y_space_check), 1>(y_obj_size, y_obj_size, 1, &bytes,
                                                                         &raw_objects_size, y_space_check);
                MakeObjectsAlive(ov1, 1);
                t_free = reinterpret_cast<GenerationalSpaces *>(
                    object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
            }
            MakeObjectsGarbage(old_root_size, root_size_);
            gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        }

        MakeObjectsGarbage(0, root_size_);
        gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        ASSERT_EQ(initial_heap, ms_->GetFootprintHeap());
    }
    ResetRuntime();
}

TEST_F(G1GCFullGCTest, TestOOMFullNearLimit)
{
    std::string gctype = static_cast<std::string>(GCStringFromType(GCType::G1_GC));
    SetupRuntime(gctype);
    {
        HandleScope<ObjectHeader *> scope(thread_);
        PrepareTest<panda::PandaAssemblyLanguageConfig>();
        [[maybe_unused]] size_t bytes, raw_objects_size;

        [[maybe_unused]] size_t young_size =
            reinterpret_cast<GenerationalSpaces *>(reinterpret_cast<ObjectAllocatorGenBase *>(
                object_allocator_)->GetHeapSpace())->GetCurrentMaxYoungSize();
        [[maybe_unused]] size_t heap_size = mem::MemConfig::GetHeapSizeLimit();
        [[maybe_unused]] auto g1_alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(object_allocator_);
        [[maybe_unused]] size_t max_y_size = g1_alloc->GetYoungAllocMaxSize();

        [[maybe_unused]] auto y_space_check = [](uintptr_t addr) -> bool { return IsInYoung(addr); };
        [[maybe_unused]] auto h_space_check = [](uintptr_t addr) -> bool { return !IsInYoung(addr); };
        [[maybe_unused]] auto t_free =
            reinterpret_cast<GenerationalSpaces *>(object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
        const size_t y_obj_size = max_y_size / 10;
        gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        size_t initial_heap = ms_->GetFootprintHeap();

        {
            // Allocating until we are close to OOM, then do release this mem,
            // then allocate the same size once again checking if we can handle it w/o OOM
            auto old_root_size = root_size_;
            int i = 0;
            while (t_free > 2.2 * young_size) {
                ObjVec ov1 = MakeAllocations<decltype(y_space_check), 1>(y_obj_size, y_obj_size, 1, &bytes,
                                                                         &raw_objects_size, y_space_check);
                MakeObjectsAlive(ov1, 1);
                t_free = reinterpret_cast<GenerationalSpaces *>(
                    object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
                i++;
            }
            MakeObjectsGarbage(old_root_size, root_size_);

            old_root_size = root_size_;
            while (--i > 0) {
                ObjVec ov1 = MakeAllocations<decltype(y_space_check), 1>(y_obj_size, y_obj_size, 1, &bytes,
                                                                         &raw_objects_size, y_space_check);
                MakeObjectsAlive(ov1, 1);
                t_free = reinterpret_cast<GenerationalSpaces *>(
                    object_allocator_->GetHeapSpace())->GetCurrentFreeTenuredSize();
            }
            MakeObjectsGarbage(old_root_size, root_size_);
            gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        }

        MakeObjectsGarbage(0, root_size_);
        gc_->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        ASSERT_EQ(initial_heap, ms_->GetFootprintHeap());
    }
    ResetRuntime();
}
}  // namespace panda::mem::test
