/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <memory>

#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/gc_task.h"
#include "runtime/mem/gc/cmc/cmc-gc.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/gc/cmc/common_components/mutator/satb_buffer.h"

#include "runtime/tests/test_utils.h"
#include "runtime/tests/interpreter_test_utils.h"

namespace ark::mem {

namespace cvm = ark::common_vm;

class CMCGCTest : public testing::Test {
public:
    void SetUp() override
    {
        Runtime::Create(CreateRuntimeOptions());
        thread_ = MTManagedThread::GetCurrent();
        gc_ = static_cast<CmcGC<PandaAssemblyLanguageConfig> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC());
    }

    void TearDown() override
    {
        Runtime::Destroy();
        thread_ = nullptr;
        gc_ = nullptr;
        cvm::ThreadLocal::SetAllocBuffer(nullptr);
    }

    static constexpr size_t LargeStringLength()
    {
        return cvm::RegionDesc::LARGE_OBJECT_DEFAULT_THRESHOLD - sizeof(coretypes::LineString);
    }

    static constexpr size_t LargeObjectArrayLength()
    {
        return (cvm::RegionDesc::LARGE_OBJECT_DEFAULT_THRESHOLD - sizeof(coretypes::Array)) / OBJECT_POINTER_SIZE;
    }

    coretypes::String *AllocString(size_t length)
    {
        return ObjectAllocator::AllocString(length);
    }

    coretypes::String *AllocEmptyNonMovableString()
    {
        return ObjectAllocator::AllocString(0, false, false);
    }

    coretypes::String *AllocEmptyString()
    {
        return AllocString(0);
    }

    coretypes::Array *AllocArray(size_t length, ClassRoot classRoot)
    {
        return ObjectAllocator::AllocArray(length, classRoot, false, false);
    }

    coretypes::Array *AllocNonMovableArray(size_t length, ClassRoot classRoot)
    {
        return ObjectAllocator::AllocArray(length, classRoot, true, false);
    }

    void FillCurrentRegion(ObjectHeader *obj = nullptr)
    {
        // Object size must be less then LARGE_OBJECT_DEFAULT_THRESHOLD
        // CC-OFFNXT(G.NAM.03-CPP) project code style
        static constexpr size_t OBJECT_SIZE = cvm::RegionDesc::LARGE_OBJECT_DEFAULT_THRESHOLD - sizeof(ObjectHeader);
        // CC-OFFNXT(G.NAM.03-CPP) project code style
        static constexpr size_t STRING_LENGTH = OBJECT_SIZE - sizeof(coretypes::LineString);
        if (obj == nullptr) {
            obj = AllocString(STRING_LENGTH);
        }
        auto *region = cvm::RegionDesc::GetRegionDescAt(obj);
        cvm::RegionDesc *r = nullptr;
        do {
            auto *o = AllocString(STRING_LENGTH);
            r = cvm::RegionDesc::GetRegionDescAt(o);
        } while (r == region);
    }

    void TriggerGC(GCTaskCause cause)
    {
        GCTask task(cause);
        gc_->WaitForGCInManaged(task);
    }

    template <class T>
    void PromoteToOld(VMHandle<T> obj)
    {
        PromoteToOld(obj.GetPtr());
    }

    void PromoteToOld(ObjectHeader *obj)
    {
        FillCurrentRegion(obj);
        TriggerGC(GCTaskCause::YOUNG_GC_CAUSE);
    }

    template <class T>
    cvm::RegionDesc::RegionType GetRegionType(VMHandle<T> obj)
    {
        return GetRegionType(obj.GetPtr());
    }

    cvm::RegionDesc::RegionType GetRegionType(ObjectHeader *obj)
    {
        return cvm::RegionDesc::GetRegionDescAt(obj)->GetRegionType();
    }

private:
    static RuntimeOptions CreateRuntimeOptions()
    {
        RuntimeOptions options;
        options.SetInterpreterType("cpp");
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("cmc-gc");
        options.SetLoadRuntimes({"core"});
        options.SetGcTriggerType("debug-never");
        options.SetRunGcInPlace(true);

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "Error: PANDA_STD_LIB env variable is empty\n";
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        return options;
    }

protected:
    MTManagedThread *thread_ = nullptr;
    CmcGC<PandaAssemblyLanguageConfig> *gc_ = nullptr;
};

TEST_F(CMCGCTest, AllocMovableObject)
{
    ScopedManagedCodeThread managedScope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    VMHandle<coretypes::String> obj(thread_, AllocEmptyString());
    auto *addr = obj.GetPtr();
    ASSERT_EQ(cvm::RegionDesc::RegionType::THREAD_LOCAL_REGION, GetRegionType(obj));
    FillCurrentRegion(obj.GetPtr());
    ASSERT_EQ(cvm::RegionDesc::RegionType::RECENT_FULL_REGION, GetRegionType(obj));
    TriggerGC(GCTaskCause::YOUNG_GC_CAUSE);
    ASSERT_NE(addr, obj.GetPtr());
    RegionDesc *region = RegionDesc::GetRegionDescAt(addr);
    ASSERT_EQ(cvm::RegionDesc::RegionType::FREE_REGION, region->GetRegionType());
    ASSERT_EQ(cvm::RegionDesc::RegionType::OLD_REGION, GetRegionType(obj));
}

TEST_F(CMCGCTest, AllocateNonMovableObject)
{
    ScopedManagedCodeThread managedScope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    VMHandle<coretypes::String> obj(thread_, AllocEmptyNonMovableString());
    auto *objAddr = obj.GetPtr();
    ASSERT_EQ(cvm::RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION, GetRegionType(obj));
    FillCurrentRegion();
    TriggerGC(GCTaskCause::YOUNG_GC_CAUSE);
    ASSERT_EQ(objAddr, obj.GetPtr());
}

// The test is disabled because the current situation is strange.
// We cannot allocate an object in the large space becuase TLAB can fits it.
// Need to adjust sizes.
TEST_F(CMCGCTest, DISABLED_AllocateLargeObject)
{
    ScopedManagedCodeThread managedScope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    VMHandle<coretypes::String> obj(thread_, AllocString(LargeStringLength()));
    auto *objAddr = obj.GetPtr();
    ASSERT_EQ(cvm::RegionDesc::RegionType::LARGE_REGION, GetRegionType(obj));
    FillCurrentRegion();
    TriggerGC(GCTaskCause::YOUNG_GC_CAUSE);
    ASSERT_EQ(objAddr, obj.GetPtr());
}

TEST_F(CMCGCTest, CardTableTest)
{
    ScopedManagedCodeThread managedScope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    VMHandle<coretypes::Array> nonmovable(thread_, AllocNonMovableArray(1, ClassRoot::ARRAY_STRING));
    ASSERT_EQ(cvm::RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION, GetRegionType(nonmovable));
    VMHandle<coretypes::Array> large(thread_, AllocArray(LargeObjectArrayLength(), ClassRoot::ARRAY_STRING));
    // NOTE(artemu) Check the region is LARGE_REGION
    VMHandle<coretypes::Array> old(thread_, AllocArray(1, ClassRoot::ARRAY_STRING));
    PromoteToOld(old);
    ASSERT_EQ(cvm::RegionDesc::RegionType::OLD_REGION, GetRegionType(old));
    VMHandle<coretypes::String> young(thread_, AllocEmptyString());
    ASSERT_EQ(cvm::RegionDesc::RegionType::THREAD_LOCAL_REGION, GetRegionType(young));

    nonmovable->Set(0, young.GetPtr());
    old->Set(0, young.GetPtr());
    large->Set(0, young.GetPtr());

    cvm::RegionDesc *nonmovableRegion = cvm::RegionDesc::GetRegionDescAt(nonmovable.GetPtr());
    bool found = false;
    nonmovableRegion->GetRSet()->VisitAllMarkedCardBefore(
        [nonmovable, &found](BaseObject *obj) {
            if (ToUintPtr(obj) == ToUintPtr(nonmovable.GetPtr())) {
                found = true;
            }
        },
        nonmovableRegion->GetRegionBase(), ToUintPtr(nonmovable.GetPtr()) + OBJECT_POINTER_SIZE);
    ASSERT_TRUE(found);

    cvm::RegionDesc *largeRegion = cvm::RegionDesc::GetRegionDescAt(large.GetPtr());
    found = false;
    largeRegion->GetRSet()->VisitAllMarkedCardBefore(
        [large, &found](BaseObject *obj) {
            if (ToUintPtr(obj) == ToUintPtr(large.GetPtr())) {
                found = true;
            }
        },
        largeRegion->GetRegionBase(), ToUintPtr(large.GetPtr()) + OBJECT_POINTER_SIZE);
    ASSERT_TRUE(found);

    cvm::RegionDesc *oldRegion = cvm::RegionDesc::GetRegionDescAt(old.GetPtr());
    found = false;
    oldRegion->GetRSet()->VisitAllMarkedCardBefore(
        [old, &found](BaseObject *obj) {
            if (ToUintPtr(obj) == ToUintPtr(old.GetPtr())) {
                found = true;
            }
        },
        oldRegion->GetRegionBase(), ToUintPtr(old.GetPtr()) + OBJECT_POINTER_SIZE);
    ASSERT_TRUE(found);
}

TEST_F(CMCGCTest, TestVregRoot)
{
    ScopedManagedCodeThread scope(thread_);

    ObjectHeader *obj = AllocEmptyString();
    ASSERT_EQ(cvm::RegionDesc::RegionType::THREAD_LOCAL_REGION, GetRegionType(obj));
    auto frame = interpreter::test::CreateFrame(1U, nullptr, nullptr);
    StaticFrameHandler handler(frame.get());
    handler.GetVReg(0).SetReference(obj);
    auto *cls = interpreter::test::CreateClass(panda_file::SourceLang::PANDA_ASSEMBLY);
    std::vector<uint8_t> bytecode;
    BytecodeEmitter emitter;
    ASSERT_EQ(emitter.Build(&bytecode), BytecodeEmitter::ErrorCode::SUCCESS);
    auto [method, _] = interpreter::test::CreateMethod(cls, ACC_STATIC, 1, 1, nullptr, bytecode);
    frame->SetMethod(method.get());
    thread_->SetCurrentFrame(frame.get());

    PromoteToOld(obj);

    ObjectHeader *oldObj = handler.GetVReg(0).GetReference();
    ASSERT_NE(nullptr, oldObj);
    ASSERT_NE(obj, oldObj);
    ASSERT_EQ(cvm::RegionDesc::RegionType::OLD_REGION, GetRegionType(oldObj));
}

TEST_F(CMCGCTest, TestExceptionRoot)
{
    ScopedManagedCodeThread scope(thread_);

    ObjectHeader *ex = AllocEmptyString();
    ASSERT_NE(nullptr, ex);
    ASSERT_EQ(cvm::RegionDesc::RegionType::THREAD_LOCAL_REGION, GetRegionType(ex));

    thread_->SetException(ex);
    PromoteToOld(ex);
    ASSERT_NE(ex, thread_->GetException());
    ASSERT_EQ(cvm::RegionDesc::RegionType::OLD_REGION, GetRegionType(thread_->GetException()));
}

TEST_F(CMCGCTest, TestClassRoot)
{
    ScopedManagedCodeThread scope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    auto langCtx = thread_->GetVM()->GetLanguageContext();
    auto *linkerExt = Runtime::GetCurrent()->GetClassLinker()->GetExtension(langCtx);
    auto *clsName = reinterpret_cast<const uint8_t *>("Foo");
    auto *cls =
        linkerExt->CreateClass(clsName, 0, 0, AlignUp(sizeof(Class) + OBJECT_POINTER_SIZE, OBJECT_POINTER_SIZE));
    cls->SetRefFieldsNum(1, true);
    cls->SetState(Class::State::INITIALIZED);
    Field field(cls, static_cast<panda_file::File::EntityId>(0), 0,
                panda_file::Type(panda_file::Type::TypeId::REFERENCE));
    cls->SetFields(Span<Field>(&field, 1), 1);
    field.SetOffset(sizeof(ObjectHeader));

    ObjectHeader *value = AllocEmptyString();
    ObjectAccessor::SetFieldObject(cls->GetManagedObject(), field, value);

    PromoteToOld(value);
    ObjectHeader *newValue = ObjectAccessor::GetFieldObject(cls->GetManagedObject(), field);
    ASSERT_NE(value, newValue);
    ASSERT_EQ(cvm::RegionDesc::RegionType::OLD_REGION, GetRegionType(newValue));
}

TEST_F(CMCGCTest, TestGlobalObjectStorageRoot)
{
    ScopedManagedCodeThread scope(thread_);

    auto *storage = thread_->GetVM()->GetGlobalObjectStorage();
    ObjectHeader *obj = AllocEmptyString();
    auto *objRef = storage->Add(obj, Reference::ObjectType::GLOBAL);

    PromoteToOld(obj);
    ObjectHeader *newObj = storage->Get(objRef);
    ASSERT_NE(obj, newObj);
    ASSERT_EQ(cvm::RegionDesc::RegionType::OLD_REGION, GetRegionType(newObj));
}

TEST_F(CMCGCTest, TestRefUpdateAfterCopy)
{
    ScopedManagedCodeThread managedScope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    VMHandle<coretypes::Array> array(thread_, AllocArray(1, ClassRoot::ARRAY_STRING));
    PromoteToOld(array.GetPtr());
    ASSERT_EQ(cvm::RegionDesc::RegionType::OLD_REGION, GetRegionType(array));

    VMHandle<ObjectHeader> value(thread_, AllocEmptyString());
    ObjectHeader *oldAddr = value.GetPtr();
    array->Set(0, value.GetPtr());
    PromoteToOld(value.GetPtr());
    ASSERT_NE(oldAddr, value.GetPtr());
    ASSERT_EQ(value.GetPtr(), array->Get<ObjectHeader *>(0));
}

class PreBarrierCMCGCTest : public CMCGCTest, public GCListener {
public:
    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        markingDone_ = true;
        array_->Set(0, newObj_.GetPtr());
        const auto *node = static_cast<const cvm::SatbBuffer::TreapNode *>(thread_->GetSatbBufferNode());
        EXPECT_NE(nullptr, node);
        if (node != nullptr) {
            PandaStack<cvm::BaseObject *> objects;
            const_cast<cvm::SatbBuffer::TreapNode *>(node)->GetObjects(objects);
            size_t size = 0;
            while (!objects.empty()) {
                if (objects.top() == obj_.GetPtr()) {
                    return;
                }
                ++size;
                objects.pop();
            }
            FAIL() << "Object " << obj_.GetPtr() << " not faound in SATB buffer of size " << size;
        }
    }

protected:
    VMHandle<coretypes::Array> array_;
    VMHandle<ObjectHeader> obj_;
    VMHandle<ObjectHeader> newObj_;
    bool markingDone_ = false;
};

TEST_F(PreBarrierCMCGCTest, TestPreBarrier)
{
    ScopedManagedCodeThread managedScope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    array_ = VMHandle<coretypes::Array>(thread_, AllocArray(1, ClassRoot::ARRAY_STRING));
    obj_ = VMHandle<ObjectHeader>(thread_, AllocEmptyString());
    array_->Set(0, obj_.GetPtr());
    newObj_ = VMHandle<ObjectHeader>(thread_, AllocEmptyString());

    gc_->AddListener(this);

    TriggerGC(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
    ASSERT_TRUE(markingDone_);
}

class ReadBarrierCMCGCTest : public CMCGCTest, public GCListener, public testing::WithParamInterface<GCTaskCause> {
public:
    void GCStarted(const GCTask &task, size_t) override
    {
        reason_ = task.reason;
    }

    void GCPhaseStarted(GCPhase phase) override
    {
        if ((reason_ == GCTaskCause::YOUNG_GC_CAUSE && phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE) ||
            (reason_ == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE && phase == GCPhase::GC_PHASE_COPY)) {
            EXPECT_EQ(cvm::RegionDesc::RegionType::TO_REGION, GetRegionType(array_.GetPtr()));
            EXPECT_EQ(cvm::RegionDesc::RegionType::FROM_REGION, GetRegionType(obj_));
            EXPECT_FALSE(obj_->IsForwarded());
            ObjectHeader *fwdObj = array_->Get<ObjectHeader *>(0);
            EXPECT_EQ(cvm::RegionDesc::RegionType::TO_REGION, GetRegionType(fwdObj));
            EXPECT_TRUE(obj_->IsForwarded());
            copyDone_ = true;
        }
    }

protected:
    GCTaskCause reason_ = GCTaskCause::INVALID_CAUSE;
    VMHandle<coretypes::Array> array_;
    ObjectHeader *obj_;
    bool copyDone_ = false;
};

TEST_P(ReadBarrierCMCGCTest, TestReadBarrier)
{
    ScopedManagedCodeThread managedScope(thread_);
    HandleScope<ObjectHeader *> handleScope(thread_);

    array_ = VMHandle<coretypes::Array>(thread_, AllocArray(1, ClassRoot::ARRAY_STRING));
    obj_ = AllocEmptyString();
    array_->Set(0, obj_);
    FillCurrentRegion(obj_);
    gc_->AddListener(this);

    TriggerGC(GetParam());
    ASSERT_TRUE(copyDone_);
}
INSTANTIATE_TEST_SUITE_P(ReadBarrierCMCGCTestSuite, ReadBarrierCMCGCTest,
                         testing::Values(GCTaskCause::YOUNG_GC_CAUSE, GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE));

class NonOverlappingCMCGCTest : public CMCGCTest, public GCListener {
    enum class GCState {
        IDLE,
        RUNNING,
    };

public:
    void SetUp() override
    {
        Runtime::Create(CreateRuntimeOptions());
        thread_ = MTManagedThread::GetCurrent();
        gc_ = static_cast<CmcGC<PandaAssemblyLanguageConfig> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC());
    }

    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
    {
        // Atomic with seq_cst order reason: don't care about perf in test
        auto old = counter_.fetch_add(1, std::memory_order_seq_cst);
        auto oldState = (old % 2U == 0) ? GCState::IDLE : GCState::RUNNING;

        ASSERT_EQ(oldState, GCState::IDLE);
    }

    void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize)
    {
        // Atomic with seq_cst order reason: don't care about perf in test
        auto old = counter_.fetch_add(1, std::memory_order_seq_cst);
        auto oldState = (old % 2U == 0) ? GCState::IDLE : GCState::RUNNING;

        ASSERT_EQ(oldState, GCState::RUNNING);
    }

private:
    static RuntimeOptions CreateRuntimeOptions()
    {
        RuntimeOptions options;
        options.SetInterpreterType("cpp");
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("cmc-gc");
        options.SetLoadRuntimes({"core"});
        options.SetGcTriggerType("debug-never");
        options.SetRunGcInPlace(false);

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "Error: PANDA_STD_LIB env variable is empty\n";
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        return options;
    }

protected:
    std::atomic_uint32_t counter_ {0};
};

TEST_F(NonOverlappingCMCGCTest, TestOOMInterrupt)
{
    ASSERT_FALSE(gc_->GetSettings()->RunGCInPlace());
    gc_->AddListener(this);

    ScopedManagedCodeThread msc(thread_);
    FillCurrentRegion();
    gc_->Trigger(MakePandaUnique<ark::GCTask>(GCTaskCause::YOUNG_GC_CAUSE));
    // Atomic with seq_cst order reason: don't care about perf in test
    while (counter_.load(std::memory_order_seq_cst) < 2U) {
        FillCurrentRegion();
    }
}

}  // namespace ark::mem
