/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */

#include <cassert>
#include <cstdint>
#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"

#include "libabckit/src/macros.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"

namespace libabckit {

// ========================================
// File
// ========================================

extern "C" AbckitArktsModule *FileAddExternalModuleArktsV1(AbckitFile *file,
                                                           const struct AbckitArktsV1ExternalModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    switch (file->frontend) {
        case Mode::DYNAMIC:
            return FileAddExternalArkTsV1Module(file, params);
        case Mode::STATIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Module
// ========================================

extern "C" AbckitArktsImportDescriptor *ModuleAddImportFromArktsV1ToArktsV1(
    AbckitArktsModule *importing, AbckitArktsModule *imported,
    const AbckitArktsImportFromDynamicModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(importing, nullptr);
    LIBABCKIT_BAD_ARGUMENT(imported, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    if (importing->core->target != imported->core->target || importing->core->target != ABCKIT_TARGET_ARK_TS_V1) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    return ModuleAddImportFromDynamicModuleDynamic(importing->core, imported->core, params);
}

extern "C" AbckitArktsImportDescriptor *ModuleAddImportFromArktsV2ToArktsV2(
    AbckitArktsModule * /*importing*/, AbckitArktsModule * /*imported*/,
    const AbckitArktsImportFromDynamicModuleCreateParams * /*params*/)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

extern "C" void ModuleRemoveImport(AbckitArktsModule *m, AbckitArktsImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(i)

    switch (m->core->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleRemoveImportDynamic(m->core, i);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsExportDescriptor *ModuleAddExportFromArktsV1ToArktsV1(
    AbckitArktsModule *exporting, AbckitArktsModule *exported, const AbckitArktsDynamicModuleExportCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(exporting, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    if (exporting->core->target != ABCKIT_TARGET_ARK_TS_V1 ||
        (exported != nullptr && exported->core->target != ABCKIT_TARGET_ARK_TS_V1)) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    return DynamicModuleAddExportDynamic(exporting->core, exported != nullptr ? exported->core : nullptr, params);
}

extern "C" AbckitArktsExportDescriptor *ModuleAddExportFromArktsV2ToArktsV2(
    AbckitArktsModule * /*exporting*/ /* assert(exporting.target === AbckitTarget_ArkTS_v2) */,
    AbckitArktsModule * /*exported*/ /* assert(exporting.target === AbckitTarget_ArkTS_v2) */,
    const AbckitArktsDynamicModuleExportCreateParams * /*params*/)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

extern "C" void ModuleRemoveExport(AbckitArktsModule *m, AbckitArktsExportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(i)

    switch (m->core->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleRemoveExportDynamic(m->core, i);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsAnnotationInterface *ModuleAddAnnotationInterface(
    AbckitArktsModule *m, const AbckitArktsAnnotationInterfaceCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    switch (m->core->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleAddAnnotationInterfaceDynamic(m->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Class
// ========================================

extern "C" AbckitArktsAnnotation *ClassAddAnnotation(AbckitArktsClass *klass,
                                                     const AbckitArktsAnnotationCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    switch (klass->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ClassAddAnnotationDynamic(klass->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void ClassRemoveAnnotation(AbckitArktsClass *klass, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(klass);
    LIBABCKIT_BAD_ARGUMENT_VOID(anno);

    switch (klass->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ClassRemoveAnnotationDynamic(klass->core, anno->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// AnnotationInterface
// ========================================

extern "C" AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddField(
    AbckitArktsAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(ai, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    switch (ai->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationInterfaceAddFieldDynamic(ai->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void AnnotationInterfaceRemoveField(AbckitArktsAnnotationInterface *ai,
                                               AbckitArktsAnnotationInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(ai);
    LIBABCKIT_BAD_ARGUMENT_VOID(field);

    switch (ai->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationInterfaceRemoveFieldDynamic(ai->core, field->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Function
// ========================================

extern "C" AbckitArktsAnnotation *FunctionAddAnnotation(AbckitArktsFunction *function,
                                                        const AbckitArktsAnnotationCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    switch (function->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionAddAnnotationDynamic(function->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void FunctionRemoveAnnotation(AbckitArktsFunction *function, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(function);
    LIBABCKIT_BAD_ARGUMENT_VOID(anno);

    switch (function->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionRemoveAnnotationDynamic(function->core, anno->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Annotation
// ========================================

extern "C" AbckitArktsAnnotationElement *AnnotationAddAnnotationElement(
    AbckitArktsAnnotation *anno, AbckitArktsAnnotationElementCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    LIBABCKIT_INTERNAL_ERROR(anno->core->ai, nullptr);

    AbckitCoreModule *module = anno->core->ai->owningModule;

    LIBABCKIT_INTERNAL_ERROR(module, nullptr);
    LIBABCKIT_INTERNAL_ERROR(module->file, nullptr);

    switch (module->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationAddAnnotationElementDynamic(anno->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void AnnotationRemoveAnnotationElement(AbckitArktsAnnotation *anno, AbckitArktsAnnotationElement *elem)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(anno);
    LIBABCKIT_BAD_ARGUMENT_VOID(elem);

    LIBABCKIT_INTERNAL_ERROR_VOID(anno->core->ai);

    AbckitCoreModule *module = anno->core->ai->owningModule;

    LIBABCKIT_INTERNAL_ERROR_VOID(module);
    LIBABCKIT_INTERNAL_ERROR_VOID(module->file);

    switch (module->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationRemoveAnnotationElementDynamic(anno->core, elem->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Type
// ========================================

// ========================================
// Value
// ========================================

// ========================================
// String
// ========================================

// ========================================
// LiteralArray
// ========================================

AbckitArktsModifyApi g_arktsModifyApiImpl = {

    // ========================================
    // File
    // ========================================

    FileAddExternalModuleArktsV1,

    // ========================================
    // Module
    // ========================================

    ModuleAddImportFromArktsV1ToArktsV1, ModuleRemoveImport, ModuleAddExportFromArktsV1ToArktsV1, ModuleRemoveExport,
    ModuleAddAnnotationInterface,

    // ========================================
    // Class
    // ========================================

    ClassAddAnnotation, ClassRemoveAnnotation,

    // ========================================
    // AnnotationInterface
    // ========================================

    AnnotationInterfaceAddField, AnnotationInterfaceRemoveField,

    // ========================================
    // Function
    // ========================================

    FunctionAddAnnotation, FunctionRemoveAnnotation,

    // ========================================
    // Annotation
    // ========================================

    AnnotationAddAnnotationElement, AnnotationRemoveAnnotationElement,

    // ========================================
    // Type
    // ========================================

    // ========================================
    // Value
    // ========================================

    // ========================================
    // String
    // ========================================

    // ========================================
    // LiteralArray
    // ========================================

    // ========================================
    // LiteralArray
    // ========================================
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitArktsModifyApi const *AbckitGetArktsModifyApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockArktsModifyApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_arktsModifyApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
