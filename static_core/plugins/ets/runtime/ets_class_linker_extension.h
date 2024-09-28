/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_EXTENSION_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_EXTENSION_H_

#include <cstddef>

#include <libpandafile/include/source_lang_enum.h>

#include "libpandabase/macros.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/class_root.h"
#include "runtime/include/mem/panda_string.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark {

class Class;
class Method;
class ObjectHeader;

}  // namespace ark

namespace ark::ets {

class EtsClassLinkerExtension : public ClassLinkerExtension {
public:
    EtsClassLinkerExtension() : ClassLinkerExtension(panda_file::SourceLang::ETS) {}

    ~EtsClassLinkerExtension() override;

    bool InitializeArrayClass(Class *arrayClass, Class *componentClass) override;

    void InitializePrimitiveClass(Class *primitiveClass) override;

    size_t GetClassVTableSize(ClassRoot root) override;

    size_t GetClassIMTSize(ClassRoot root) override;

    size_t GetClassSize(ClassRoot root) override;

    size_t GetArrayClassVTableSize() override;

    size_t GetArrayClassIMTSize() override;

    size_t GetArrayClassSize() override;

    Class *CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size) override;

    void FreeClass(Class *klass) override;

    void InitializeClassRoot();

    bool InitializeClass(Class *klass) override;

    const void *GetNativeEntryPointFor(Method *method) const override;

    bool CanThrowException([[maybe_unused]] const Method *method) const override
    {
        return true;
    }

    ClassLinkerErrorHandler *GetErrorHandler() override
    {
        return &errorHandler_;
    };

    Class *FromClassObject(ark::ObjectHeader *obj) override;
    size_t GetClassObjectSizeFromClassSize(uint32_t size) override;

    void InitializeBuiltinClasses();

    Class *GetObjectClass()
    {
        return GetClassRoot(ClassRoot::OBJECT);
    }

    Class *GetUndefinedClass() const
    {
        return undefinedClass_;
    }

    Class *GetPromiseClass()
    {
        return promiseClass_;
    }

    Class *GetPromiseRefClass()
    {
        return promiseRefClass_;
    }

    Class *GetWaitersListClass()
    {
        return waiterListClass_;
    }

    Class *GetMutexClass()
    {
        return mutexClass_;
    }

    Class *GetEventClass()
    {
        return eventClass_;
    }

    Class *GetCondVarClass()
    {
        return condVarClass_;
    }

    Class *GetBigIntClass()
    {
        return bigintClass_;
    }

    Class *GetExceptionClass()
    {
        return exceptionClass_;
    }

    Class *GetErrorClass()
    {
        return errorClass_;
    }

    Class *GetArrayAsListIntClass()
    {
        return arrayAsListIntClass_;
    }

    Class *GetArrayClass()
    {
        return arrayClass_;
    }

    Class *GetArrayBufferClass()
    {
        return arraybufClass_;
    }

    Class *GetStringBuilderClass()
    {
        return stringBuilderClass_;
    }

    Class *GetSharedMemoryClass()
    {
        return sharedMemoryClass_;
    }

    Class *GetTypeAPIFieldClass()
    {
        return typeapiFieldClass_;
    }

    Class *GetTypeAPIMethodClass()
    {
        return typeapiMethodClass_;
    }

    Class *GetTypeAPIParameterClass()
    {
        return typeapiParameterClass_;
    }

    Class *GetIFunctionClass()
    {
        return ifuncClass_;
    }

    Class *GetBoxBooleanClass()
    {
        return boxBooleanClass_;
    }

    Class *GetBoxByteClass()
    {
        return boxByteClass_;
    }

    Class *GetBoxCharClass()
    {
        return boxCharClass_;
    }

    Class *GetBoxShortClass()
    {
        return boxShortClass_;
    }

    Class *GetBoxIntClass()
    {
        return boxIntClass_;
    }

    Class *GetBoxLongClass()
    {
        return boxLongClass_;
    }

    Class *GetBoxFloatClass()
    {
        return boxFloatClass_;
    }

    Class *GetBoxDoubleClass()
    {
        return boxDoubleClass_;
    }

    Class *GetFinalizableWeakRefClass()
    {
        return finalizableWeakClass_;
    }

    Method *GetSubscribeOnAnotherPromiseMethod()
    {
        return subscribeOnAnotherPromiseMethod_;
    }

    static EtsClassLinkerExtension *FromCoreType(ClassLinkerExtension *ext)
    {
        ASSERT(ext->GetLanguage() == panda_file::SourceLang::ETS);
        return static_cast<EtsClassLinkerExtension *>(ext);
    }

    LanguageContext GetLanguageContext() const
    {
        return langCtx_;
    }

    bool IsJSValueClass(Class *maybeJSValue) const
    {
        return maybeJSValue == jsvalueClass_;
    }

    NO_COPY_SEMANTIC(EtsClassLinkerExtension);
    NO_MOVE_SEMANTIC(EtsClassLinkerExtension);

private:
    bool InitializeImpl(bool compressedStringEnabled) override;

    Class *InitializeClass(ObjectHeader *objectHeader, const uint8_t *descriptor, size_t vtableSize, size_t imtSize,
                           size_t size);

    Class *CreateClassRoot(const uint8_t *descriptor, ClassRoot root);

    Class *CacheClass(std::string_view descriptor, bool forceInit = false);
    template <typename F>
    Class *CacheClass(std::string_view descriptor, F const &setup, bool forceInit = false);

    class ErrorHandler : public ClassLinkerErrorHandler {
    public:
        void OnError(ClassLinker::Error error, const PandaString &message) override;
    };

    ErrorHandler errorHandler_;
    LanguageContext langCtx_ {nullptr};
    mem::HeapManager *heapManager_ {nullptr};

    // Cached classes
    Class *undefinedClass_ = nullptr;
    // std.core box classes
    Class *boxBooleanClass_ = nullptr;
    Class *boxByteClass_ = nullptr;
    Class *boxCharClass_ = nullptr;
    Class *boxShortClass_ = nullptr;
    Class *boxIntClass_ = nullptr;
    Class *boxLongClass_ = nullptr;
    Class *boxFloatClass_ = nullptr;
    Class *boxDoubleClass_ = nullptr;
    // std.core
    Class *bigintClass_ = nullptr;
    Class *exceptionClass_ = nullptr;
    Class *errorClass_ = nullptr;
    Class *promiseClass_ = nullptr;
    Method *subscribeOnAnotherPromiseMethod_ = nullptr;
    Class *promiseRefClass_ = nullptr;
    Class *waiterListClass_ = nullptr;
    Class *mutexClass_ = nullptr;
    Class *eventClass_ = nullptr;
    Class *condVarClass_ = nullptr;
    Class *arrayClass_ = nullptr;
    Class *arraybufClass_ = nullptr;
    Class *stringBuilderClass_ = nullptr;
    Class *arrayAsListIntClass_ = nullptr;
    Class *jsvalueClass_ = nullptr;
    Class *finalizableWeakClass_ = nullptr;
    // Cached type API classes
    Class *typeapiFieldClass_ = nullptr;
    Class *typeapiMethodClass_ = nullptr;
    Class *typeapiParameterClass_ = nullptr;
    Class *ifuncClass_ = nullptr;
    // escompat
    Class *sharedMemoryClass_ = nullptr;
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_EXTENSION_H_
