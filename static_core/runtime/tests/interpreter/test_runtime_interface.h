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
#ifndef PANDA_RUNTIME_TESTS_INTERPRETER_TEST_RUNTIME_INTERFACE_H_
#define PANDA_RUNTIME_TESTS_INTERPRETER_TEST_RUNTIME_INTERFACE_H_

#include <gtest/gtest.h>

#include <cstdint>

#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/gc.h"

namespace panda::interpreter::test {

class DummyGC : public panda::mem::GC {
public:
    NO_COPY_SEMANTIC(DummyGC);
    NO_MOVE_SEMANTIC(DummyGC);

    explicit DummyGC(panda::mem::ObjectAllocatorBase *object_allocator, const panda::mem::GCSettings &settings);
    ~DummyGC() override = default;
    // NOLINTNEXTLINE(misc-unused-parameters)
    bool WaitForGC([[maybe_unused]] GCTask task) override
    {
        return false;
    }
    void InitGCBits([[maybe_unused]] panda::ObjectHeader *obj_header) override {}
    void InitGCBitsForAllocationInTLAB([[maybe_unused]] panda::ObjectHeader *obj_header) override {}
    bool Trigger([[maybe_unused]] PandaUniquePtr<GCTask> task) override
    {
        return false;
    }

    bool IsPinningSupported() const override
    {
        return true;
    }

    bool IsPostponeGCSupported() const override
    {
        return true;
    }

private:
    size_t VerifyHeap() override
    {
        return 0;
    }
    void InitializeImpl() override {}

    void PreRunPhasesImpl() override {}
    void RunPhasesImpl([[maybe_unused]] GCTask &task) override {}
    bool IsMarked([[maybe_unused]] const ObjectHeader *object) const override
    {
        return false;
    }
    void MarkObject([[maybe_unused]] ObjectHeader *object) override {}
    bool MarkObjectIfNotMarked([[maybe_unused]] ObjectHeader *object) override
    {
        return false;
    }
    void MarkReferences([[maybe_unused]] mem::GCMarkingStackType *references,
                        [[maybe_unused]] panda::mem::GCPhase gc_phase) override
    {
    }
    void VisitRoots([[maybe_unused]] const GCRootVisitor &gc_root_visitor,
                    [[maybe_unused]] mem::VisitGCRootFlags flags) override
    {
    }
    void VisitClassRoots([[maybe_unused]] const GCRootVisitor &gc_root_visitor) override {}
    void VisitCardTableRoots([[maybe_unused]] mem::CardTable *card_table,
                             [[maybe_unused]] const GCRootVisitor &gc_root_visitor,
                             [[maybe_unused]] const MemRangeChecker &range_checker,
                             [[maybe_unused]] const ObjectChecker &range_object_checker,
                             [[maybe_unused]] const ObjectChecker &from_object_checker,
                             [[maybe_unused]] const uint32_t processed_flag) override
    {
    }
    void CommonUpdateRefsToMovedObjects() override {}
    void UpdateRefsToMovedObjectsInPygoteSpace() override {}
    void UpdateVmRefs() override {}
    void UpdateGlobalObjectStorage() override {}
    void UpdateClassLinkerContextRoots() override {}
    void UpdateThreadLocals() override {}
    void ClearLocalInternalAllocatorPools() override {}
};

template <class T>
static T *ToPointer(size_t value)
{
    return reinterpret_cast<T *>(AlignUp(value, alignof(T)));
}

class RuntimeInterface {
public:
    static constexpr bool NEED_READ_BARRIER = false;
    static constexpr bool NEED_WRITE_BARRIER = false;

    using InvokeMethodHandler = std::function<Value(ManagedThread *, Method *, Value *)>;

    struct NullPointerExceptionData {
        bool expected {false};
    };

    struct ArithmeticException {
        bool expected {false};
    };

    struct ArrayIndexOutOfBoundsExceptionData {
        bool expected {false};
        coretypes::ArraySsizeT idx {};
        coretypes::ArraySizeT length {};
    };

    struct NegativeArraySizeExceptionData {
        bool expected {false};
        coretypes::ArraySsizeT size {};
    };

    struct ClassCastExceptionData {
        bool expected {false};
        Class *dst_type {};
        Class *src_type {};
    };

    struct AbstractMethodError {
        bool expected {false};
        Method *method {};
    };

    struct ArrayStoreExceptionData {
        bool expected {false};
        Class *array_class {};
        Class *elem_class {};
    };

    static constexpr BytecodeId METHOD_ID {0xaabb};
    static constexpr BytecodeId FIELD_ID {0xeeff};
    static constexpr BytecodeId TYPE_ID {0x5566};
    static constexpr BytecodeId LITERALARRAY_ID {0x7788};

    static coretypes::Array *ResolveLiteralArray([[maybe_unused]] PandaVM *vm, [[maybe_unused]] const Method &caller,
                                                 BytecodeId id)
    {
        EXPECT_EQ(id, LITERALARRAY_ID);
        // NOLINTNEXTLINE(readability-magic-numbers)
        return ToPointer<coretypes::Array>(0x7788);
    }

    static Method *ResolveMethod([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] const Method &caller,
                                 BytecodeId id)
    {
        EXPECT_EQ(id, METHOD_ID);
        return resolved_method_;
    }

    static Field *ResolveField([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] const Method &caller,
                               BytecodeId id)
    {
        EXPECT_EQ(id, FIELD_ID);
        return resolved_field_;
    }

    template <bool NEED_INIT>
    static Class *ResolveClass([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] const Method &caller,
                               BytecodeId id)
    {
        EXPECT_EQ(id, TYPE_ID);
        return resolved_class_;
    }

    static uint32_t FindCatchBlock([[maybe_unused]] const Method &method, [[maybe_unused]] ObjectHeader *exception,
                                   [[maybe_unused]] uint32_t pc)
    {
        return catch_block_pc_offset_;
    }

    static void SetCatchBlockPcOffset(uint32_t pc_offset)
    {
        catch_block_pc_offset_ = pc_offset;
    }

    static uint32_t GetCompilerHotnessThreshold()
    {
        return jit_threshold_;
    }

    static bool IsCompilerEnableJit()
    {
        return true;
    }

    static void SetCompilerHotnessThreshold(uint32_t threshold)
    {
        jit_threshold_ = threshold;
    }

    static void JITCompileMethod(Method *method)
    {
        method->SetCompiledEntryPoint(entry_point_);
    }

    static void SetCurrentFrame([[maybe_unused]] ManagedThread *thread, Frame *frame)
    {
        ASSERT_NE(frame, nullptr);
    }

    static RuntimeNotificationManager *GetNotificationManager()
    {
        return nullptr;
    }

    static void SetupResolvedMethod(Method *method)
    {
        ManagedThread::GetCurrent()->GetInterpreterCache()->Clear();
        resolved_method_ = method;
    }

    static void SetupResolvedField(Field *field)
    {
        ManagedThread::GetCurrent()->GetInterpreterCache()->Clear();
        resolved_field_ = field;
    }

    static void SetupResolvedClass(Class *klass)
    {
        ManagedThread::GetCurrent()->GetInterpreterCache()->Clear();
        resolved_class_ = klass;
    }

    static void SetupCatchBlockPcOffset(uint32_t pc_offset)
    {
        catch_block_pc_offset_ = pc_offset;
    }

    static void SetupNativeEntryPoint(const void *p)
    {
        entry_point_ = p;
    }

    static coretypes::Array *CreateArray(Class *klass, coretypes::ArraySizeT length)
    {
        EXPECT_EQ(klass, array_class_);
        EXPECT_EQ(length, array_length_);
        return array_object_;
    }

    static void SetupArrayClass(Class *klass)
    {
        array_class_ = klass;
    }

    static void SetupArrayLength(coretypes::ArraySizeT length)
    {
        array_length_ = length;
    }

    static void SetupArrayObject(coretypes::Array *obj)
    {
        array_object_ = obj;
    }

    static ObjectHeader *CreateObject(Class *klass)
    {
        EXPECT_EQ(klass, object_class_);
        return object_;
    }

    static void SetupObjectClass(Class *klass)
    {
        object_class_ = klass;
    }

    static void SetupObject(ObjectHeader *obj)
    {
        object_ = obj;
    }

    static Value InvokeMethod(ManagedThread *thread, Method *method, Value *args)
    {
        return invoke_handler_(thread, method, args);
    }

    static void SetupInvokeMethodHandler(const InvokeMethodHandler &handler)
    {
        invoke_handler_ = handler;
    }

    // Throw exceptions

    static void ThrowNullPointerException()
    {
        ASSERT_TRUE(npe_data_.expected);
    }

    static void ThrowArrayIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length)
    {
        ASSERT_TRUE(array_oob_exception_data_.expected);
        ASSERT_EQ(array_oob_exception_data_.idx, idx);
        ASSERT_EQ(array_oob_exception_data_.length, length);
    }

    static void ThrowNegativeArraySizeException(coretypes::ArraySsizeT size)
    {
        ASSERT_TRUE(array_neg_size_exception_data_.expected);
        ASSERT_EQ(array_neg_size_exception_data_.size, size);
    }

    static void ThrowArithmeticException()
    {
        ASSERT_TRUE(arithmetic_exception_data_.expected);
    }

    static void ThrowClassCastException(Class *dst_type, Class *src_type)
    {
        ASSERT_TRUE(class_cast_exception_data_.expected);
        ASSERT_EQ(class_cast_exception_data_.dst_type, dst_type);
        ASSERT_EQ(class_cast_exception_data_.src_type, src_type);
    }

    static void ThrowAbstractMethodError(Method *method)
    {
        ASSERT_TRUE(abstract_method_error_data_.expected);
        ASSERT_EQ(abstract_method_error_data_.method, method);
    }

    static void ThrowIncompatibleClassChangeErrorForMethodConflict([[maybe_unused]] Method *method) {}

    static void ThrowOutOfMemoryError([[maybe_unused]] const PandaString &msg) {}

    static void ThrowVerificationException([[maybe_unused]] const PandaString &msg)
    {
        // ASSERT_TRUE verification_of_method_exception_data.expected
        // ASSERT_EQ verification_of_method_exception_data.msg, msg
    }

    static void ThrowArrayStoreException(Class *array_klass, Class *elem_class)
    {
        ASSERT_TRUE(array_store_exception_data_.expected);
        ASSERT_EQ(array_store_exception_data_.array_class, array_klass);
        ASSERT_EQ(array_store_exception_data_.elem_class, elem_class);
    }

    static void SetArrayStoreException(ArrayStoreExceptionData data)
    {
        array_store_exception_data_ = data;
    }

    static void SetNullPointerExceptionData(NullPointerExceptionData data)
    {
        npe_data_ = data;
    }

    static void SetArrayIndexOutOfBoundsExceptionData(ArrayIndexOutOfBoundsExceptionData data)
    {
        array_oob_exception_data_ = data;
    }

    static void SetNegativeArraySizeExceptionData(NegativeArraySizeExceptionData data)
    {
        array_neg_size_exception_data_ = data;
    }

    static void SetArithmeticExceptionData(ArithmeticException data)
    {
        arithmetic_exception_data_ = data;
    }

    static void SetClassCastExceptionData(ClassCastExceptionData data)
    {
        class_cast_exception_data_ = data;
    }

    static void SetAbstractMethodErrorData(AbstractMethodError data)
    {
        abstract_method_error_data_ = data;
    }

    template <bool IS_DYNAMIC = false>
    static Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
    {
        uint32_t ext_sz = EMPTY_EXT_FRAME_DATA_SIZE;
        auto allocator = Thread::GetCurrent()->GetVM()->GetHeapManager()->GetInternalAllocator();
        void *mem =
            allocator->Allocate(panda::Frame::GetAllocSize(panda::Frame::GetActualSize<IS_DYNAMIC>(nregs), ext_sz),
                                GetLogAlignment(8), ManagedThread::GetCurrent());
        return new (Frame::FromExt(mem, ext_sz)) panda::Frame(mem, method, prev, nregs);
    }

    static Frame *CreateFrameWithActualArgsAndSize(uint32_t size, uint32_t nregs, uint32_t num_actual_args,
                                                   Method *method, Frame *prev)
    {
        uint32_t ext_sz = EMPTY_EXT_FRAME_DATA_SIZE;
        auto allocator = Thread::GetCurrent()->GetVM()->GetHeapManager()->GetInternalAllocator();
        void *mem = allocator->Allocate(panda::Frame::GetAllocSize(size, ext_sz), GetLogAlignment(8),
                                        ManagedThread::GetCurrent());
        if (UNLIKELY(mem == nullptr)) {
            return nullptr;
        }
        return new (Frame::FromExt(mem, ext_sz)) panda::Frame(mem, method, prev, nregs, num_actual_args);
    }

    static void FreeFrame(ManagedThread *thread, Frame *frame)
    {
        auto allocator = thread->GetVM()->GetHeapManager()->GetInternalAllocator();
        allocator->Free(frame->GetExt());
    }

    static mem::GC *GetGC()
    {
        return &panda::interpreter::test::RuntimeInterface::dummy_gc_;
    }

    static const uint8_t *GetMethodName([[maybe_unused]] Method *caller, [[maybe_unused]] BytecodeId method_id)
    {
        return nullptr;
    }

    static Class *GetMethodClass([[maybe_unused]] Method *caller, [[maybe_unused]] BytecodeId method_id)
    {
        return resolved_class_;
    }

    static uint32_t GetMethodArgumentsCount([[maybe_unused]] Method *caller, [[maybe_unused]] BytecodeId method_id)
    {
        return 0;
    }

    static void CollectRoots([[maybe_unused]] Frame *frame) {}

    static void Safepoint() {}

    static LanguageContext GetLanguageContext(const Method &method)
    {
        return Runtime::GetCurrent()->GetLanguageContext(*method.GetClass());
    }

private:
    static ArrayIndexOutOfBoundsExceptionData array_oob_exception_data_;

    static NegativeArraySizeExceptionData array_neg_size_exception_data_;

    static NullPointerExceptionData npe_data_;

    static ArithmeticException arithmetic_exception_data_;

    static ClassCastExceptionData class_cast_exception_data_;

    static AbstractMethodError abstract_method_error_data_;

    static ArrayStoreExceptionData array_store_exception_data_;

    static coretypes::Array *array_object_;

    static Class *array_class_;

    static coretypes::ArraySizeT array_length_;

    static ObjectHeader *object_;

    static Class *object_class_;

    static Class *resolved_class_;

    static uint32_t catch_block_pc_offset_;

    static Method *resolved_method_;

    static Field *resolved_field_;

    static InvokeMethodHandler invoke_handler_;

    static const void *entry_point_;

    static uint32_t jit_threshold_;

    static panda::interpreter::test::DummyGC dummy_gc_;
};

}  // namespace panda::interpreter::test

#endif  // PANDA_RUNTIME_TESTS_INTERPRETER_TEST_RUNTIME_INTERFACE_H_
