/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_LANGUAGE_CONTEXT_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_LANGUAGE_CONTEXT_H_

#include <libarkfile/include/source_lang_enum.h>

#include "libarkbase/macros.h"
#include "libarkbase/utils/utf.h"
#include "runtime/class_initializer.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/itable_builder.h"
#include "runtime/include/language_context.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/method.h"
#include "runtime/include/vtable_builder_interface.h"
#include "runtime/interpreter/acc_vregister.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_types.h"
#include "runtime/mem/gc/gc_settings.h"
#include "runtime/include/mem/allocator.h"
#include "plugins/ets/runtime/tooling/pt_ets_extension.h"

#include "ets_class_linker_extension.h"
#include "ets_vm.h"

namespace ark::ets {

class EtsLanguageContext : public LanguageContextBase {
public:
    EtsLanguageContext() = default;

    DEFAULT_COPY_SEMANTIC(EtsLanguageContext);
    DEFAULT_MOVE_SEMANTIC(EtsLanguageContext);

    ~EtsLanguageContext() override = default;

    panda_file::SourceLang GetLanguage() const override
    {
        return panda_file::SourceLang::ETS;
    }

    LangTypeT GetLanguageType() const override
    {
        return EtsLanguageConfig::LANG_TYPE;
    }

    const uint8_t *GetObjectClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/Object;");
    }

    const uint8_t *GetAsyncAnnotationDescriptor() const override
    {
        return utf::CStringAsMutf8("Lets/coroutine/Async;");
    }

    const uint8_t *GetAsyncContextClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Larkruntime/AsyncContext;");
    }

    const uint8_t *GetPromiseClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/Promise;");
    }

    const uint8_t *GetClassClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/Class;");
    }

    const uint8_t *GetUniqueObjectClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/Null;");
    }

    const uint8_t *GetClassArrayClassDescriptor() const override
    {
        return utf::CStringAsMutf8("[Lstd/core/Class;");
    }

    const uint8_t *GetStringArrayClassDescriptor() const override
    {
        return utf::CStringAsMutf8("[Lstd/core/String;");
    }

    const uint8_t *GetCtorName() const override
    {
        return utf::CStringAsMutf8("<ctor>");
    }

    const uint8_t *GetCctorName() const override
    {
        return utf::CStringAsMutf8("<cctor>");
    }

    const uint8_t *GetNullPointerExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/NullPointerError;");
    }

    const uint8_t *GetStackOverflowErrorClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/StackOverflowError;");
    }

    const uint8_t *GetArrayIndexOutOfBoundsExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/ArrayIndexOutOfBoundsError;");
    }

    const uint8_t *GetIndexOutOfBoundsExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/IndexOutOfBoundsError;");
    }

    const uint8_t *GetIllegalStateExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/IllegalStateError;");
    }
    const uint8_t *GetNegativeArraySizeExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/NegativeArraySizeError;");
    }

    const uint8_t *GetStringIndexOutOfBoundsExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/StringIndexOutOfBoundsError;");
    }

    const uint8_t *GetRangeErrorExceptionClassDescriptor() const
    {
        return utf::CStringAsMutf8("Lstd/core/RangeError;");
    }

    const uint8_t *GetArithmeticExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/ArithmeticError;");
    }

    const uint8_t *GetClassCastExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/ClassCastError;");
    }

    const uint8_t *GetAbstractMethodErrorClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerAbstractMethodError;");
    }

    const uint8_t *GetArrayStoreExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/ArrayStoreError;");
    }

    const uint8_t *GetRuntimeExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/RuntimeError;");
    }

    const uint8_t *GetFileNotFoundExceptionClassDescriptor() const override
    {
        UNREACHABLE();
    }

    const uint8_t *GetIOExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/IOError;");
    }

    const uint8_t *GetIllegalArgumentExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/IllegalArgumentError;");
    }

    const uint8_t *GetIllegalAccessExceptionClassDescriptor() const override
    {
        UNREACHABLE();
    }

    const uint8_t *GetOutOfMemoryErrorClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/OutOfMemoryError;");
    }

    const uint8_t *GetNoClassDefFoundErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerUnresolvedClassError;");
    }

    const uint8_t *GetClassCircularityErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerTypeCircularityError;");
    }

    const uint8_t *GetCoroutinesLimitExceedErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/CoroutinesLimitExceedError;");
    }

    const uint8_t *GetNoSuchFieldErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerUnresolvedFieldError;");
    }

    const uint8_t *GetNoSuchMethodErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerUnresolvedMethodError;");
    }

    const uint8_t *GetExceptionInInitializerErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/ExceptionInInitializerError;");
    }

    const uint8_t *GetClassNotFoundExceptionDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerClassNotFoundError;");
    }

    const uint8_t *GetInstantiationErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/InstantiationError;");
    }

    const uint8_t *GetUnsupportedOperationExceptionClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/UnsupportedOperationError;");
    }

    const uint8_t *GetVerifyErrorClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerVerificationError;");
    }

    const uint8_t *GetErrorClassDescriptor() const override
    {
        return utf::CStringAsMutf8("Lescompat/Error;");
    }

    const uint8_t *GetIncompatibleClassChangeErrorDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/LinkerBadSupertypeError;");
    }

    coretypes::TaggedValue GetInitialTaggedValue() const override
    {
        UNREACHABLE();
        return coretypes::TaggedValue(coretypes::TaggedValue::VALUE_UNDEFINED);
    }

    coretypes::TaggedValue GetEncodedTaggedValue([[maybe_unused]] int64_t value,
                                                 [[maybe_unused]] int64_t tag) const override
    {
        UNREACHABLE();
        return coretypes::TaggedValue(coretypes::TaggedValue::VALUE_UNDEFINED);
    }

    void SetExceptionToVReg(interpreter::AccVRegister &vreg, ObjectHeader *obj) const override
    {
        vreg.AsVRegRef().SetReference(obj);
    }

    bool IsCallableObject([[maybe_unused]] ObjectHeader *obj) const override
    {
        UNREACHABLE();
        return false;
    }

    Method *GetCallTarget([[maybe_unused]] ObjectHeader *obj) const override
    {
        UNREACHABLE();
        return nullptr;
    }

    const uint8_t *GetReferenceErrorDescriptor() const override
    {
        UNREACHABLE();
        return nullptr;
    }

    const uint8_t *GetTypedErrorDescriptor() const override
    {
        UNREACHABLE();
        return nullptr;
    }

    const uint8_t *GetIllegalMonitorStateExceptionDescriptor() const override
    {
        return utf::CStringAsMutf8("Lstd/core/IllegalMonitorStateError;");
    }

    void ThrowException(ManagedThread *thread, const uint8_t *mutf8Name, const uint8_t *mutf8Msg) const override;

    PandaUniquePtr<ITableBuilder> CreateITableBuilder(ClassLinkerErrorHandler *errHandler) const override;

    PandaUniquePtr<VTableBuilder> CreateVTableBuilder(ClassLinkerErrorHandler *errHandler) const override;

    bool InitializeClass(ClassLinker *classLinker, ManagedThread *thread, Class *klass) const override
    {
        return ClassInitializer<MT_MODE_TASK>::Initialize(classLinker, thread, klass);
    }

    std::unique_ptr<ClassLinkerExtension> CreateClassLinkerExtension() const override
    {
        return std::make_unique<EtsClassLinkerExtension>();
    }

    ets::PandaEtsVM *CreateVM(Runtime *runtime, const RuntimeOptions &options) const override;

    mem::GC *CreateGC(mem::GCType gcType, mem::ObjectAllocatorBase *objectAllocator,
                      const mem::GCSettings &settings) const override;

    void ThrowStackOverflowException(ManagedThread *thread) const override;

    VerificationInitAPI GetVerificationInitAPI() const override;

    const char *GetVerificationTypeClass() const override
    {
        return "std.core.Class";
    }

    const char *GetVerificationTypeObject() const override
    {
        return "std.core.Object";
    }

    const char *GetVerificationTypeThrowable() const override
    {
        return "std.core.Object";
    }

    void WrapClassInitializerException([[maybe_unused]] ClassLinker *classLinker,
                                       [[maybe_unused]] ManagedThread *thread) const override
    {
    }

    std::unique_ptr<tooling::PtLangExt> CreatePtLangExt() const override
    {
        return std::make_unique<PtEtsExtension>();
    }

    bool HasValueEqualitySemantic() const override
    {
        return true;
    }
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_LANGUAGE_CONTEXT_H_
