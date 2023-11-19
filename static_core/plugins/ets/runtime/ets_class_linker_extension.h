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

namespace panda {

class Class;
class Method;
class ObjectHeader;

}  // namespace panda

namespace panda::ets {

class EtsClassLinkerExtension : public ClassLinkerExtension {
public:
    EtsClassLinkerExtension() : ClassLinkerExtension(panda_file::SourceLang::ETS) {}

    ~EtsClassLinkerExtension() override;

    bool InitializeArrayClass(Class *array_class, Class *component_class) override;

    void InitializePrimitiveClass(Class *primitive_class) override;

    size_t GetClassVTableSize(ClassRoot root) override;

    size_t GetClassIMTSize(ClassRoot root) override;

    size_t GetClassSize(ClassRoot root) override;

    size_t GetArrayClassVTableSize() override;

    size_t GetArrayClassIMTSize() override;

    size_t GetArrayClassSize() override;

    Class *CreateClass(const uint8_t *descriptor, size_t vtable_size, size_t imt_size, size_t size) override;

    void FreeClass(Class *klass) override;

    bool InitializeClass([[maybe_unused]] Class *klass) override;

    const void *GetNativeEntryPointFor([[maybe_unused]] Method *method) const override;

    bool CanThrowException([[maybe_unused]] const Method *method) const override
    {
        return true;
    }

    ClassLinkerErrorHandler *GetErrorHandler() override
    {
        return &error_handler_;
    };

    Class *FromClassObject(panda::ObjectHeader *obj) override;
    size_t GetClassObjectSizeFromClassSize(uint32_t size) override;

    Class *GetObjectClass()
    {
        return object_class_;
    }

    Class *GetPromiseClass()
    {
        return promise_class_;
    }

    Class *GetArrayBufferClass()
    {
        return arraybuf_class_;
    }

    Class *GetSharedMemoryClass()
    {
        return shared_memory_class_;
    }

    Class *GetTypeAPIFieldClass()
    {
        return typeapi_field_class_;
    }

    Class *GetTypeAPIMethodClass()
    {
        return typeapi_method_class_;
    }

    Class *GetTypeAPIParameterClass()
    {
        return typeapi_parameter_class_;
    }

    Class *GetVoidClass()
    {
        return void_class_;
    }

    Class *GetBoxBooleanClass()
    {
        return box_boolean_class_;
    }

    Class *GetBoxByteClass()
    {
        return box_byte_class_;
    }

    Class *GetBoxCharClass()
    {
        return box_char_class_;
    }

    Class *GetBoxShortClass()
    {
        return box_short_class_;
    }

    Class *GetBoxIntClass()
    {
        return box_int_class_;
    }

    Class *GetBoxLongClass()
    {
        return box_long_class_;
    }

    Class *GetBoxFloatClass()
    {
        return box_float_class_;
    }

    Class *GetBoxDoubleClass()
    {
        return box_double_class_;
    }

    static EtsClassLinkerExtension *FromCoreType(ClassLinkerExtension *ext)
    {
        ASSERT(ext->GetLanguage() == panda_file::SourceLang::ETS);
        return static_cast<EtsClassLinkerExtension *>(ext);
    }

    LanguageContext GetLanguageContext() const
    {
        return lang_ctx_;
    }

    NO_COPY_SEMANTIC(EtsClassLinkerExtension);
    NO_MOVE_SEMANTIC(EtsClassLinkerExtension);

private:
    bool InitializeImpl(bool compressed_string_enabled) override;

    Class *InitializeClass(ObjectHeader *object_header, const uint8_t *descriptor, size_t vtable_size, size_t imt_size,
                           size_t size);

    Class *CreateClassRoot(const uint8_t *descriptor, ClassRoot root);

    bool CacheClass(Class **class_for_cache, const char *descriptor);

    class ErrorHandler : public ClassLinkerErrorHandler {
    public:
        void OnError(ClassLinker::Error error, const PandaString &message) override;
    };

    ErrorHandler error_handler_;
    LanguageContext lang_ctx_ {nullptr};
    mem::HeapManager *heap_manager_ {nullptr};

    // void class
    Class *void_class_ = nullptr;

    // Box classes
    Class *box_boolean_class_ = nullptr;
    Class *box_byte_class_ = nullptr;
    Class *box_char_class_ = nullptr;
    Class *box_short_class_ = nullptr;
    Class *box_int_class_ = nullptr;
    Class *box_long_class_ = nullptr;
    Class *box_float_class_ = nullptr;
    Class *box_double_class_ = nullptr;

    // Cached classes
    Class *object_class_ = nullptr;
    Class *promise_class_ = nullptr;
    Class *arraybuf_class_ = nullptr;

    // Cached type API classes
    Class *typeapi_field_class_ = nullptr;
    Class *typeapi_method_class_ = nullptr;
    Class *typeapi_parameter_class_ = nullptr;

    // Escompat classes
    Class *shared_memory_class_ = nullptr;
};

}  // namespace panda::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_EXTENSION_H_
