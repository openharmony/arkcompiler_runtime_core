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

#include "runtime/include/runtime.h"

#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/thread_scopes.h"
#include "common_interfaces/heap/region_desc.h"
#include "common_components/mutator/thread_local.h"
#include "common_components/mutator/satb_buffer.h"
#include "common_components/common/scoped_object_access.h"
#include "common_interfaces/objects/base_object.h"
#include "runtime/tests/interpreter_test_utils.h"
#include "runtime/include/gc_task.h"

namespace ark::mem {

namespace cvm = common_vm;

constexpr size_t TEST_ARRAY_SIZE = 1;
constexpr size_t TEST_CLASS_VTABLE_SIZE = 0;
constexpr size_t TEST_CLASS_IMT_SIZE = 0;
constexpr size_t INT_ARRAY_SIZE = 8;
constexpr std::array<uint32_t, INT_ARRAY_SIZE> INT_ARRAY = {8, 95, 34, 47, 74, 72, 29, 27};

class CMCGCPreBarrierChecker : public cvm::GCListener {
public:
    void OnGCStart(common_vm::GCReason reason, common_vm::GCType type) override {}
    void OnGCFinish(common_vm::GCReason reason, common_vm::GCType type) override {}
    void OnGCPhaseStart(common_vm::GCPhase phase) override
    {
        if (phase == cvm::GCPhase::GC_PHASE_MARK) {
            {
                auto &[lock, cond] = markStartedMon;
                std::unique_lock phaseScope(lock);
                markStarted = true;
                cond.notify_all();
            }
            {
                auto &[lock, cond] = markStartedActionMon;
                std::unique_lock actionScope(lock);
                cond.wait(actionScope, [this] { return markStartedActionCompleted; });
            }
        }
    }
    void OnGCPhaseEnd(common_vm::GCPhase phase) override
    {
        if (phase == cvm::GCPhase::GC_PHASE_MARK) {
            {
                auto &[lock, cond] = markFinishedMon;
                std::unique_lock phaseScope(lock);
                markFinished = true;
                cond.notify_all();
            }
            {
                auto &[lock, cond] = markFinishedActionMon;
                std::unique_lock actionScope(lock);
                cond.wait(actionScope, [this] { return markFinishedActionCompleted; });
            }
        }
    }
    template <typename A>
    void OnStartPhaseMark(A action)
    {
        {
            auto &[lock, cond] = markStartedMon;
            std::unique_lock phaseScope(lock);
            cond.wait(phaseScope, [this] { return markStarted; });
        }
        {
            auto &[lock, cond] = markStartedActionMon;
            std::unique_lock actionScope(lock);
            action();
            markStartedActionCompleted = true;
            cond.notify_all();
        }
    }
    template <typename A>
    void OnFinishPhaseMark(A action)
    {
        {
            auto &[lock, cond] = markFinishedMon;
            std::unique_lock phaseScope(lock);
            cond.wait(phaseScope, [this] { return markFinished; });
        }
        {
            auto &[lock, cond] = markFinishedActionMon;
            std::unique_lock actionScope(lock);
            action();
            markFinishedActionCompleted = true;
            cond.notify_all();
        }
    }
    ~CMCGCPreBarrierChecker()
    {
        cvm::BaseRuntime::GetInstance()->RemoveGCListener(this);
    }

private:
    bool markStarted = false;
    std::pair<std::mutex, std::condition_variable> markStartedMon;

    bool markStartedActionCompleted = false;
    std::pair<std::mutex, std::condition_variable> markStartedActionMon;

    bool markFinished = false;
    std::pair<std::mutex, std::condition_variable> markFinishedMon;

    bool markFinishedActionCompleted = false;
    std::pair<std::mutex, std::condition_variable> markFinishedActionMon;
};

class CMCGCReadBarrierChecker : public cvm::GCListener {
public:
    void OnGCStart(common_vm::GCReason reason, common_vm::GCType type) override {}
    void OnGCFinish(common_vm::GCReason reason, common_vm::GCType type) override {}
    void OnGCPhaseStart(common_vm::GCPhase phase) override
    {
        if (phase == cvm::GCPhase::GC_PHASE_MARK) {
            {
                auto &[lock, cond] = markStartedMon;
                std::unique_lock phaseScope(lock);
                markStarted = true;
                cond.notify_all();
            }
            {
                auto &[lock, cond] = markStartedActionMon;
                std::unique_lock actionScope(lock);
                cond.wait(actionScope, [this] { return markStartedActionCompleted; });
            }
        } else if (phase == cvm::GCPhase::GC_PHASE_PRECOPY) {
            {
                auto &[lock, cond] = precopyStartedMon;
                std::unique_lock phaseScope(lock);
                precopyStarted = true;
                cond.notify_all();
            }
            {
                auto &[lock, cond] = precopyStartedActionMon;
                std::unique_lock actionScope(lock);
                cond.wait(actionScope, [this] { return precopyStartedActionCompleted; });
            }
        }
    }
    void OnGCPhaseEnd(common_vm::GCPhase phase) override {}
    template <typename A>
    void OnStartPhaseMark(A action)
    {
        {
            auto &[lock, cond] = markStartedMon;
            std::unique_lock phaseScope(lock);
            cond.wait(phaseScope, [this] { return markStarted; });
        }
        {
            auto &[lock, cond] = markStartedActionMon;
            std::unique_lock actionScope(lock);
            action();
            markStartedActionCompleted = true;
            cond.notify_all();
        }
    }
    template <typename A>
    void OnStartPhasePrecopy(A action)
    {
        {
            auto &[lock, cond] = precopyStartedMon;
            std::unique_lock phaseScope(lock);
            cond.wait(phaseScope, [this] { return precopyStarted; });
        }
        {
            auto &[lock, cond] = precopyStartedActionMon;
            std::unique_lock actionScope(lock);
            action();
            precopyStartedActionCompleted = true;
            cond.notify_all();
        }
    }
    ~CMCGCReadBarrierChecker()
    {
        cvm::BaseRuntime::GetInstance()->RemoveGCListener(this);
    }

private:
    bool markStarted = false;
    std::pair<std::mutex, std::condition_variable> markStartedMon;

    bool markStartedActionCompleted = false;
    std::pair<std::mutex, std::condition_variable> markStartedActionMon;

    bool precopyStarted = false;
    std::pair<std::mutex, std::condition_variable> precopyStartedMon;

    bool precopyStartedActionCompleted = false;
    std::pair<std::mutex, std::condition_variable> precopyStartedActionMon;
};

class CMCGCTest : public testing::Test {
public:
    void FillCurrentMemRegion(ets::EtsExecutionContext *execCtx)
    {
        auto *vm = execCtx->GetPandaVM();
        auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
        auto *obj = ets::EtsObjectArray::Create(objCls, TEST_ARRAY_SIZE)->AsObject();
        auto *objRegion = cvm::RegionDesc::GetRegionDescAt(reinterpret_cast<uintptr_t>(obj));
        cvm::RegionDesc *curRegion = nullptr;
        do {
            auto *curObj = ets::EtsObjectArray::Create(objCls, TEST_ARRAY_SIZE)->AsObject();
            curRegion = cvm::RegionDesc::GetRegionDescAt(reinterpret_cast<uintptr_t>(curObj));
        } while (objRegion == curRegion);
    }

    void TriggerGC()
    {
        cvm::BaseRuntime::GetInstance()->RequestGC(cvm::GCReason::GC_REASON_FORCE, false, cvm::GCType::GC_TYPE_YOUNG);
        cvm::BaseRuntime::WaitForGCFinish();
    }

    static RuntimeOptions CreateRuntimeOptions()
    {
        RuntimeOptions options;
        options.SetInterpreterType("cpp");
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("cmc-gc");
        options.SetLoadRuntimes({"ets"});
        options.SetGcTriggerType("debug-never");

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        return options;
    }

    CMCGCTest() : CMCGCTest(CreateRuntimeOptions()) {}

    explicit CMCGCTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~CMCGCTest()
    {
        bool success = Runtime::Destroy();
        ASSERT(success);
        Logger::Destroy();
        cvm::ThreadLocal::SetAllocBuffer(nullptr);
    }

    NO_COPY_SEMANTIC(CMCGCTest);
    NO_MOVE_SEMANTIC(CMCGCTest);
};

TEST_F(CMCGCTest, AllocEtsMovableObject)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *vm = execCtx->GetPandaVM();
    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);

    ets::EtsObject *arr = ets::EtsObjectArray::Create(objCls, TEST_ARRAY_SIZE)->AsObject();
    ets::EtsHandleScope scope(execCtx);
    ets::EtsHandle<ets::EtsObject> arrHandle(execCtx, arr);
    auto *arrPtr = arrHandle.GetPtr();
    FillCurrentMemRegion(execCtx);
    TriggerGC();
    ASSERT_NE(arrPtr, arrHandle.GetPtr());
}

TEST_F(CMCGCTest, AllocateNonMovableObject)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto langCtx = execCtx->GetPandaVM()->GetLanguageContext();
    auto *runtime = Runtime::GetCurrent();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);

    auto *linkerExt = runtime->GetClassLinker()->GetExtension(langCtx);
    std::string className("Foo");
    Class *coreCls =
        linkerExt->CreateClass(reinterpret_cast<const uint8_t *>(className.data()), TEST_CLASS_VTABLE_SIZE,
                               TEST_CLASS_IMT_SIZE, AlignUp(sizeof(Class) + OBJECT_POINTER_SIZE, OBJECT_POINTER_SIZE));
    ets::EtsClass *etsCls = ets::EtsClass::FromRuntimeClass(coreCls);
    ets::EtsHandleScope scope(execCtx);
    ets::EtsHandle<ets::EtsClass> etsClsHandle(execCtx, etsCls);
    auto *etsClsPtr = etsClsHandle.GetPtr();
    TriggerGC();
    ASSERT_EQ(etsClsPtr, etsClsHandle.GetPtr());
}

TEST_F(CMCGCTest, RemSetCheck)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *vm = execCtx->GetPandaVM();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);

    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
    ets::EtsObjectArray *arr = ets::EtsObjectArray::Create(objCls, TEST_ARRAY_SIZE);
    ets::EtsHandleScope scope(execCtx);
    ets::EtsHandle<ets::EtsObjectArray> arrHandle(execCtx, arr);

    FillCurrentMemRegion(execCtx);
    TriggerGC();

    uintptr_t arrPtr = reinterpret_cast<uintptr_t>(arrHandle.GetPtr());
    cvm::RegionDesc *arrRegion = cvm::RegionDesc::GetRegionDescAt(arrPtr);
    ASSERT_TRUE(arrRegion->IsInOldSpace());

    ets::EtsObject *obj = ets::EtsObject::Create(objCls);
    uintptr_t objPtr = reinterpret_cast<uintptr_t>(obj);
    cvm::RegionDesc *objRegion = cvm::RegionDesc::GetRegionDescAt(objPtr);
    ASSERT_TRUE(objRegion->IsInYoungSpace());
    arrHandle.GetPtr()->Set(0, obj);

    auto *rset = arrRegion->GetRSet();
    uintptr_t rsetPtr = reinterpret_cast<uintptr_t>(rset);
    auto *cardTable =
        reinterpret_cast<cvm::RegionRSet::CardElement *>(rsetPtr + cvm::RegionRSet::CARD_TABLE_DATA_OFFSET);
    ASSERT_NE(0, cardTable[0]);

    int visited = 0;
    arrRegion->VisitRememberSetBeforeMarking([&](cvm::BaseObject *obj) { visited++; });
    ASSERT_NE(0, visited);
}

TEST_F(CMCGCTest, VregRoot)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *vm = execCtx->GetPandaVM();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread scope(coro);

    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
    ets::EtsObject *obj = ets::EtsObject::Create(objCls);
    ASSERT_NE(nullptr, obj);

    auto frame = interpreter::test::CreateFrame(1U, nullptr, nullptr);
    coro->SetCurrentFrame(frame.get());

    StaticFrameHandler handler(frame.get());
    handler.GetVReg(0).SetReference(reinterpret_cast<ObjectHeader *>(obj));
    ObjectHeader *objRef = handler.GetVReg(0).GetReference();

    auto langCtx = execCtx->GetPandaVM()->GetLanguageContext();
    auto mainCls = interpreter::test::CreateClass(langCtx.GetLanguage());
    std::vector<uint8_t> bytecode;
    BytecodeEmitter emitter;
    ASSERT_EQ(emitter.Build(&bytecode), BytecodeEmitter::ErrorCode::SUCCESS);
    auto mainMethodData = interpreter::test::CreateMethod(mainCls, ACC_STATIC, 1, 1, nullptr, bytecode);
    frame->SetMethod(mainMethodData.first.get());
    FillCurrentMemRegion(execCtx);
    TriggerGC();
    ASSERT_NE(nullptr, handler.GetVReg(0).GetReference());
    ASSERT_NE(objRef, handler.GetVReg(0).GetReference());
}

TEST_F(CMCGCTest, ExceptionRoot)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *vm = execCtx->GetPandaVM();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread scope(coro);

    auto *exCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
    ObjectHeader *ex = reinterpret_cast<ObjectHeader *>(ets::EtsObject::Create(exCls));
    ASSERT_NE(nullptr, ex);

    coro->SetException(ex);
    FillCurrentMemRegion(execCtx);
    TriggerGC();
    ASSERT_NE(ex, coro->GetException());
}

TEST_F(CMCGCTest, ClassRoot)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto langCtx = execCtx->GetPandaVM()->GetLanguageContext();
    auto *runtime = Runtime::GetCurrent();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread scope(coro);

    auto *linkerExt = runtime->GetClassLinker()->GetExtension(langCtx);
    auto *clsName = reinterpret_cast<const uint8_t *>("Foo");
    auto *cls = linkerExt->CreateClass(clsName, TEST_CLASS_VTABLE_SIZE, TEST_CLASS_IMT_SIZE,
                                       AlignUp(sizeof(Class) + OBJECT_POINTER_SIZE, OBJECT_POINTER_SIZE));
    cls->SetRefFieldsNum(1, true);
    cls->SetState(Class::State::INITIALIZED);
    Field field(cls, static_cast<panda_file::File::EntityId>(0), 0,
                panda_file::Type(panda_file::Type::TypeId::REFERENCE));
    cls->SetFields(Span<Field>(&field, 1), 1);
    field.SetOffset(cls->GetRefFieldsOffset<true>() + sizeof(ObjectHeader));

    auto *vm = execCtx->GetPandaVM();
    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
    ObjectHeader *obj = reinterpret_cast<ObjectHeader *>(ets::EtsObject::Create(objCls));
    ObjectAccessor::SetFieldObject(cls->GetManagedObject(), field, obj);

    FillCurrentMemRegion(execCtx);
    TriggerGC();
    ASSERT_NE(obj, ObjectAccessor::GetFieldObject(cls->GetManagedObject(), field));
}

TEST_F(CMCGCTest, GlobalObjectStorageRoot)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *vm = execCtx->GetPandaVM();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread scope(coro);

    auto *storage = vm->GetGlobalObjectStorage();
    ASSERT_NE(nullptr, storage);

    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
    auto *obj = reinterpret_cast<ObjectHeader *>(ets::EtsObject::Create(objCls));
    auto *objRef = storage->Add(obj, Reference::ObjectType::GLOBAL);

    FillCurrentMemRegion(execCtx);
    TriggerGC();
    ASSERT_NE(obj, storage->Get(objRef));
}

TEST_F(CMCGCTest, ObjectCopy)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);

    ets::EtsHandleScope scope(execCtx);
    ets::EtsHandle<ets::EtsIntArray> arrHandle(execCtx, ets::EtsIntArray::Create(INT_ARRAY_SIZE));
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        arrHandle->Set(i, INT_ARRAY[i]);
    }

    auto *arrPtr = arrHandle.GetPtr();

    FillCurrentMemRegion(execCtx);
    TriggerGC();

    ASSERT_NE(arrPtr, arrHandle.GetPtr());
    ASSERT_EQ(INT_ARRAY_SIZE, arrHandle->GetLength());
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        ASSERT_EQ(INT_ARRAY[i], arrHandle->Get(i));
    }
}

TEST_F(CMCGCTest, PreBarrierCheck)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);
    ets::EtsHandleScope scope(execCtx);

    auto *vm = execCtx->GetPandaVM();
    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);

    auto *arr = ets::EtsObjectArray::Create(objCls, TEST_ARRAY_SIZE);
    ets::EtsHandle<ets::EtsObjectArray> arrHandle(execCtx, arr);

    ets::EtsObject *obj1 = ets::EtsObject::Create(objCls);
    ets::EtsHandle<ets::EtsObject> obj1Handle(execCtx, obj1);
    arr->Set(0, obj1);

    ets::EtsObject *obj2 = ets::EtsObject::Create(objCls);
    ets::EtsHandle<ets::EtsObject> obj2Handle(execCtx, obj2);

    FillCurrentMemRegion(execCtx);
    TriggerGC();

    uintptr_t arrPtr = reinterpret_cast<uintptr_t>(arrHandle.GetPtr());
    auto *arrRegion = cvm::RegionDesc::GetRegionDescAt(arrPtr);
    ASSERT_TRUE(arrRegion->IsInOldSpace());

    uintptr_t obj1Ptr = reinterpret_cast<uintptr_t>(obj1Handle.GetPtr());
    auto obj1Region = cvm::RegionDesc::GetRegionDescAt(obj1Ptr);
    ASSERT_TRUE(obj1Region->IsInOldSpace());
    uintptr_t obj2Ptr = reinterpret_cast<uintptr_t>(obj2Handle.GetPtr());
    auto obj2Region = cvm::RegionDesc::GetRegionDescAt(obj2Ptr);
    ASSERT_TRUE(obj2Region->IsInOldSpace());

    CMCGCPreBarrierChecker checker;
    cvm::BaseRuntime::GetInstance()->AddGCListener(&checker);

    cvm::BaseRuntime::GetInstance()->RequestGC(cvm::GCReason::GC_REASON_USER, true, cvm::GCType::GC_TYPE_FULL);

    {
        cvm::ScopedEnterSaferegion safeRegion(true);
        checker.OnStartPhaseMark([&arrHandle, &obj2Handle] {
            auto *mutator = cvm::Mutator::GetMutator();
            mutator->LeaveSaferegion();
            arrHandle.GetPtr()->Set(0, obj2Handle.GetPtr());
            mutator->EnterSaferegion(true);
        });
    }

    std::vector<cvm::BaseObject *> objects;
    {
        cvm::ScopedEnterSaferegion safeRegion(true);
        checker.OnFinishPhaseMark([&objects, &coro] {
            const auto *node = static_cast<const cvm::SatbBuffer::TreapNode *>(coro->GetSatbBufferNode());
            const_cast<cvm::SatbBuffer::TreapNode *>(node)->GetObjects(objects);
        });
    }
    ASSERT_EQ(1, objects.size());
}

TEST_F(CMCGCTest, ReadBarrierCheck)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *coro = ets::EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);
    ets::EtsHandleScope scope(execCtx);

    auto *vm = execCtx->GetPandaVM();
    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);

    ets::EtsObject *obj = ets::EtsObject::Create(objCls);
    ets::EtsHandle<ets::EtsObject> objHandle(execCtx, obj);

    FillCurrentMemRegion(execCtx);

    CMCGCReadBarrierChecker checker;
    cvm::BaseRuntime::GetInstance()->AddGCListener(&checker);

    cvm::BaseRuntime::GetInstance()->RequestGC(cvm::GCReason::GC_REASON_USER, true, cvm::GCType::GC_TYPE_FULL);

    uintptr_t objPtr = 0;
    {
        cvm::ScopedEnterSaferegion safeRegion(true);
        checker.OnStartPhaseMark([&objHandle, &objPtr] { objPtr = reinterpret_cast<uintptr_t>(objHandle.GetPtr()); });
    }

    auto objRegion = cvm::RegionDesc::GetRegionDescAt(objPtr);
    ASSERT_TRUE(objRegion->IsInFromSpace());

    {
        cvm::ScopedEnterSaferegion safeRegion(true);
        checker.OnStartPhasePrecopy(
            [&objHandle, &objPtr] { objPtr = reinterpret_cast<uintptr_t>(objHandle.GetPtr()); });
    }

    objRegion = cvm::RegionDesc::GetRegionDescAt(objPtr);
    ASSERT_TRUE(objRegion->IsInToSpace());
}
}  // namespace ark::mem
