/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "runtime/handle_scope-inl.h"
#include <gtest/gtest.h>
#include "handle_scope.h"
#include "mem/vm_handle.h"
#include "runtime/include/runtime.h"
#include "assembly-parser.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/heap_verifier.h"
#include "gtest/gtest.h"

namespace ark::mem {

class HeapVerifierTest : public testing::Test {
public:
    explicit HeapVerifierTest(std::string gc_type = "g1-gc")
    {
        auto opts = CreateDefaultOptions(gc_type);
        [[maybe_unused]] bool res = Runtime::Create(opts);
        ASSERT(res);
        thread_ = Thread::GetCurrent();
        ASSERT(thread_ != nullptr);
    }

    ~HeapVerifierTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(HeapVerifierTest);
    NO_MOVE_SEMANTIC(HeapVerifierTest);

    static RuntimeOptions CreateDefaultOptions(std::string gc_type)
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType(gc_type);
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        return options;
    }

    template <class FSETA, class FVER>
    void TestHeapVerification(FSETA SetUpA, FVER Verify)
    {
        ScopedManagedCodeThread s(ManagedThread::GetCurrent());
        auto simpleAsm = R"(
            .record A{}
            .record B{
                A member0
            }
        )";
        pandasm::Parser p;
        auto res = p.Parse(simpleAsm);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        ASSERT_NE(classLinker, nullptr);
        classLinker->AddPandaFile(std::move(pf));

        PandaString descriptor;

        Class *class_a = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                             ->GetClass(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("A"), 0, &descriptor));
        Class *class_b = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                             ->GetClass(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("B"), 0, &descriptor));
        auto instance_b = ObjectHeader::Create(class_b);
        ASSERT_NE(instance_b, nullptr);
        ASSERT_NE(class_a, nullptr);

        // SetUpA can trigger GC
        auto thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope {thread};
        VMHandle<ObjectHeader> handleInstB(thread, instance_b);

        auto instance_a = SetUpA(class_a);
        instance_b = handleInstB.GetPtr();

        ASSERT_NE(instance_b, nullptr);

        auto field_member0 = class_b->GetInstanceFieldByName(utf::CStringAsMutf8("member0"));
        instance_b->SetFieldObject<false, false>(field_member0->GetOffset(), instance_a);

        auto fail_count = Verify(GetHeapManager());
        EXPECT_EQ(fail_count, EXPECTED_FAIL_COUNT_ONE);
    }

    static ObjectHeader *GetDeletedObj(Class *cls)
    {
        auto obj = ObjectHeader::Create(cls);

        {
            auto thread = ManagedThread::GetCurrent();
            [[maybe_unused]] HandleScope<ObjectHeader *> scope {thread};
            VMHandle<ObjectHeader> handleObj(thread, obj);
            ScopedNativeCodeThread snct(thread);
            GCTask task(GCTaskCause::OOM_CAUSE);
            task.Run(*thread->GetVM()->GetGC());
            obj->SetMark(handleObj->GetMark());  // Restore mark so it is not forwarded
        }
        return reinterpret_cast<ObjectHeader *>(obj);
    }

    static ObjectHeader *GetForwardedObject(Class *cls)
    {
        auto obj = ObjectHeader::Create(cls);
        auto mark = obj->GetMark().DecodeFromForwardingAddress(PANDA_32BITS_HEAP_START_ADDRESS);
        obj->SetMark(mark);
        return obj;
    }

    static auto OldVerifierCB(HeapManager *hm)
    {
        return HeapVerifier<PandaAssemblyLanguageConfig>(hm).VerifyHeap();
    }

    static auto FastVerifierCB(HeapManager *hm)
    {
        return FastHeapVerifier<PandaAssemblyLanguageConfig>(hm).VerifyAll();
    }

    HeapManager *GetHeapManager()
    {
        return thread_->GetVM()->GetHeapManager();
    }

    PandaUniquePtr<Method> MakeFakeMethod()
    {
        mem::InternalAllocatorPtr allocator = thread_->GetVM()->GetHeapManager()->GetInternalAllocator();

        auto runtime = Runtime::GetCurrent();
        auto etx = runtime->GetClassLinker()->GetExtension(
            runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        auto klass = etx->CreateClass(nullptr, 0, 0, AlignUp(sizeof(Class), OBJECT_POINTER_SIZE));

        PandaUniquePtr<Method> method(reinterpret_cast<Method *>(allocator->Alloc(sizeof(Method))));
        new (method.get()) Method(reinterpret_cast<Class *>(klass), nullptr, panda_file::File::EntityId(0),
                                  panda_file::File::EntityId(0), 0, 0, nullptr);
        return method;
    }

    static ObjectHeader *MakeInvalidObject(ObjectHeader *obj, size_t obj_size)
    {
        return ToNativePtr<ObjectHeader>(ToUintPtr(obj) + obj_size / 2U);
    }

    const size_t EXPECTED_FAIL_COUNT_ZERO = 0;
    const size_t EXPECTED_FAIL_COUNT_ONE = 1;

    Thread *thread_;
};

class HeapVerifierTestStw : public HeapVerifierTest {
public:
    HeapVerifierTestStw() : HeapVerifierTest("stw") {}
};

// we have many asserts in GC which check that objects are correct, we can meet this ASSERT in DEBUG before this test
// finishes, so it will not pass
#ifdef NDEBUG

TEST_F(HeapVerifierTest, OldHVDeletedObj)
{
    TestHeapVerification(GetDeletedObj, OldVerifierCB);
}

TEST_F(HeapVerifierTest, OldHVForwardedObj)
{
    TestHeapVerification(GetForwardedObject, OldVerifierCB);
}

TEST_F(HeapVerifierTest, FastHVDeletedObj)
{
    TestHeapVerification(GetDeletedObj, FastVerifierCB);
}

TEST_F(HeapVerifierTest, FastHVForwardedObj)
{
    TestHeapVerification(GetForwardedObject, FastVerifierCB);
}
#endif  // NDEBUG

TEST_F(HeapVerifierTest, VerifyRoot)
{
    ScopedManagedCodeThread s(ManagedThread::GetCurrent());
    HeapVerifier<PandaAssemblyLanguageConfig> verifier(GetHeapManager());

    constexpr size_t NUM_VREGS = 1;
    auto frameDeleter = [](Frame *frame) { FreeFrame(frame); };
    {
        PandaUniquePtr<Method> fake_method = MakeFakeMethod();
        auto frame = std::unique_ptr<Frame, decltype(frameDeleter)>(
            CreateFrameWithSize(NUM_VREGS * 2, NUM_VREGS, fake_method.get(), nullptr), frameDeleter);
        ASSERT(frame != nullptr);
        ManagedThread::GetCurrent()->SetCurrentFrame(frame.get());
        EXPECT_EQ(verifier.VerifyRoot(), EXPECTED_FAIL_COUNT_ZERO);
    }
}

TEST_F(HeapVerifierTestStw, IsValidHeapObjectAddress)
{
    ScopedManagedCodeThread s(ManagedThread::GetCurrent());
    auto verifier = HeapVerifier<PandaAssemblyLanguageConfig>(GetHeapManager());
    EXPECT_FALSE(verifier.IsValidObjectAddress(reinterpret_cast<void *>(1U)));
    EXPECT_FALSE(verifier.IsValidObjectAddress(reinterpret_cast<void *>(4U)));
    EXPECT_FALSE(verifier.IsValidObjectAddress(nullptr));
    EXPECT_FALSE(verifier.IsHeapAddress(&verifier));

    auto obj_allocator = Thread::GetCurrent()->GetVM()->GetGC()->GetObjectAllocator();
    auto fnVerify = [&verifier, &obj_allocator](const std::string &str) {
        coretypes::LineString *str_obj = coretypes::LineString::CreateFromMUtf8(
            reinterpret_cast<const uint8_t *>(&str[0]), str.length(),
            Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY),
            Runtime::GetCurrent()->GetPandaVM());
        ASSERT_TRUE(str_obj != nullptr);
        EXPECT_TRUE(verifier.IsValidObjectAddress(str_obj));
        EXPECT_FALSE(verifier.IsValidObjectAddress(ToVoidPtr(ToUintPtr(str_obj) + 1)));
        EXPECT_TRUE(obj_allocator->IsLive(str_obj));
    };

    fnVerify("a");
    fnVerify(std::string(obj_allocator->GetRegularObjectMaxSize(), 'a'));
    fnVerify(std::string(obj_allocator->GetLargeObjectMaxSize(), 'a'));
}

TEST_F(HeapVerifierTestStw, AllocatorStatusTest)
{
    ScopedManagedCodeThread s(ManagedThread::GetCurrent());
    [[maybe_unused]] auto fnVerify = [this](const char *source) {
        pandasm::Parser p;
        auto res = p.Parse(source);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());

        auto class_linker = Runtime::GetCurrent()->GetClassLinker();
        ASSERT_NE(class_linker, nullptr);

        class_linker->AddPandaFile(std::move(pf));

        PandaString descriptor;
        auto *ext = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
        Class *b_class = ext->GetClass(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("B"), 0, &descriptor));
        auto instance_b = ObjectHeader::Create(b_class);
        ASSERT_NE(instance_b, nullptr);
        auto obj_allocator = thread_->GetVM()->GetGC()->GetObjectAllocator();
        EXPECT_EQ(obj_allocator->VerifyAllocatorStatus(), EXPECTED_FAIL_COUNT_ZERO);

        Class *a_class = ext->GetClass(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("A"), 0, &descriptor));

        // set instance b's class word to A to generate a allocator state error.
        instance_b->SetClass(a_class);
        EXPECT_EQ(obj_allocator->VerifyAllocatorStatus(), EXPECTED_FAIL_COUNT_ONE);

        // recover
        instance_b->SetClass(b_class);
    };
    auto source = R"(
        .record A {}
        .record B {
            u64 member0
            u64 member1
            u64 member2
            u64 member3
        }
    )";

    fnVerify(source);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid test logic
TEST_F(HeapVerifierTestStw, AllocatorsVerificationTest)
{
    ScopedManagedCodeThread s(ManagedThread::GetCurrent());
    constexpr size_t REGULAR_OBJ_SIZE = RunSlots<>::MaxSlotSize() / 2;
    constexpr size_t LARGE_OBJ_SIZE = RunSlots<>::MaxSlotSize() * 2;
    constexpr size_t HUMONGOUS_OBJ_SIZE = FreeListAllocator<EmptyMemoryConfig>::GetMaxSize() * 2;
    constexpr size_t OBJECTS_COUNT = 10;
    constexpr size_t ALLOCATORS_COUNT = 3;
    std::array<ObjectHeader *, OBJECTS_COUNT * ALLOCATORS_COUNT> objects;
    auto *obj_allocator = static_cast<ObjectAllocatorNoGen<> *>(thread_->GetVM()->GetGC()->GetObjectAllocator());

    // allocate in RunSlotsAllocator
    for (size_t i = 0; i < OBJECTS_COUNT; ++i) {
        void *regular_mem =
            obj_allocator->Allocate(REGULAR_OBJ_SIZE, Alignment(std::log2(sizeof(Class))), ManagedThread::GetCurrent(),
                                    ObjectAllocatorBase::ObjMemInitPolicy::NO_INIT, false);
        ASSERT(regular_mem != nullptr);
        objects[i] = reinterpret_cast<ObjectHeader *>(regular_mem);
        ASSERT_TRUE(obj_allocator->ContainObject(objects[i]))
            << "RunSlotsAllocator must have object " << std::hex << objects[i];
        ASSERT_TRUE(obj_allocator->IsLive(objects[i]))
            << "Object " << std::hex << objects[i] << " must be live in RunSlotsAllocator";
        ObjectHeader *invalid_object = MakeInvalidObject(objects[i], REGULAR_OBJ_SIZE);
        ASSERT_FALSE(obj_allocator->IsLive(invalid_object))
            << "Object " << std::hex << invalid_object << " is invalid, incorrect verification";
    }

    // allocate in FreeListAllocator
    for (size_t i = OBJECTS_COUNT; i < OBJECTS_COUNT * 2U; ++i) {
        void *large_mem =
            obj_allocator->Allocate(LARGE_OBJ_SIZE, Alignment(std::log2(sizeof(Class))), ManagedThread::GetCurrent(),
                                    ObjectAllocatorBase::ObjMemInitPolicy::NO_INIT, false);
        ASSERT(large_mem != nullptr);
        objects[i] = reinterpret_cast<ObjectHeader *>(large_mem);
        ASSERT_TRUE(obj_allocator->ContainObject(objects[i]))
            << "FreeListAllocator must have object " << std::hex << objects[i];
        ASSERT_TRUE(obj_allocator->IsLive(objects[i]))
            << "Object " << std::hex << objects[i] << " must be live in FreeListAllocator";
        ObjectHeader *invalid_object = MakeInvalidObject(objects[i], LARGE_OBJ_SIZE);
        ASSERT_FALSE(obj_allocator->IsLive(invalid_object))
            << "Object " << std::hex << invalid_object << " is invalid, incorrect verification";
    }

    // allocate in HumongousObjAllocator
    for (size_t i = OBJECTS_COUNT * 2U; i < OBJECTS_COUNT * 3U; ++i) {
        void *humongous_mem =
            obj_allocator->Allocate(HUMONGOUS_OBJ_SIZE, Alignment(std::log2(sizeof(Class))),
                                    ManagedThread::GetCurrent(), ObjectAllocatorBase::ObjMemInitPolicy::NO_INIT, false);
        ASSERT(humongous_mem != nullptr);
        objects[i] = reinterpret_cast<ObjectHeader *>(humongous_mem);
        ASSERT_TRUE(obj_allocator->ContainObject(objects[i]))
            << "HumongousObjAllocator must have object " << std::hex << objects[i];
        ASSERT_TRUE(obj_allocator->IsLive(objects[i]))
            << "Object " << std::hex << objects[i] << " must be live in HumongousObjAllocator";
        ObjectHeader *invalid_object = MakeInvalidObject(objects[i], HUMONGOUS_OBJ_SIZE);
        ASSERT_FALSE(obj_allocator->IsLive(invalid_object))
            << "Object " << std::hex << invalid_object << " is invalid, incorrect verification";
    }

    // Checks for addresses outside object space
    ASSERT_FALSE(obj_allocator->IsLive(ToNativePtr<ObjectHeader>(PANDA_32BITS_HEAP_START_ADDRESS)))
        << "Address to start of object space can not be valid object";

    auto end_of_vm_memory = PoolManager::GetMmapMemPool()->GetMaxObjectAddress();
    ASSERT_FALSE(obj_allocator->IsLive(ToNativePtr<ObjectHeader>(end_of_vm_memory)))
        << "Address to end of vm memory can not be valid object";

    // Free() analog for objects
    obj_allocator->Collect(
        [&objects](ObjectHeader *obj) {
            auto it = std::find(objects.begin(), objects.end(), static_cast<void *>(obj));
            if (it != objects.end()) {
                return ObjectStatus::DEAD_OBJECT;
            }
            return ObjectStatus::ALIVE_OBJECT;
        },
        GCCollectMode::GC_ALL);

    // Check that deleted objects are dead
    for (size_t i = 0; i < OBJECTS_COUNT; ++i) {
        ASSERT_FALSE(obj_allocator->IsLive(objects[i]))
            << "Object " << std::hex << objects[i] << " was deleted and must be dead";
    }
}

}  // namespace ark::mem
