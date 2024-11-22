/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../../include/c/extensions/arkts/metadata_arkts.h"

#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock {

// NOLINTBEGIN(readability-identifier-naming)

inline AbckitArktsModule *FileAddExternalModuleArktsV1(AbckitFile *file,
                                                       const struct AbckitArktsV1ExternalModuleCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_V1_EXTERNAL_MODULE_CREATE_PARAMS);
    return DEFAULT_MODULE;
}

inline AbckitArktsImportDescriptor *ModuleAddImportFromArktsV1ToArktsV1(
    AbckitArktsModule *importing, AbckitArktsModule *imported,
    const struct AbckitArktsImportFromDynamicModuleCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(importing == DEFAULT_MODULE);
    EXPECT_TRUE(imported == DEFAULT_MODULE);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_IMPORT_FROM_DYNAMIC_CREATE_PARAMS);
    return DEFAULT_IMPORT_DESCRIPTOR;
}

inline void ModuleRemoveImport(AbckitArktsModule *m, AbckitArktsImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_MODULE);
    EXPECT_TRUE(id == DEFAULT_IMPORT_DESCRIPTOR);
}

inline AbckitArktsExportDescriptor *ModuleAddExportFromArktsV1ToArktsV1(
    AbckitArktsModule *exporting, AbckitArktsModule *exported,
    const struct AbckitArktsDynamicModuleExportCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(exporting == DEFAULT_MODULE);
    EXPECT_TRUE(exported == DEFAULT_MODULE);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_DYNAMIC_MODULE_EXPORT_CREATE_PARAMS);
    return DEFAULT_EXPORT_DESCRIPTOR;
}

inline void ModuleRemoveExport(AbckitArktsModule *m, AbckitArktsExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_MODULE);
    EXPECT_TRUE(ed == DEFAULT_EXPORT_DESCRIPTOR);
}

inline AbckitArktsAnnotationInterface *ModuleAddAnnotationInterface(
    AbckitArktsModule *m, const struct AbckitArktsAnnotationInterfaceCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_MODULE);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_ANNOTATION_INTERFACE_CREATE_PARAMS);
    return DEFAULT_ANNOTATION_INTERFACE;
}

inline AbckitArktsAnnotation *ClassAddAnnotation(AbckitArktsClass *klass,
                                                 const struct AbckitArktsAnnotationCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_CLASS);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_ANNOTATION_CREATE_PARAMS);
    return DEFAULT_ANNOTATION;
}

inline void ClassRemoveAnnotation(AbckitArktsClass *klass, AbckitArktsAnnotation *anno)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_CLASS);
    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
}

inline AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddField(
    AbckitArktsAnnotationInterface *ai, const struct AbckitArktsAnnotationInterfaceFieldCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ai == DEFAULT_ANNOTATION_INTERFACE);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_ANNOTATION_INTERFACE_FIELD_CREATE_PARAMS);
    return DEFAULT_ANNOTATION_INTERFACE_FIELD;
}

inline void AnnotationInterfaceRemoveField(AbckitArktsAnnotationInterface *ai,
                                           AbckitArktsAnnotationInterfaceField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ai == DEFAULT_ANNOTATION_INTERFACE);
    EXPECT_TRUE(field == DEFAULT_ANNOTATION_INTERFACE_FIELD);
}

inline AbckitArktsAnnotation *FunctionAddAnnotation(AbckitArktsFunction *function,
                                                    const struct AbckitArktsAnnotationCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_FUNCTION);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_ANNOTATION_CREATE_PARAMS);
    return DEFAULT_ANNOTATION;
}

inline void FunctionRemoveAnnotation(AbckitArktsFunction *function, AbckitArktsAnnotation *anno)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_FUNCTION);
    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
}

inline AbckitArktsAnnotationElement *AnnotationAddAnnotationElement(
    AbckitArktsAnnotation *anno, struct AbckitArktsAnnotationElementCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
    EXPECT_TRUE(params == DEFAULT_STRUCT_ARKTS_ANNOTATION_ELEMENT_CREATE_PARAMS);
    return DEFAULT_ANNOTATION_ELEMENT;
}

inline void AnnotationRemoveAnnotationElement(AbckitArktsAnnotation *anno, AbckitArktsAnnotationElement *elem)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
    EXPECT_TRUE(elem == DEFAULT_ANNOTATION_ELEMENT);
}

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

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock

AbckitArktsModifyApi const *AbckitGetMockArktsModifyApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::g_arktsModifyApiImpl;
}
