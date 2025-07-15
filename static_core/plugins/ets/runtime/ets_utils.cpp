/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

bool IsEtsGlobalClassName(const std::string &descriptor)
{
    ASSERT(descriptor.length() >= 2U);  // L...;
    ASSERT('L' == descriptor[0]);
    ASSERT(';' == descriptor[descriptor.size() - 1]);

    constexpr size_t ETSGLOBAL_SEMICOLON_LENGTH = sizeof(ETSGLOBAL_CLASS_NAME);

    const auto etsGlobalSubstringPos = descriptor.rfind(ETSGLOBAL_CLASS_NAME);
    if (etsGlobalSubstringPos == std::string::npos) {
        return false;
    }
    const bool etsGlobalClass = (1 == etsGlobalSubstringPos) && descriptor.length() == 1 + ETSGLOBAL_SEMICOLON_LENGTH;
    const bool endsWithETSGLOBAL = descriptor.length() - ETSGLOBAL_SEMICOLON_LENGTH == etsGlobalSubstringPos;
    const bool etsGlobalClassInPackage = endsWithETSGLOBAL && '/' == descriptor[etsGlobalSubstringPos - 1];

    return etsGlobalClass || etsGlobalClassInPackage;
}

void LambdaUtils::InvokeVoid(EtsCoroutine *coro, EtsObject *lambda)
{
    ASSERT(lambda != nullptr);
    EtsMethod *invoke = lambda->GetClass()->GetInstanceMethod(INVOKE_METHOD_NAME, nullptr);
    if (invoke == nullptr) {
        LOG(FATAL, RUNTIME) << "No method '$_invoke' found";
        return;
    }
    Value arg(lambda->GetCoreType());
    invoke->GetPandaMethod()->InvokeVoid(coro, &arg);
}

EtsString *ManglingUtils::GetDisplayNameStringFromField(EtsField *field)
{
    auto fieldNameData = field->GetCoreType()->GetName();
    auto fieldNameLength = fieldNameData.utf16Length;
    std::string_view fieldName(utf::Mutf8AsCString(fieldNameData.data), fieldNameLength);
    if (fieldName.rfind(PROPERTY, 0) == 0) {
        ASSERT(fieldNameLength >= PROPERTY_PREFIX_LENGTH);
        return EtsString::Resolve(
            fieldNameData.data + PROPERTY_PREFIX_LENGTH,  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            fieldNameLength - PROPERTY_PREFIX_LENGTH);
    }
    return EtsString::Resolve(fieldNameData.data, fieldNameData.utf16Length);
}

EtsField *ManglingUtils::GetFieldIDByDisplayName(EtsClass *klass, const PandaString &name, const char *sig)
{
    auto u8name = utf::CStringAsMutf8(name.c_str());
    auto *field = EtsField::FromRuntimeField(klass->GetRuntimeClass()->GetInstanceFieldByName(u8name));
    if (field == nullptr && !klass->GetRuntimeClass()->GetInterfaces().empty()) {
        auto mangledName = PandaString(PROPERTY) + name;
        u8name = utf::CStringAsMutf8(mangledName.c_str());
        field = EtsField::FromRuntimeField(klass->GetRuntimeClass()->GetInstanceFieldByName(u8name));
    }

    if (sig != nullptr && field != nullptr) {
        auto fieldTypeDescriptor = reinterpret_cast<const uint8_t *>(field->GetTypeDescriptor());
        if (utf::CompareMUtf8ToMUtf8(fieldTypeDescriptor, reinterpret_cast<const uint8_t *>(sig)) != 0) {
            return nullptr;
        }
    }

    return field;
}

static void ExtractClassDescriptorsFromArray(const panda_file::File *pfile, const panda_file::ArrayValue &classesArray,
                                             std::vector<std::string> &outDescriptors)
{
    const uint32_t valCount = classesArray.GetCount();
    outDescriptors.resize(valCount);
    for (uint32_t j = 0; j < valCount; j++) {
        auto entityId = classesArray.Get<panda_file::File::EntityId>(j);
        panda_file::ClassDataAccessor classData(*pfile, entityId);
        outDescriptors[j] = utf::Mutf8AsCString(classData.GetDescriptor());
    }
}

// NOLINT(clang-analyzer-core) // false positive: this is a function, not a global var
bool GetExportedClassDescriptorsFromModule(ark::ets::EtsClass *etsGlobalClass, std::vector<std::string> &outDescriptors)
{
    ASSERT(etsGlobalClass != nullptr);

    const auto *runtimeClass = etsGlobalClass->GetRuntimeClass();
    const panda_file::File *pfile = runtimeClass->GetPandaFile();
    panda_file::ClassDataAccessor cda(*pfile, runtimeClass->GetFileId());

    bool found = false;
    cda.EnumerateAnnotation(panda_file_items::class_descriptors::ANNOTATION_MODULE.data(),
                            [&outDescriptors, &found, pfile](panda_file::AnnotationDataAccessor &annotationAccessor) {
                                const uint32_t count = annotationAccessor.GetCount();
                                for (uint32_t i = 0; i < count; i++) {
                                    auto elem = annotationAccessor.GetElement(i);
                                    std::string nameStr =
                                        utf::Mutf8AsCString(pfile->GetStringData(elem.GetNameId()).data);
                                    if (nameStr == panda_file_items::class_descriptors::ANNOTATION_MODULE_EXPORTED) {
                                        auto classesArray = elem.GetArrayValue();
                                        ExtractClassDescriptorsFromArray(pfile, classesArray, outDescriptors);
                                        found = true;
                                        break;
                                    }
                                }
                                return true;
                            });

    return found;
}

}  // namespace ark::ets
