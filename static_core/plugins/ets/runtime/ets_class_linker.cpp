/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_annotation.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "types/ets_field.h"

namespace panda::ets {

EtsClassLinker::EtsClassLinker(ClassLinker *class_linker) : class_linker_(class_linker) {}

/*static*/
Expected<PandaUniquePtr<EtsClassLinker>, PandaString> EtsClassLinker::Create(ClassLinker *class_linker)
{
    PandaUniquePtr<EtsClassLinker> ets_class_linker = MakePandaUnique<EtsClassLinker>(class_linker);
    return Expected<PandaUniquePtr<EtsClassLinker>, PandaString>(std::move(ets_class_linker));
}

bool EtsClassLinker::Initialize()
{
    ClassLinkerExtension *ext = class_linker_->GetExtension(panda_file::SourceLang::ETS);
    ext_ = EtsClassLinkerExtension::FromCoreType(ext);
    return true;
}

bool EtsClassLinker::InitializeClass(EtsCoroutine *coroutine, EtsClass *klass)
{
    return class_linker_->InitializeClass(coroutine, klass->GetRuntimeClass());
}

EtsClass *EtsClassLinker::GetClassRoot(EtsClassRoot root) const
{
    return EtsClass::FromRuntimeClass(ext_->GetClassRoot(static_cast<panda::ClassRoot>(root)));
}

EtsClass *EtsClassLinker::GetClass(const char *name, bool need_copy_descriptor,
                                   ClassLinkerContext *class_linker_context, ClassLinkerErrorHandler *error_handler)
{
    const uint8_t *class_descriptor = utf::CStringAsMutf8(name);
    Class *cls = ext_->GetClass(class_descriptor, need_copy_descriptor, class_linker_context, error_handler);
    return LIKELY(cls != nullptr) ? EtsClass::FromRuntimeClass(cls) : nullptr;
}

EtsClass *EtsClassLinker::GetClass(const panda_file::File &pf, panda_file::File::EntityId id,
                                   ClassLinkerContext *class_linker_context, ClassLinkerErrorHandler *error_handler)
{
    Class *cls = ext_->GetClass(pf, id, class_linker_context, error_handler);
    return LIKELY(cls != nullptr) ? EtsClass::FromRuntimeClass(cls) : nullptr;
}

Method *EtsClassLinker::GetMethod(const panda_file::File &pf, panda_file::File::EntityId id)
{
    return class_linker_->GetMethod(pf, id);
}

Method *EtsClassLinker::GetAsyncImplMethod(Method *method, EtsCoroutine *coroutine)
{
    panda_file::File::EntityId async_ann_id = EtsAnnotation::FindAsyncAnnotation(method);
    ASSERT(async_ann_id.IsValid());
    const panda_file::File &pf = *method->GetPandaFile();
    panda_file::AnnotationDataAccessor ada(pf, async_ann_id);
    auto impl_method_id = ada.GetElement(0).GetScalarValue().Get<panda_file::File::EntityId>();
    Method *result = GetMethod(pf, impl_method_id);
    if (result == nullptr) {
        panda_file::MethodDataAccessor mda(pf, impl_method_id);
        PandaStringStream out;
        out << "Cannot resolve async method " << mda.GetFullName();
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::NO_SUCH_METHOD_ERROR, out.str());
    }
    return result;
}

EtsClass *EtsClassLinker::GetObjectClass()
{
    return EtsClass::FromRuntimeClass(ext_->GetObjectClass());
}

EtsClass *EtsClassLinker::GetPromiseClass()
{
    return EtsClass::FromRuntimeClass(ext_->GetPromiseClass());
}

EtsClass *EtsClassLinker::GetArrayBufferClass()
{
    return EtsClass::FromRuntimeClass(ext_->GetArrayBufferClass());
}

EtsClass *EtsClassLinker::GetSharedMemoryClass()
{
    return EtsClass::FromRuntimeClass(ext_->GetSharedMemoryClass());
}

EtsClass *EtsClassLinker::GetTypeAPIFieldClass()
{
    return EtsClass::FromRuntimeClass(ext_->GetTypeAPIFieldClass());
}

EtsClass *EtsClassLinker::GetTypeAPIMethodClass()
{
    return EtsClass::FromRuntimeClass(ext_->GetTypeAPIMethodClass());
}

EtsClass *EtsClassLinker::GetTypeAPIParameterClass()
{
    return EtsClass::FromRuntimeClass(ext_->GetTypeAPIParameterClass());
}

}  // namespace panda::ets
