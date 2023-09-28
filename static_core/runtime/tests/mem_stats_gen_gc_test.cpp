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
class MemStatsGenGCTest : public testing::Test {
public:
    using ObjVec = PandaVector<ObjectHeader *>;
    using HanVec = PandaVector<VMHandle<ObjectHeader *> *>;
    static constexpr size_t ROOT_MAX_SIZE = 100000U;
    static constexpr int MIX_TEST_ALLOC_TIMES = 5;
    static constexpr int FULL_TEST_ALLOC_TIMES = 2;

    static constexpr GCTaskCause MIXED_G1_GC_CAUSE = GCTaskCause::YOUNG_GC_CAUSE;
    static constexpr GCTaskCause FULL_GC_CAUSE = GCTaskCause::EXPLICIT_CAUSE;

    enum class TargetSpace {
        YOUNG,
        TENURED_REGULAR,
        /*
         * Some allocators have Large objects, it's not the same as Humongous. Objects can be less than Humongous but be
         * allocated directly in the tenured space for example.
         */
        TENURED_LARGE,
        HUMONGOUS
    };

    // this class allows to iterate over configurations for JIT and TLAB
    class Config {
    public:
        enum class JITConfig : bool { NO_JIT = false, JIT };

        enum class TLABConfig : bool { NO_TLAB = false, TLAB };

        bool IsJITEnabled() const
        {
            return static_cast<bool>(jit_cfg_);
        }

        bool IsTLABEnabled() const
        {
            return static_cast<bool>(tlab_cfg_);
        }

        bool End() const
        {
            return jit_cfg_ == JITConfig::JIT && tlab_cfg_ == TLABConfig::NO_TLAB;
        }

        Config &operator++()
        {
            if (jit_cfg_ == JITConfig::NO_JIT) {
                jit_cfg_ = JITConfig::JIT;
            } else {
                jit_cfg_ = JITConfig::NO_JIT;
                if (tlab_cfg_ == TLABConfig::TLAB) {
                    tlab_cfg_ = TLABConfig::NO_TLAB;
                } else {
                    tlab_cfg_ = TLABConfig::TLAB;
                }
            }
            return *this;
        }

    private:
        JITConfig jit_cfg_ {JITConfig::NO_JIT};
        TLABConfig tlab_cfg_ {TLABConfig::TLAB};
    };

    class GCCounter : public GCListener {
    public:
        void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size) override
        {
            count++;
        }

        void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                        [[maybe_unused]] size_t heap_size) override
        {
        }

        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        int count = 0;
    };

    struct MemOpReport {
        size_t allocated_count;
        size_t allocated_bytes;
        size_t saved_count;
        size_t saved_bytes;
    };

    struct RealStatsLocations {
        uint32_t *young_freed_objects_count;
        uint64_t *young_freed_objects_size;
        uint32_t *young_moved_objects_count;
        uint64_t *young_moved_objects_size;
        uint32_t *tenured_freed_objects_count;
        uint64_t *tenured_freed_objects_size;
    };

    void SetupRuntime(const std::string &gc_type_param, const Config &cfg)
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetUseTlabForAllocations(cfg.IsTLABEnabled());
        options.SetGcType(gc_type_param);
        options.SetGcTriggerType("debug-never");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(cfg.IsJITEnabled());
        options.SetExplicitConcurrentGcEnabled(false);
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);

        thread = panda::MTManagedThread::GetCurrent();
        gc_type = Runtime::GetGCType(options, plugins::RuntimeTypeToLang(Runtime::GetRuntimeType()));
        [[maybe_unused]] auto gc_local = thread->GetVM()->GetGC();
        ASSERT(gc_local->GetType() == panda::mem::GCTypeFromString(gc_type_param));
        ASSERT(gc_local->IsGenerational());
        thread->ManagedCodeBegin();
    }

    void ResetRuntime()
    {
        DeleteHandles();
        internal_allocator->Delete(gccnt);
        thread->ManagedCodeEnd();
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }

    template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE>
    ObjVec MakeAllocationsWithRepeats(size_t min_size, size_t max_size, size_t count, size_t *allocated,
                                      size_t *requested, F space_checker, bool check_oom_in_tenured);

    void InitRoot();
    void MakeObjectsAlive(const ObjVec &objects, int every = 1);
    void MakeObjectsPermAlive(const ObjVec &objects, int every = 1);
    void MakeObjectsGarbage(size_t start_idx, size_t after_end_idx, int every = 1);
    void DumpHandles();
    void DumpAliveObjects();
    void DeleteHandles();
    bool IsInYoung(uintptr_t addr);
    MemOpReport HelpAllocTenured();

    bool NeedToCheckYoungFreedCount()
    {
        return (gc_type != GCType::G1_GC) || Runtime::GetOptions().IsG1TrackFreedObjects();
    }

    template <class LanguageConfig>
    void PrepareTest();

    template <class LanguageConfig>
    typename GenerationalGC<LanguageConfig>::MemStats *GetGenMemStats();

    // Allocate a series of objects in a specific space. If DO_SAVE is true, a subsequence of objects
    // is going to be kept alive and put into the roots array this->root_
    // If IS_SINGLE is true, then only 1 object is allocated of unaligned size
    // If IS_SINGLE is false, then an array of objects of different sizes is allocated in triplets twice
    // Saved subsequence contains 2 equal subsequences of objects (2 of 3 objs in each triplets are garbage)
    template <MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE = false, bool IS_SIMPLE = false>
    typename MemStatsGenGCTest::MemOpReport MakeAllocations();

    template <typename T>
    RealStatsLocations GetGenMemStatsDetails(T gms);

    void TearDown() override {}

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    panda::MTManagedThread *thread {};
    GCType gc_type {};

    LanguageContext ctx {nullptr};
    ObjectAllocatorBase *object_allocator {};
    mem::InternalAllocatorPtr internal_allocator;
    PandaVM *vm {};
    GC *gc {};
    std::vector<HanVec> handles;
    MemStatsType *ms {};
    GCStats *gc_ms {};
    coretypes::Array *root = nullptr;
    size_t root_size = 0;
    GCCounter *gccnt {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE>
MemStatsGenGCTest::ObjVec MemStatsGenGCTest::MakeAllocationsWithRepeats(size_t min_size, size_t max_size, size_t count,
                                                                        size_t *allocated, size_t *requested,
                                                                        [[maybe_unused]] F space_checker,
                                                                        bool check_oom_in_tenured)
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
    result.reserve(count * REPEAT);
    for (size_t j = 0; j < count; ++j) {
        size_t size = obj_templates[j].length() + sizeof(coretypes::String);
        if (check_oom_in_tenured) {
            // Leaving 5MB in tenured seems OK
            auto free =
                reinterpret_cast<GenerationalSpaces *>(object_allocator->GetHeapSpace())->GetCurrentFreeTenuredSize();
            // NOLINTNEXTLINE(readability-magic-numbers)
            if (size + 5000000 > free) {
                return result;
            }
        }
        for (size_t i = 0; i < REPEAT; ++i) {
            coretypes::String *string_obj = coretypes::String::CreateFromMUtf8(
                reinterpret_cast<const uint8_t *>(&obj_templates[j][0]), obj_templates[j].length(), ctx, vm);
            ASSERT(string_obj != nullptr);
            ASSERT(space_checker(ToUintPtr(string_obj)) == true);
            if (gc_type == GCType::G1_GC && SPACE == TargetSpace::HUMONGOUS) {
                // for humongous objects in G1 we calculate size of the region instead of just alignment size
                Region *region = AddrToRegion(string_obj);
                *allocated += region->Size();
            } else {
                *allocated += GetAlignedObjectSize(size);
            }
            *requested += size;
            result.push_back(string_obj);
        }
    }
    return result;
}

void MemStatsGenGCTest::InitRoot()
{
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    Class *klass = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    root = coretypes::Array::Create(klass, ROOT_MAX_SIZE);
    root_size = 0;
    MakeObjectsPermAlive({root});
}

void MemStatsGenGCTest::MakeObjectsAlive(const ObjVec &objects, int every)
{
    int cnt = every;
    for (auto *obj : objects) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        root->Set(root_size, obj);
        root_size++;
        ASSERT(root_size < ROOT_MAX_SIZE);
        cnt = every;
    }
}

void MemStatsGenGCTest::MakeObjectsGarbage(size_t start_idx, size_t after_end_idx, int every)
{
    int cnt = every;
    for (size_t i = start_idx; i < after_end_idx; ++i) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        root->Set(i, 0);
        cnt = every;
    }
}

void MemStatsGenGCTest::MakeObjectsPermAlive(const ObjVec &objects, int every)
{
    HanVec result;
    result.reserve(objects.size() / every);
    int cnt = every;
    for (auto *obj : objects) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        result.push_back(internal_allocator->New<VMHandle<ObjectHeader *>>(thread, obj));
        cnt = every;
    }
    handles.push_back(result);
}

void MemStatsGenGCTest::DumpHandles()
{
    for (auto &hv : handles) {
        for (auto *handle : hv) {
            std::cout << "vector " << (void *)&hv << " handle " << (void *)handle << " obj " << handle->GetPtr()
                      << std::endl;
        }
    }
}

void MemStatsGenGCTest::DumpAliveObjects()
{
    std::cout << "Alive root array : " << handles[0][0]->GetPtr() << std::endl;
    for (size_t i = 0; i < root_size; ++i) {
        if (root->Get<ObjectHeader *>(i) != nullptr) {
            std::cout << "Alive idx " << i << " : " << root->Get<ObjectHeader *>(i) << std::endl;
        }
    }
}

void MemStatsGenGCTest::DeleteHandles()
{
    for (auto &hv : handles) {
        for (auto *handle : hv) {
            internal_allocator->Delete(handle);
        }
    }
    handles.clear();
}

template <class LanguageConfig>
void MemStatsGenGCTest::PrepareTest()
{
    if constexpr (std::is_same<LanguageConfig, panda::PandaAssemblyLanguageConfig>::value) {
        DeleteHandles();
        ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        object_allocator = thread->GetVM()->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
        vm = Runtime::GetCurrent()->GetPandaVM();
        internal_allocator = Runtime::GetCurrent()->GetClassLinker()->GetAllocator();
        gc = vm->GetGC();
        ms = vm->GetMemStats();
        gc_ms = vm->GetGCStats();
        gccnt = internal_allocator->New<GCCounter>();
        gc->AddListener(gccnt);
        InitRoot();
    } else {
        UNREACHABLE();
    }
}

template <class LanguageConfig>
typename GenerationalGC<LanguageConfig>::MemStats *MemStatsGenGCTest::GetGenMemStats()
{
    // An explicit getter, because the typename has to be template-specialized
    return &reinterpret_cast<GenerationalGC<LanguageConfig> *>(gc)->mem_stats_;
}

bool MemStatsGenGCTest::IsInYoung(uintptr_t addr)
{
    switch (gc_type) {
        case GCType::GEN_GC: {
            return object_allocator->IsObjectInYoungSpace(reinterpret_cast<ObjectHeader *>(addr));
        }
        case GCType::G1_GC: {
            auto mem_pool = PoolManager::GetMmapMemPool();
            if (mem_pool->GetSpaceTypeForAddr(reinterpret_cast<ObjectHeader *>(addr)) != SpaceType::SPACE_TYPE_OBJECT) {
                return false;
            }
            return AddrToRegion(reinterpret_cast<ObjectHeader *>(addr))->HasFlag(RegionFlag::IS_EDEN);
        }
        default:
            UNREACHABLE();  // NYI
    }
    return false;
}

template <MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE, bool IS_SINGLE>
typename MemStatsGenGCTest::MemOpReport MemStatsGenGCTest::MakeAllocations()
{
    [[maybe_unused]] int gc_cnt = gccnt->count;
    MemStatsGenGCTest::MemOpReport report {};
    report.allocated_count = 0;
    report.allocated_bytes = 0;
    report.saved_count = 0;
    report.saved_bytes = 0;
    size_t bytes = 0;
    [[maybe_unused]] size_t raw_objects_size;  // currently not tracked by memstats
    size_t count = 0;
    size_t min_size = 0;
    size_t max_size = 0;
    bool check_oom = false;
    size_t young_size = reinterpret_cast<GenerationalSpaces *>(
                            reinterpret_cast<ObjectAllocatorGenBase *>(object_allocator)->GetHeapSpace())
                            ->GetCurrentYoungSize();
    switch (gc_type) {
        case GCType::GEN_GC: {
            auto gen_alloc = reinterpret_cast<ObjectAllocatorGen<MT_MODE_MULTI> *>(object_allocator);
            // NOLINTNEXTLINE(readability-magic-numbers)
            count = 15;
            if constexpr (SPACE == TargetSpace::YOUNG) {
                min_size = 0;
                max_size = gen_alloc->GetYoungAllocMaxSize();
            } else if constexpr (SPACE == TargetSpace::TENURED_REGULAR) {
                min_size = gen_alloc->GetYoungAllocMaxSize() + 1;
                max_size = gen_alloc->GetRegularObjectMaxSize();
                if (min_size >= max_size) {
                    // Allocator configuration disallows allocating directly in this space
                    return report;
                }
            } else if constexpr (SPACE == TargetSpace::TENURED_LARGE) {
                min_size = gen_alloc->GetYoungAllocMaxSize() + 1;
                min_size = std::max(min_size, gen_alloc->GetRegularObjectMaxSize() + 1);
                max_size = gen_alloc->GetLargeObjectMaxSize();
                if (min_size >= max_size) {
                    // Allocator configuration disallows allocating directly in this space
                    return report;
                }
            } else {
                ASSERT(SPACE == TargetSpace::HUMONGOUS);
                count = 3;
                min_size = gen_alloc->GetYoungAllocMaxSize() + 1;
                min_size = std::max(min_size, gen_alloc->GetRegularObjectMaxSize() + 1);
                min_size = std::max(min_size, gen_alloc->GetLargeObjectMaxSize() + 1);
                max_size = min_size * 3;
                check_oom = true;
            }
            break;
        }
        case GCType::G1_GC: {
            auto g1_alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(object_allocator);
            // NOLINTNEXTLINE(readability-magic-numbers)
            count = 15;
            if constexpr (SPACE == TargetSpace::YOUNG) {
                min_size = 0;
                max_size = g1_alloc->GetYoungAllocMaxSize();
            } else if constexpr (SPACE == TargetSpace::TENURED_REGULAR) {
                min_size = g1_alloc->GetYoungAllocMaxSize() + 1;
                max_size = g1_alloc->GetRegularObjectMaxSize();
                if (min_size >= max_size) {
                    // Allocator configuration disallows allocating directly in this space
                    return report;
                }
            } else if constexpr (SPACE == TargetSpace::TENURED_LARGE) {
                min_size = g1_alloc->GetYoungAllocMaxSize() + 1;
                min_size = std::max(min_size, g1_alloc->GetRegularObjectMaxSize() + 1);
                max_size = g1_alloc->GetLargeObjectMaxSize();
                if (min_size >= max_size) {
                    // Allocator configuration disallows allocating directly in this space
                    return report;
                }
            } else {
                ASSERT(SPACE == TargetSpace::HUMONGOUS);
                count = 3;
                min_size = g1_alloc->GetYoungAllocMaxSize() + 1;
                min_size = std::max(min_size, g1_alloc->GetRegularObjectMaxSize() + 1);
                min_size = std::max(min_size, g1_alloc->GetLargeObjectMaxSize() + 1);
                max_size = min_size * 3;
                check_oom = true;
            }
            break;
        }
        default:
            UNREACHABLE();
    }

    auto space_check = [&](uintptr_t addr) -> bool {
        if constexpr (SPACE == TargetSpace::YOUNG) {
            return IsInYoung(addr);
        } else if constexpr (SPACE == TargetSpace::TENURED_REGULAR) {
            return !IsInYoung(addr);
        } else if constexpr (SPACE == TargetSpace::TENURED_LARGE) {
            return !IsInYoung(addr);
        } else if constexpr (SPACE == TargetSpace::HUMONGOUS) {
            return !IsInYoung(addr);
        }
        UNREACHABLE();
    };

    if constexpr (SPACE == TargetSpace::YOUNG) {
        // To prevent Young GC collection while we're allocating
        max_size = std::min(young_size / (count * 6), max_size);
    }

    if (IS_SINGLE) {
        ObjVec ov1 = MakeAllocationsWithRepeats<decltype(space_check), 1, SPACE>(
            min_size + 1, max_size, 1, &bytes, &raw_objects_size, space_check, check_oom);
        report.allocated_count += 1;
        report.allocated_bytes += bytes;
        if constexpr (DO_SAVE) {
            MakeObjectsAlive(ov1, 1);
            report.saved_count = report.allocated_count;
            report.saved_bytes = report.allocated_bytes;
        }
    } else {
        ObjVec ov1 = MakeAllocationsWithRepeats<decltype(space_check), 3, SPACE>(
            min_size, max_size, count, &bytes, &raw_objects_size, space_check, check_oom);
        report.allocated_count += count * 3;
        report.allocated_bytes += bytes;
        ObjVec ov2 = MakeAllocationsWithRepeats<decltype(space_check), 3, SPACE>(
            min_size, max_size, count, &bytes, &raw_objects_size, space_check, check_oom);
        report.allocated_count += count * 3;
        report.allocated_bytes += bytes;
        if constexpr (DO_SAVE) {
            MakeObjectsAlive(ov1, 3);
            MakeObjectsAlive(ov2, 3);
            report.saved_count = report.allocated_count / 3;
            report.saved_bytes = report.allocated_bytes / 3;
        }
    }

    // We must not have uncounted GCs
    ASSERT(gc_cnt == gccnt->count);
    return report;
}

typename MemStatsGenGCTest::MemOpReport MemStatsGenGCTest::HelpAllocTenured()
{
    MemStatsGenGCTest::MemOpReport report {};
    report.allocated_count = 0;
    report.allocated_bytes = 0;
    report.saved_count = 0;
    report.saved_bytes = 0;

    auto old_root_size = root_size;

    // One way to get objects into tenured space - by promotion
    auto r = MakeAllocations<TargetSpace::YOUNG, true>();
    gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
    MakeObjectsGarbage(old_root_size, old_root_size + (root_size - old_root_size) / 2);

    report.allocated_count = r.saved_count;
    report.allocated_bytes = r.saved_bytes;
    report.saved_count = r.saved_count / 2;
    report.saved_bytes = r.saved_bytes / 2;

    // Another way - by direct allocation in tenured if possible
    auto r2 = MakeAllocations<TargetSpace::TENURED_REGULAR, true>();

    report.allocated_count += r2.allocated_count;
    report.allocated_bytes += r2.allocated_bytes;
    report.saved_count += r2.saved_count;
    report.saved_bytes += r2.saved_bytes;

    // Large objects are also tenured in terms of gen memstats
    auto r3 = MakeAllocations<TargetSpace::TENURED_LARGE, true>();

    report.allocated_count += r3.allocated_count;
    report.allocated_bytes += r3.allocated_bytes;
    report.saved_count += r3.saved_count;
    report.saved_bytes += r3.saved_bytes;

    auto r4 = MakeAllocations<TargetSpace::HUMONGOUS, true>();

    report.allocated_count += r4.allocated_count;
    report.allocated_bytes += r4.allocated_bytes;
    report.saved_count += r4.saved_count;
    report.saved_bytes += r4.saved_bytes;
    return report;
}

template <typename T>
MemStatsGenGCTest::RealStatsLocations MemStatsGenGCTest::GetGenMemStatsDetails(T gms)
{
    RealStatsLocations loc {};
    loc.young_freed_objects_count = &gms->young_free_object_count_;
    loc.young_freed_objects_size = &gms->young_free_object_size_;
    loc.young_moved_objects_count = &gms->young_move_object_count_;
    loc.young_moved_objects_size = &gms->young_move_object_size_;
    loc.tenured_freed_objects_count = &gms->tenured_free_object_count_;
    loc.tenured_freed_objects_size = &gms->tenured_free_object_size_;
    return loc;
}

TEST_F(MemStatsGenGCTest, TrivialStatsGenGcTest)
{
    for (int gctype_idx = 0; static_cast<GCType>(gctype_idx) <= GCType::GCTYPE_LAST; ++gctype_idx) {
        auto gc_type_local = static_cast<GCType>(gctype_idx);
        if (gc_type_local == GCType::EPSILON_G1_GC || gc_type_local == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(gc_type_local)) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(gc_type_local));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<panda::PandaAssemblyLanguageConfig>();
                auto *gen_ms = GetGenMemStats<panda::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(gen_ms)>(gen_ms);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));  // Heap doesn't have unexpected garbage now

                // Make a trivial allocation of unaligned size and make it garbage
                auto r = MakeAllocations<TargetSpace::YOUNG, false, true>();
                gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
                ASSERT_EQ(2, gccnt->count);
                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(*loc.young_freed_objects_count, r.allocated_count);
                }
                ASSERT_EQ(*loc.young_freed_objects_size, r.allocated_bytes);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_size, 0);
                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(gc_ms->GetObjectsFreedCount(), r.allocated_count);
                }
                if (PANDA_TRACK_TLAB_ALLOCATIONS) {
                    ASSERT_EQ(gc_ms->GetObjectsFreedBytes(), r.allocated_bytes);
                }
                ASSERT_EQ(gc_ms->GetLargeObjectsFreedCount(), 0);
                ASSERT_EQ(gc_ms->GetLargeObjectsFreedBytes(), 0);

                // Make a trivial allocation of unaligned size and make it alive
                r = MakeAllocations<TargetSpace::YOUNG, true, true>();
                gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
                ASSERT_EQ(3, gccnt->count);
                ASSERT_EQ(*loc.young_freed_objects_count, 0);
                ASSERT_EQ(*loc.young_freed_objects_size, 0);
                ASSERT_EQ(*loc.young_moved_objects_count, r.saved_count);
                ASSERT_EQ(*loc.young_moved_objects_size, r.saved_bytes);
                ASSERT_EQ(*loc.tenured_freed_objects_count, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_size, 0);

                // Expecting that r.saved_bytes/count have been promoted into tenured
                // Make them garbage
                MakeObjectsGarbage(0, root_size);
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(4, gccnt->count);
                ASSERT_EQ(*loc.young_freed_objects_count, 0);
                ASSERT_EQ(*loc.young_freed_objects_size, 0);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, r.saved_count);
                ASSERT_EQ(*loc.tenured_freed_objects_size, r.saved_bytes);

                // Make a trivial allocation of unaligned size in tenured space and make it garbage
                r = MakeAllocations<TargetSpace::TENURED_REGULAR, false, true>();
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(5, gccnt->count);
                ASSERT_EQ(*loc.young_freed_objects_count, 0);
                ASSERT_EQ(*loc.young_freed_objects_size, 0);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, r.allocated_count);
                ASSERT_EQ(*loc.tenured_freed_objects_size, r.allocated_bytes);

                // Make a trivial allocation of unaligned size large object and make it garbage
                r = MakeAllocations<TargetSpace::TENURED_LARGE, false, true>();
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(6, gccnt->count);
                ASSERT_EQ(*loc.young_freed_objects_count, 0);
                ASSERT_EQ(*loc.young_freed_objects_size, 0);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, r.allocated_count);
                ASSERT_EQ(*loc.tenured_freed_objects_size, r.allocated_bytes);

                r = MakeAllocations<TargetSpace::HUMONGOUS, false, true>();
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(*loc.young_freed_objects_count, 0);
                ASSERT_EQ(*loc.young_freed_objects_size, 0);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, r.allocated_count);
                ASSERT_EQ(*loc.tenured_freed_objects_size, r.allocated_bytes);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, YoungStatsGenGcTest)
{
    for (int gctype_idx = 0; static_cast<GCType>(gctype_idx) <= GCType::GCTYPE_LAST; ++gctype_idx) {
        if (static_cast<GCType>(gctype_idx) == GCType::EPSILON_G1_GC ||
            static_cast<GCType>(gctype_idx) == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(static_cast<GCType>(gctype_idx))) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(static_cast<GCType>(gctype_idx)));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<panda::PandaAssemblyLanguageConfig>();
                auto *gen_ms = GetGenMemStats<panda::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(gen_ms)>(gen_ms);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                // Young shall be empty now.
                auto r = MakeAllocations<TargetSpace::YOUNG, true>();
                gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));

                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(*loc.young_freed_objects_count, r.allocated_count - r.saved_count);
                }
                ASSERT_EQ(*loc.young_freed_objects_size, r.allocated_bytes - r.saved_bytes);
                ASSERT_EQ(*loc.young_moved_objects_count, r.saved_count);
                ASSERT_EQ(*loc.young_moved_objects_size, r.saved_bytes);
                ASSERT_EQ(*loc.tenured_freed_objects_count, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_size, 0);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, TenuredStatsFullGenGcTest)
{
    for (int gctype_idx = 0; static_cast<GCType>(gctype_idx) <= GCType::GCTYPE_LAST; ++gctype_idx) {
        if (static_cast<GCType>(gctype_idx) == GCType::EPSILON_G1_GC ||
            static_cast<GCType>(gctype_idx) == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(static_cast<GCType>(gctype_idx))) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(static_cast<GCType>(gctype_idx)));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<panda::PandaAssemblyLanguageConfig>();
                auto *gen_ms = GetGenMemStats<panda::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(gen_ms)>(gen_ms);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                // Young shall be empty now.

                uint32_t t_count = 0;
                uint64_t t_bytes = 0;

                for (int i = 0; i < FULL_TEST_ALLOC_TIMES; ++i) {
                    [[maybe_unused]] int gc_cnt = gccnt->count;
                    auto r = HelpAllocTenured();
                    // HelpAllocTenured shall trigger young gc, which is allowed to be mixed
                    ASSERT(gc_cnt + 1 == gccnt->count);
                    auto tfoc_y = *loc.tenured_freed_objects_count;
                    auto tfos_y = *loc.tenured_freed_objects_size;
                    ASSERT(r.allocated_count > 0);
                    gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                    ASSERT_EQ(*loc.young_freed_objects_count, 0);
                    ASSERT_EQ(*loc.young_freed_objects_size, 0);
                    ASSERT_EQ(*loc.young_moved_objects_count, 0);
                    ASSERT_EQ(*loc.young_moved_objects_size, 0);
                    ASSERT_EQ(*loc.tenured_freed_objects_count + tfoc_y, r.allocated_count - r.saved_count);
                    ASSERT_EQ(*loc.tenured_freed_objects_size + tfos_y, r.allocated_bytes - r.saved_bytes);
                    t_count += r.saved_count;
                    t_bytes += r.saved_bytes;
                }

                // Empty everything
                auto ry = MakeAllocations<TargetSpace::YOUNG, false>();
                MakeObjectsGarbage(0, root_size);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(*loc.young_freed_objects_count, ry.allocated_count);
                }
                ASSERT_EQ(*loc.young_freed_objects_size, ry.allocated_bytes);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, t_count);
                ASSERT_EQ(*loc.tenured_freed_objects_size, t_bytes);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(*loc.young_freed_objects_count, 0);
                ASSERT_EQ(*loc.young_freed_objects_size, 0);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_size, 0);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, TenuredStatsMixGenGcTest)
{
    for (int gctype_idx = 0; static_cast<GCType>(gctype_idx) <= GCType::GCTYPE_LAST; ++gctype_idx) {
        if (static_cast<GCType>(gctype_idx) == GCType::EPSILON_G1_GC ||
            static_cast<GCType>(gctype_idx) == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(static_cast<GCType>(gctype_idx))) {
            continue;
        }
        if (static_cast<GCType>(gctype_idx) == GCType::GEN_GC) {
            // Doesn't have mixed GC collection
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(static_cast<GCType>(gctype_idx)));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<panda::PandaAssemblyLanguageConfig>();
                GCTaskCause mixed_cause;
                switch (gc_type) {
                    case GCType::GEN_GC: {
                        UNREACHABLE();  // Doesn't have mixed GC collection
                    }
                    case GCType::G1_GC: {
                        mixed_cause = MIXED_G1_GC_CAUSE;
                        break;
                    }
                    default:
                        UNREACHABLE();  // NIY
                }
                auto *gen_ms = GetGenMemStats<panda::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(gen_ms)>(gen_ms);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                // Young shall be empty now.

                uint32_t t_count = 0;
                uint64_t t_bytes = 0;

                {
                    uint32_t dead_count = 0;
                    uint64_t dead_bytes = 0;
                    uint32_t expected_dead_count = 0;
                    uint64_t expected_dead_bytes = 0;
                    for (int i = 0; i < MIX_TEST_ALLOC_TIMES; ++i) {
                        [[maybe_unused]] int gc_cnt = gccnt->count;
                        auto r = HelpAllocTenured();
                        // HelpAllocTenured shall trigger young gc, which is allowed to be mixed
                        ASSERT(gc_cnt + 1 == gccnt->count);
                        dead_count += *loc.tenured_freed_objects_count;
                        dead_bytes += *loc.tenured_freed_objects_size;
                        // Mixed can free not all the tenured garbage, so run it until it stalls
                        do {
                            gc->WaitForGCInManaged(GCTask(mixed_cause));
                            ASSERT_EQ(*loc.young_freed_objects_count, 0);
                            ASSERT_EQ(*loc.young_freed_objects_size, 0);
                            ASSERT_EQ(*loc.young_moved_objects_count, 0);
                            ASSERT_EQ(*loc.young_moved_objects_size, 0);
                            dead_count += *loc.tenured_freed_objects_count;
                            dead_bytes += *loc.tenured_freed_objects_size;
                        } while (*loc.tenured_freed_objects_count != 0);
                        t_count += r.saved_count;
                        t_bytes += r.saved_bytes;
                        expected_dead_count += r.allocated_count - r.saved_count;
                        expected_dead_bytes += r.allocated_bytes - r.saved_bytes;
                    }
                    gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                    ASSERT_EQ(*loc.young_freed_objects_count, 0);
                    ASSERT_EQ(*loc.young_freed_objects_size, 0);
                    ASSERT_EQ(*loc.young_moved_objects_count, 0);
                    ASSERT_EQ(*loc.young_moved_objects_size, 0);
                    dead_count += *loc.tenured_freed_objects_count;
                    dead_bytes += *loc.tenured_freed_objects_size;
                    ASSERT_EQ(dead_count, expected_dead_count);
                    ASSERT_EQ(dead_bytes, expected_dead_bytes);
                }

                // Empty everything
                auto ry = MakeAllocations<TargetSpace::YOUNG, false>();
                MakeObjectsGarbage(0, root_size);
                {
                    uint32_t dead_count = 0;
                    uint64_t dead_bytes = 0;
                    do {
                        gc->WaitForGCInManaged(GCTask(mixed_cause));
                        if (NeedToCheckYoungFreedCount()) {
                            ASSERT_EQ(*loc.young_freed_objects_count, ry.allocated_count);
                        }
                        ASSERT_EQ(*loc.young_freed_objects_size, ry.allocated_bytes);
                        ASSERT_EQ(*loc.young_moved_objects_count, 0);
                        ASSERT_EQ(*loc.young_moved_objects_size, 0);
                        dead_count += *loc.tenured_freed_objects_count;
                        dead_bytes += *loc.tenured_freed_objects_size;
                        ry.allocated_count = 0;
                        ry.allocated_bytes = 0;
                    } while (*loc.tenured_freed_objects_count != 0);
                    gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                    ASSERT_EQ(*loc.young_freed_objects_count, 0);
                    ASSERT_EQ(*loc.young_freed_objects_size, 0);
                    ASSERT_EQ(*loc.young_moved_objects_count, 0);
                    ASSERT_EQ(*loc.young_moved_objects_size, 0);
                    dead_count += *loc.tenured_freed_objects_count;
                    dead_bytes += *loc.tenured_freed_objects_size;
                    ASSERT_EQ(dead_count, t_count);
                    ASSERT_EQ(dead_bytes, t_bytes);
                }
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(*loc.young_freed_objects_count, 0);
                ASSERT_EQ(*loc.young_freed_objects_size, 0);
                ASSERT_EQ(*loc.young_moved_objects_count, 0);
                ASSERT_EQ(*loc.young_moved_objects_size, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_count, 0);
                ASSERT_EQ(*loc.tenured_freed_objects_size, 0);
            }

            ResetRuntime();
        }
    }
}
}  // namespace panda::mem::test
