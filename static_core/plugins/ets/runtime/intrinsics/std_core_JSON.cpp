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

#include "intrinsics.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_reflect_field.h"
#include "helpers/json_helper.h"

namespace ark::ets::intrinsics {

static EtsBoolean IsFieldHasAnnotation(EtsReflectField *reflectField, char const *annotClassDescriptor)
{
    EtsField *field = reflectField->GetEtsField();
    bool result = false;
    auto *runtimeClass = field->GetDeclaringClass()->GetRuntimeClass();
    const panda_file::File &pf = *runtimeClass->GetPandaFile();
    panda_file::FieldDataAccessor fda(pf, field->GetRuntimeField()->GetFileId());
    fda.EnumerateAnnotations([&pf, &result, &annotClassDescriptor](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        if (utf::IsEqual(utf::CStringAsMutf8(annotClassDescriptor), pf.GetStringData(ada.GetClassId()).data)) {
            result = true;
        }
    });
    return static_cast<EtsBoolean>(result);
}

EtsBoolean StdCoreJSONGetJSONStringifyIgnore(EtsReflectField *reflectField)
{
    return IsFieldHasAnnotation(reflectField, EtsPlatformTypes::DESCRIPTOR_coreJSONStringifyIgnore);
}

EtsBoolean StdCoreJSONGetJSONParseIgnore(EtsReflectField *reflectField)
{
    return IsFieldHasAnnotation(reflectField, EtsPlatformTypes::DESCRIPTOR_coreJSONParseIgnore);
}

EtsString *StdCoreJSONGetJSONRename(EtsReflectField *reflectField)
{
    auto *thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    EtsField *field = reflectField->GetEtsField();
    VMHandle<EtsString> retStrHandle;
    auto *runtimeClass = field->GetDeclaringClass()->GetRuntimeClass();
    const panda_file::File &pf = *runtimeClass->GetPandaFile();
    panda_file::FieldDataAccessor fda(pf, field->GetRuntimeField()->GetFileId());
    fda.EnumerateAnnotations([&pf, &retStrHandle, &thread](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        if (utf::IsEqual(utf::CStringAsMutf8(EtsPlatformTypes::DESCRIPTOR_coreJSONRename),
                         pf.GetStringData(ada.GetClassId()).data)) {
            const auto value = ada.GetElement(0).GetScalarValue();
            const auto id = value.Get<panda_file::File::EntityId>();
            auto stringData = pf.GetStringData(id);
            retStrHandle = VMHandle<EtsString>(
                thread, EtsString::CreateFromMUtf8(reinterpret_cast<const char *>(stringData.data))->GetCoreType());
        }
    });
    return retStrHandle.GetPtr();
}

extern "C" EtsString *StdCoreJSONStringifyFast(EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(coro->HasPendingException() == false);

    EtsHandleScope scope(coro);
    EtsHandle<EtsObject> valueHandle(coro, value);

    helpers::JSONStringifier stringifier;
    return stringifier.Stringify(valueHandle);
}

}  // namespace ark::ets::intrinsics
