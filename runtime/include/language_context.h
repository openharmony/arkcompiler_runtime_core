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
#ifndef PANDA_RUNTIME_LANGUAGE_CONTEXT_H_
#define PANDA_RUNTIME_LANGUAGE_CONTEXT_H_

#include "libpandabase/utils/utf.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/file_items.h"
#include "macros.h"
#include "runtime/class_initializer.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/imtable_builder.h"
#include "runtime/include/itable_builder.h"
#include "runtime/include/vtable_builder.h"
#include "runtime/include/tooling/pt_lang_extension.h"
#include "runtime/mem/gc/gc_types.h"

namespace panda {
class Thread;
class Trace;
class Runtime;
class RuntimeOptions;

namespace mem {
class GC;
class ObjectAllocatorBase;
class GCSettings;
}  // namespace mem

struct VerificationInitAPI {
    std::vector<panda_file::Type::TypeId> primitive_roots_for_verification;
    std::vector<const uint8_t *> array_elements_for_verification;
    bool is_need_object_synthetic_class = false;
    bool is_need_string_synthetic_class = false;
    bool is_need_class_synthetic_class = false;
};

class LanguageContextBase {
public:
    LanguageContextBase() = default;

    DEFAULT_COPY_SEMANTIC(LanguageContextBase);
    DEFAULT_MOVE_SEMANTIC(LanguageContextBase);

    virtual ~LanguageContextBase() = default;

    virtual panda_file::SourceLang GetLanguage() const = 0;

    virtual const uint8_t *GetStringClassDescriptor() const
    {
        return utf::CStringAsMutf8(panda::panda_file::GetStringClassDescriptor(GetLanguage()));
    }

    virtual const uint8_t *GetObjectClassDescriptor() const = 0;

    virtual const uint8_t *GetClassClassDescriptor() const = 0;

    virtual const uint8_t *GetClassArrayClassDescriptor() const = 0;

    virtual const uint8_t *GetStringArrayClassDescriptor() const = 0;

    virtual const uint8_t *GetCtorName() const
    {
        return utf::CStringAsMutf8(panda::panda_file::GetCtorName(GetLanguage()));
    }

    virtual const uint8_t *GetCctorName() const
    {
        return utf::CStringAsMutf8(panda::panda_file::GetCctorName(GetLanguage()));
    }

    virtual const uint8_t *GetNullPointerExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetStackOverflowErrorClassDescriptor() const
    {
        UNREACHABLE();
        return nullptr;
    }

    virtual const uint8_t *GetArrayIndexOutOfBoundsExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetIndexOutOfBoundsExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetIllegalStateExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetNegativeArraySizeExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetStringIndexOutOfBoundsExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetArithmeticExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetClassCastExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetAbstractMethodErrorClassDescriptor() const = 0;

    virtual const uint8_t *GetArrayStoreExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetRuntimeExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetFileNotFoundExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetIOExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetIllegalArgumentExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetIllegalAccessExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetOutOfMemoryErrorClassDescriptor() const = 0;

    virtual const uint8_t *GetNoClassDefFoundErrorDescriptor() const = 0;

    virtual const uint8_t *GetClassCircularityErrorDescriptor() const = 0;

    virtual const uint8_t *GetNoSuchFieldErrorDescriptor() const = 0;

    virtual const uint8_t *GetNoSuchMethodErrorDescriptor() const = 0;

    virtual coretypes::TaggedValue GetInitialTaggedValue() const = 0;

    virtual DecodedTaggedValue GetInitialDecodedValue() const = 0;

    virtual DecodedTaggedValue GetDecodedTaggedValue(const coretypes::TaggedValue &value) const = 0;

    virtual coretypes::TaggedValue GetEncodedTaggedValue(int64_t value, int64_t tag) const = 0;

    virtual std::pair<Method *, uint32_t> GetCatchMethodAndOffset(Method *method, ManagedThread *thread) const;

    virtual PandaVM *CreateVM(Runtime *runtime, const RuntimeOptions &options) const = 0;

    virtual mem::GC *CreateGC(mem::GCType gc_type, mem::ObjectAllocatorBase *object_allocator,
                              const mem::GCSettings &settings) const = 0;

    virtual std::unique_ptr<ClassLinkerExtension> CreateClassLinkerExtension() const;

    virtual PandaUniquePtr<tooling::PtLangExt> CreatePtLangExt() const;

    virtual void ThrowException(ManagedThread *thread, const uint8_t *mutf8_name, const uint8_t *mutf8_msg) const;

    virtual void SetExceptionToVReg(
        [[maybe_unused]] interpreter::AccVRegister &vreg,  // NOLINTNEXTLINE(google-runtime-references)
        [[maybe_unused]] ObjectHeader *obj) const;

    virtual uint64_t GetTypeTag(interpreter::TypeTag tag) const
    {
        // return TypeTag default
        return tag;
    }

    virtual bool IsCallableObject([[maybe_unused]] ObjectHeader *obj) const = 0;

    virtual Method *GetCallTarget(ObjectHeader *obj) const = 0;

    virtual const uint8_t *GetExceptionInInitializerErrorDescriptor() const = 0;

    virtual const uint8_t *GetClassNotFoundExceptionDescriptor() const = 0;

    virtual const uint8_t *GetInstantiationErrorDescriptor() const = 0;

    virtual const uint8_t *GetUnsupportedOperationExceptionClassDescriptor() const = 0;

    virtual const uint8_t *GetVerifyErrorClassDescriptor() const = 0;

    virtual const uint8_t *GetReferenceErrorDescriptor() const = 0;

    virtual const uint8_t *GetTypedErrorDescriptor() const = 0;

    virtual const uint8_t *GetIllegalMonitorStateExceptionDescriptor() const = 0;

    virtual void ThrowStackOverflowException([[maybe_unused]] ManagedThread *thread) const
    {
        UNREACHABLE();
    }

    virtual const uint8_t *GetCloneNotSupportedExceptionDescriptor() const
    {
        UNREACHABLE();
    }

    virtual uint32_t GetFrameExtSize() const
    {
        return 0;
    }

    virtual const uint8_t *GetIncompatibleClassChangeErrorDescriptor() const
    {
        UNREACHABLE();
    }

    virtual PandaUniquePtr<IMTableBuilder> CreateIMTableBuilder() const
    {
        return MakePandaUnique<IMTableBuilder>();
    }

    virtual const uint8_t *GetErrorClassDescriptor() const
    {
        return nullptr;
    }

    virtual PandaUniquePtr<ITableBuilder> CreateITableBuilder() const
    {
        return nullptr;
    }

    virtual PandaUniquePtr<VTableBuilder> CreateVTableBuilder() const
    {
        return nullptr;
    }

    virtual bool InitializeClass([[maybe_unused]] ClassLinker *class_linker, [[maybe_unused]] ManagedThread *thread,
                                 [[maybe_unused]] Class *klass) const
    {
        return true;
    }

    virtual size_t GetStringSize(const ObjectHeader *string_object) const
    {
        return string_object->ObjectSize();
    }

    virtual Trace *CreateTrace([[maybe_unused]] PandaUniquePtr<panda::os::file::File> trace_file,
                               [[maybe_unused]] size_t buffer_size) const
    {
        UNREACHABLE();
    }

    virtual panda::panda_file::File::OpenMode GetBootPandaFilesOpenMode() const
    {
        return panda_file::File::READ_ONLY;
    }

    virtual VerificationInitAPI GetVerificationInitAPI() const
    {
        return VerificationInitAPI();
    }

    virtual const char *GetVerificationTypeClass() const
    {
        return nullptr;
    }

    virtual const char *GetVerificationTypeObject() const
    {
        return nullptr;
    }

    virtual const char *GetVerificationTypeThrowable() const
    {
        return nullptr;
    }

    virtual void DeoptimizeBegin([[maybe_unused]] Frame *iframe, [[maybe_unused]] int frame_depth) const {}

    virtual void DeoptimizeEnd() const {}

    virtual bool IsEnabledCHA() const
    {
        return true;
    }
};

class LanguageContext {
public:
    explicit LanguageContext(const LanguageContextBase *context) : base_(context) {}

    DEFAULT_COPY_SEMANTIC(LanguageContext);
    DEFAULT_MOVE_SEMANTIC(LanguageContext);

    ~LanguageContext() = default;

    panda_file::SourceLang GetLanguage() const
    {
        return base_->GetLanguage();
    }

    coretypes::TaggedValue GetInitialTaggedValue() const
    {
        return base_->GetInitialTaggedValue();
    }

    DecodedTaggedValue GetDecodedTaggedValue(const coretypes::TaggedValue &value) const
    {
        return base_->GetDecodedTaggedValue(value);
    }

    coretypes::TaggedValue GetEncodedTaggedValue(int64_t value, int64_t tag) const
    {
        return base_->GetEncodedTaggedValue(value, tag);
    }

    std::pair<Method *, uint32_t> GetCatchMethodAndOffset(Method *method, ManagedThread *thread) const
    {
        return base_->GetCatchMethodAndOffset(method, thread);
    }

    PandaVM *CreateVM(Runtime *runtime, const RuntimeOptions &options)
    {
        return base_->CreateVM(runtime, options);
    }

    mem::GC *CreateGC(mem::GCType gc_type, mem::ObjectAllocatorBase *object_allocator,
                      const mem::GCSettings &settings) const
    {
        return base_->CreateGC(gc_type, object_allocator, settings);
    }

    std::unique_ptr<ClassLinkerExtension> CreateClassLinkerExtension()
    {
        return base_->CreateClassLinkerExtension();
    }

    PandaUniquePtr<tooling::PtLangExt> CreatePtLangExt()
    {
        return base_->CreatePtLangExt();
    }

    void ThrowException(ManagedThread *thread, const uint8_t *mutf8_name, const uint8_t *mutf8_msg) const
    {
        base_->ThrowException(thread, mutf8_name, mutf8_msg);
    }

    void SetExceptionToVReg(
        [[maybe_unused]] interpreter::AccVRegister &vreg,  // NOLINTNEXTLINE(google-runtime-references)
        [[maybe_unused]] ObjectHeader *obj) const
    {
        base_->SetExceptionToVReg(vreg, obj);
    }

    DecodedTaggedValue GetInitialDecodedValue()
    {
        return base_->GetInitialDecodedValue();
    }

    const uint8_t *GetStringClassDescriptor() const
    {
        return base_->GetStringClassDescriptor();
    }

    const uint8_t *GetObjectClassDescriptor() const
    {
        return base_->GetObjectClassDescriptor();
    }

    const uint8_t *GetClassClassDescriptor() const
    {
        return base_->GetClassClassDescriptor();
    }

    const uint8_t *GetClassArrayClassDescriptor() const
    {
        return base_->GetClassArrayClassDescriptor();
    }

    const uint8_t *GetStringArrayClassDescriptor() const
    {
        return base_->GetStringArrayClassDescriptor();
    }

    const uint8_t *GetCtorName() const
    {
        return base_->GetCtorName();
    }

    const uint8_t *GetCctorName() const
    {
        return base_->GetCctorName();
    }

    const uint8_t *GetNullPointerExceptionClassDescriptor() const
    {
        return base_->GetNullPointerExceptionClassDescriptor();
    }

    const uint8_t *GetStackOverflowErrorClassDescriptor() const
    {
        return base_->GetStackOverflowErrorClassDescriptor();
    }

    void ThrowStackOverflowException(ManagedThread *thread) const
    {
        return base_->ThrowStackOverflowException(thread);
    }

    const uint8_t *GetArrayIndexOutOfBoundsExceptionClassDescriptor() const
    {
        return base_->GetArrayIndexOutOfBoundsExceptionClassDescriptor();
    }

    const uint8_t *GetIndexOutOfBoundsExceptionClassDescriptor() const
    {
        return base_->GetIndexOutOfBoundsExceptionClassDescriptor();
    }

    const uint8_t *GetIllegalStateExceptionClassDescriptor() const
    {
        return base_->GetIllegalStateExceptionClassDescriptor();
    }

    const uint8_t *GetNegativeArraySizeExceptionClassDescriptor() const
    {
        return base_->GetNegativeArraySizeExceptionClassDescriptor();
    }

    const uint8_t *GetStringIndexOutOfBoundsExceptionClassDescriptor() const
    {
        return base_->GetStringIndexOutOfBoundsExceptionClassDescriptor();
    }

    const uint8_t *GetArithmeticExceptionClassDescriptor() const
    {
        return base_->GetArithmeticExceptionClassDescriptor();
    }

    const uint8_t *GetClassCastExceptionClassDescriptor() const
    {
        return base_->GetClassCastExceptionClassDescriptor();
    }

    const uint8_t *GetAbstractMethodErrorClassDescriptor() const
    {
        return base_->GetAbstractMethodErrorClassDescriptor();
    }

    const uint8_t *GetArrayStoreExceptionClassDescriptor() const
    {
        return base_->GetArrayStoreExceptionClassDescriptor();
    }

    const uint8_t *GetRuntimeExceptionClassDescriptor() const
    {
        return base_->GetRuntimeExceptionClassDescriptor();
    }

    const uint8_t *GetFileNotFoundExceptionClassDescriptor() const
    {
        return base_->GetFileNotFoundExceptionClassDescriptor();
    }

    const uint8_t *GetIOExceptionClassDescriptor() const
    {
        return base_->GetIOExceptionClassDescriptor();
    }

    const uint8_t *GetIllegalArgumentExceptionClassDescriptor() const
    {
        return base_->GetIllegalArgumentExceptionClassDescriptor();
    }

    const uint8_t *GetIllegalAccessExceptionClassDescriptor() const
    {
        return base_->GetIllegalAccessExceptionClassDescriptor();
    }

    const uint8_t *GetOutOfMemoryErrorClassDescriptor() const
    {
        return base_->GetOutOfMemoryErrorClassDescriptor();
    }

    const uint8_t *GetNoClassDefFoundErrorDescriptor() const
    {
        return base_->GetNoClassDefFoundErrorDescriptor();
    }

    const uint8_t *GetClassCircularityErrorDescriptor() const
    {
        return base_->GetClassCircularityErrorDescriptor();
    }

    const uint8_t *GetNoSuchFieldErrorDescriptor() const
    {
        return base_->GetNoSuchFieldErrorDescriptor();
    }

    const uint8_t *GetNoSuchMethodErrorDescriptor() const
    {
        return base_->GetNoSuchMethodErrorDescriptor();
    }

    const uint8_t *GetExceptionInInitializerErrorDescriptor() const
    {
        return base_->GetExceptionInInitializerErrorDescriptor();
    }

    const uint8_t *GetClassNotFoundExceptionDescriptor() const
    {
        return base_->GetClassNotFoundExceptionDescriptor();
    }

    const uint8_t *GetInstantiationErrorDescriptor() const
    {
        return base_->GetInstantiationErrorDescriptor();
    }

    const uint8_t *GetUnsupportedOperationExceptionClassDescriptor() const
    {
        return base_->GetUnsupportedOperationExceptionClassDescriptor();
    }

    const uint8_t *GetVerifyErrorClassDescriptor() const
    {
        return base_->GetVerifyErrorClassDescriptor();
    }

    const uint8_t *GetIllegalMonitorStateExceptionDescriptor() const
    {
        return base_->GetIllegalMonitorStateExceptionDescriptor();
    }

    const uint8_t *GetCloneNotSupportedExceptionDescriptor() const
    {
        return base_->GetCloneNotSupportedExceptionDescriptor();
    }

    const uint8_t *GetErrorClassDescriptor() const
    {
        return base_->GetErrorClassDescriptor();
    }

    const uint8_t *GetIncompatibleClassChangeErrorDescriptor() const
    {
        return base_->GetIncompatibleClassChangeErrorDescriptor();
    }

    friend std::ostream &operator<<(std::ostream &stream, const LanguageContext &ctx)
    {
        return stream << panda::panda_file::LanguageToString(ctx.base_->GetLanguage());
    }

    uint64_t GetTypeTag(interpreter::TypeTag tag) const
    {
        // return TypeTag default
        return base_->GetTypeTag(tag);
    }

    bool IsCallableObject([[maybe_unused]] ObjectHeader *obj)
    {
        return base_->IsCallableObject(obj);
    }

    Method *GetCallTarget(ObjectHeader *obj) const
    {
        return base_->GetCallTarget(obj);
    }

    const uint8_t *GetReferenceErrorDescriptor() const
    {
        return base_->GetReferenceErrorDescriptor();
    }

    const uint8_t *GetTypedErrorDescriptor() const
    {
        return base_->GetTypedErrorDescriptor();
    }

    uint32_t GetFrameExtSize() const
    {
        return base_->GetFrameExtSize();
    }

    PandaUniquePtr<IMTableBuilder> CreateIMTableBuilder()
    {
        return base_->CreateIMTableBuilder();
    }

    PandaUniquePtr<ITableBuilder> CreateITableBuilder()
    {
        return base_->CreateITableBuilder();
    }

    PandaUniquePtr<VTableBuilder> CreateVTableBuilder()
    {
        return base_->CreateVTableBuilder();
    }

    bool InitializeClass(ClassLinker *class_linker, ManagedThread *thread, Class *klass) const
    {
        return base_->InitializeClass(class_linker, thread, klass);
    }

    Trace *CreateTrace(PandaUniquePtr<panda::os::file::File> trace_file, size_t buffer_size) const
    {
        return base_->CreateTrace(std::move(trace_file), buffer_size);
    }

    size_t GetStringSize(const ObjectHeader *string_object) const
    {
        return base_->GetStringSize(string_object);
    }

    VerificationInitAPI GetVerificationInitAPI() const
    {
        return base_->GetVerificationInitAPI();
    }

    const char *GetVerificationTypeClass() const
    {
        return base_->GetVerificationTypeClass();
    }

    const char *GetVerificationTypeObject() const
    {
        return base_->GetVerificationTypeObject();
    }

    const char *GetVerificationTypeThrowable() const
    {
        return base_->GetVerificationTypeThrowable();
    }

    panda::panda_file::File::OpenMode GetBootPandaFilesOpenMode() const
    {
        return base_->GetBootPandaFilesOpenMode();
    }

    virtual void DeoptimizeBegin(Frame *iframe, int frame_depth) const
    {
        return base_->DeoptimizeBegin(iframe, frame_depth);
    }

    virtual void DeoptimizeEnd() const
    {
        return base_->DeoptimizeEnd();
    }

    virtual bool IsEnabledCHA() const
    {
        return base_->IsEnabledCHA();
    }

private:
    const LanguageContextBase *base_;
};

}  // namespace panda

#endif  // PANDA_RUNTIME_LANGUAGE_CONTEXT_H_
