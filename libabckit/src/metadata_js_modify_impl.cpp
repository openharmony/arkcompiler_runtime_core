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
#include "libabckit/include/c/extensions/js/metadata_js.h"

#include "libabckit/src/macros.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"

namespace libabckit {

// ========================================
// File
// ========================================

extern "C" AbckitJsModule *FileAddExternalModule(AbckitFile *file,
                                                 const struct AbckitJsExternalModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    if (file->frontend != Mode::DYNAMIC) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_MODE);
        return nullptr;
    }

    return FileAddExternalJsModule(file, params);
}

// ========================================
// Module
// ========================================

extern "C" AbckitJsImportDescriptor *ModuleAddImportFromJsToJs(
    AbckitJsModule *importing /* assert(importing.target === AbckitTarget_ArkTS_v2) */,
    AbckitJsModule *imported /* assert(importing.target === AbckitTarget_ArkTS_v2) */,
    const struct AbckitJsImportFromDynamicModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(importing, nullptr);
    LIBABCKIT_BAD_ARGUMENT(imported, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    return ModuleAddImportFromDynamicModuleDynamic(importing->core, imported->core, params);
}

extern "C" void ModuleRemoveImportJs(AbckitJsModule *m, AbckitJsImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(i)

    return ModuleRemoveImportDynamic(m->core, i);
}

extern "C" AbckitJsExportDescriptor *ModuleAddExportFromJsToJs(AbckitJsModule *exporting, AbckitJsModule *exported,
                                                               const AbckitJsDynamicModuleExportCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(exporting, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    return DynamicModuleAddExportDynamic(exporting->core, exported != nullptr ? exported->core : nullptr, params);
}

extern "C" void ModuleRemoveExportJs(AbckitJsModule *m, AbckitJsExportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(i)

    return ModuleRemoveExportDynamic(m->core, i);
}

// ========================================
// Class
// ========================================

// ========================================
// AnnotationInterface
// ========================================

// ========================================
// Function
// ========================================

// ========================================
// Annotation
// ========================================

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

AbckitJsModifyApi g_jsModifyApiImpl = {

    // ========================================
    // File
    // ========================================

    FileAddExternalModule,

    // ========================================
    // Module
    // ========================================

    ModuleAddImportFromJsToJs, ModuleRemoveImportJs, ModuleAddExportFromJsToJs, ModuleRemoveExportJs,

    // ========================================
    // Class
    // ========================================

    // ========================================
    // AnnotationInterface
    // ========================================

    // ========================================
    // Function
    // ========================================

    // ========================================
    // Annotation
    // ========================================

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

extern "C" AbckitJsModifyApi const *AbckitGetJsModifyApiImpl(AbckitApiVersion version)
{
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_jsModifyApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
