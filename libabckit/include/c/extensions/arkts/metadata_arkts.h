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

#ifndef LIBABCKIT_METADATA_ARKTS_H
#define LIBABCKIT_METADATA_ARKTS_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif /* __cplusplus */

#include "../../declarations.h"
#include "../../api_version.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Struct that holds the pointers to the non-modifying API for Arkts-specific Abckit types.
 */
struct AbckitArktsInspectApi {
    /* ========================================
     * Language-independent abstractions
     * ======================================== */

    /* ========================================
     * File
     * ======================================== */

    /* ========================================
     * String
     * ======================================== */

    /* ========================================
     * Type
     * ======================================== */

    /* ========================================
     * Value
     * ======================================== */

    /* ========================================
     * Literal
     * ======================================== */

    /* ========================================
     * LiteralArray
     * ======================================== */

    /* ========================================
     * Language-dependent abstractions
     * ======================================== */

    /* ========================================
     * Module
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsModule` to the instance of type `AbckitCoreModule`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `m`.
     * @param [ in ] m - Module to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     */
    AbckitCoreModule *(*arktsModuleToCoreModule)(AbckitArktsModule *m);

    /**
     * @brief Convert an instance of type `AbckitCoreModule` to the instance of type `AbckitArktsModule`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `m`.
     * @param [ in ] m - Module to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsModule *(*coreModuleToArktsModule)(AbckitCoreModule *m);

    /* ========================================
     * Namespace
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsNamespace` to the instance of type `AbckitCoreNamespace`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `ns`.
     * @param [ in ] ns - Namespace to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     */
    AbckitCoreNamespace *(*arktsNamespaceToCoreNamespace)(AbckitArktsNamespace *ns);

    /**
     * @brief Convert an instance of type `AbckitCoreNamespace` to the instance of type `AbckitArktsNamespace`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `ns`.
     * @param [ in ] ns - Namespace to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ns` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsNamespace *(*coreNamespaceToArktsNamespace)(AbckitCoreNamespace *ns);

    /**
     * @brief Returns constructor function for namespace.
     * @return Function that is invoked upon namespace `ns` construction.
     * @param [ in ] ns - Namespace to get the constructor function for.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ns` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    AbckitArktsFunction *(*arktsV1NamespaceGetConstructor)(AbckitArktsNamespace *ns);

    /* ========================================
     * ImportDescriptor
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsImportDescriptorToCoreImportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `id`.
     * @param [ in ] id - Import descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     */
    AbckitCoreImportDescriptor *(*arktsImportDescriptorToCoreImportDescriptor)(AbckitArktsImportDescriptor *id);

    /**
     * @brief Convert an instance of type `AbckitCoreImportDescriptorToArktsImportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `id`.
     * @param [ in ] id - Import descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `id` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsImportDescriptor *(*coreImportDescriptorToArktsImportDescriptor)(AbckitCoreImportDescriptor *id);

    /* ========================================
     * ExportDescriptor
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsExportDescriptorToCoreExportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `ed`.
     * @param [ in ] ed - Export descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     */
    AbckitCoreExportDescriptor *(*arktsExportDescriptorToCoreExportDescriptor)(AbckitArktsExportDescriptor *ed);

    /**
     * @brief Convert an instance of type `AbckitCoreExportDescriptorToArktsExportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `ed`.
     * @param [ in ] ed - Export descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ed` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsExportDescriptor *(*coreExportDescriptorToArktsExportDescriptor)(AbckitCoreExportDescriptor *ed);

    /* ========================================
     * Class
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsClass` to the instance of type `AbckitCoreClass`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `klass`.
     * @param [ in ] klass - Class to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     */
    AbckitCoreClass *(*arktsClassToCoreClass)(AbckitArktsClass *klass);

    /**
     * @brief Convert an instance of type `AbckitCoreClass` to the instance of type `AbckitArktsClass`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `klass`.
     * @param [ in ] klass - Class to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsClass *(*coreClassToArktsClass)(AbckitCoreClass *klass);

    /* ========================================
     * Function
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsFunction` to the instance of type `AbckitCoreFunction`, which can
     * be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `function`.
     * @param [ in ] function - Function to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     */
    AbckitCoreFunction *(*arktsFunctionToCoreFunction)(AbckitArktsFunction *function);

    /**
     * @brief Convert an instance of type `AbckitCoreFunction` to the instance of type `AbckitArktsFunction`, which can
     * be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `function`.
     * @param [ in ] function - Function to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsFunction *(*coreFunctionToArktsFunction)(AbckitCoreFunction *function);

    /**
     * @brief Check whether the `function` is native.
     * @return `true` if `function` is native, `false` otherwise.
     * @param [ in ] function - Function to inspect.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     */
    bool (*functionIsNative)(AbckitArktsFunction *function);

    /* ========================================
     * Annotation
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotation` to the instance of type `AbckitCoreAnnotation`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `anno`.
     * @param [ in ] anno - Annotation to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     */
    AbckitCoreAnnotation *(*arktsAnnotationToCoreAnnotation)(AbckitArktsAnnotation *anno);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotation` to the instance of type `AbckitArktsAnnotation`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `anno`.
     * @param [ in ] anno - Annotation to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `anno` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotation *(*coreAnnotationToArktsAnnotation)(AbckitCoreAnnotation *anno);

    /* ========================================
     * AnnotationElement
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotationElement` to the instance of type
     * `AbckitCoreAnnotationElement`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `annoElem`.
     * @param [ in ] annoElem - Annotation element to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoElem` is NULL.
     */
    AbckitCoreAnnotationElement *(*arktsAnnotationElementToCoreAnnotationElement)(
        AbckitArktsAnnotationElement *annoElem);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotationElement` to the instance of type
     * `AbckitArktsAnnotationElement`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `annoElem`.
     * @param [ in ] annoElem - Annotation element to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoElem` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoElem` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationElement *(*coreAnnotationElementToArktsAnnotationElement)(
        AbckitCoreAnnotationElement *annoElem);

    /* ========================================
     * AnnotationInterface
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotationInterface` to the instance of type
     * `AbckitCoreAnnotationInterface`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `annoInterface`.
     * @param [ in ] annoInterface - Annotataion interface to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterface` is NULL.
     */
    AbckitCoreAnnotationInterface *(*arktsAnnotationInterfaceToCoreAnnotationInterface)(
        AbckitArktsAnnotationInterface *annoInterface);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotationInterface` to the instance of type
     * `AbckitArktsAnnotationInterface`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `annoInterface`.
     * @param [ in ] annoInterface - Annotataion interface to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterface` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoInterface` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationInterface *(*coreAnnotationInterfaceToArktsAnnotationInterface)(
        AbckitCoreAnnotationInterface *annoInterface);

    /* ========================================
     * AnnotationInterfaceField
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotationInterfaceField` to the instance of type
     * `AbckitCoreAnnotationInterfaceField`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `annoInterfaceField`.
     * @param [ in ] annoInterfaceField - Annotation inteface field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterfaceField` is NULL.
     */
    AbckitCoreAnnotationInterfaceField *(*arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField)(
        AbckitArktsAnnotationInterfaceField *annoInterfaceField);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotationInterfaceField` to the instance of type
     * `AbckitArktsAnnotationInterfaceField`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `annoInterfaceField`.
     * @param [ in ] annoInterfaceField - Annotation inteface field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterfaceField` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoInterfaceField` is does not have `ABCKIT_TARGET_ARK_TS_V1`
     * or `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationInterfaceField *(*coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField)(
        AbckitCoreAnnotationInterfaceField *annoInterfaceField);
};

/**
 * @brief Struct that is used to create new annotation interfaces.
 */
struct AbckitArktsAnnotationInterfaceCreateParams {
    /**
     * @brief Name of the created annotation interface.
     */
    const char *name;
};

/**
 * @brief Struct that is used to create new annotation interface fields.
 */
struct AbckitArktsAnnotationInterfaceFieldCreateParams {
    /**
     * @brief Name of the created annotation interface field.
     */
    const char *name;
    /**
     * @brief Type of the created annotation interface field.
     */
    AbckitType *type;
    /**
     * @brief Default value of the created annotation interface field. Leave as NULL for no default value.
     */
    AbckitValue *defaultValue;
};

/**
 * @brief Struct that is used to create new annotations.
 */
struct AbckitArktsAnnotationCreateParams {
    /**
     * @brief Annotation interface that created annotation instantiates.
     */
    AbckitArktsAnnotationInterface *ai;
};

/**
 * @brief Struct that is used to create new annotation elements.
 */
struct AbckitArktsAnnotationElementCreateParams {
    /**
     * @brief Name of the created annotation element. Must be equal to one of the fields of the corresponding annotation
     * interface.
     */
    const char *name;
    /**
     * @brief Value that should be assigned to the annotation element.
     */
    AbckitValue *value;
};

/**
 * @brief Struct that is used to create new imports.
 */
struct AbckitArktsImportFromDynamicModuleCreateParams {
    /**
     * @brief Import name. For namespace imports equals to "*". For default imports equals to "default". For regular
     * imports is the same as in user code.
     */
    const char *name;
    /**
     * @brief Alias name for the import. For namespace imports is the same as in user code. For delault import is the
     * same as the default import name in user code. For regular imports is the same as in user code.
     */
    const char *alias;
};

/**
 * @brief Struct that is used to create new exports.
 */
struct AbckitArktsDynamicModuleExportCreateParams {
    /**
     * @brief Name of the entity that should be exported. For star exports equals to "*". For indirect exports is the
     * same as in user code. For local exports is the same as in user code.
     */
    const char *name;
    /**
     * @brief Alias under which entity should be exported. For star exports equals nullptr. For indirect exports is the
     * same as in user code. For local exports is the same as in user code.
     */
    const char *alias;
};

/**
 * @brief Struct that is used to create new external modules.
 */
struct AbckitArktsV1ExternalModuleCreateParams {
    /**
     * @brief Name of the created external module
     */
    const char *name;
};

/**
 * @brief Struct that holds the pointers to the modifying API for Arkts-specific Abckit types.
 */
struct AbckitArktsModifyApi {
    /* ========================================
     * File
     * ======================================== */

    /**
     * @brief Creates an external Arkts module with target `ABCKIT_TARGET_ARK_TS_V1` and adds it to the file `file`.
     * @return Pointer to the newly created module.
     * @param [ in ] file - Binary file to .
     * @param [ in ] params - Data that is used to create the external module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Allocates
     */
    AbckitArktsModule *(*fileAddExternalModuleArktsV1)(AbckitFile *file,
                                                       const struct AbckitArktsV1ExternalModuleCreateParams *params);

    /* ========================================
     * Module
     * ======================================== */

    /**
     * @brief Adds import from one Arkts module to another Arkts module.
     * @return Pointer to the newly created import descriptor.
     * @param [ in ] importing - Importing module.
     * @param [ in ] imported - The module the `importing` module imports from.
     * @param [ in ] params - Data that is used to create the import.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `importing` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `imported` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Allocates
     */
    AbckitArktsImportDescriptor *(*moduleAddImportFromArktsV1ToArktsV1)(
        AbckitArktsModule *importing, AbckitArktsModule *imported,
        const struct AbckitArktsImportFromDynamicModuleCreateParams *params);

    /**
     * @brief Removes import `id` from module `m`.
     * @return None.
     * @param [ in ] m - Module to remove the import `id` from.
     * @param [ in ] id - Import to remove from the module `m`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module `m` does not have the import descriptor `id`.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` doesn;t have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void (*moduleRemoveImport)(AbckitArktsModule *m, AbckitArktsImportDescriptor *id);

    /**
     * @brief Adds export from one Arkts module to another Arkts module.
     * @return Pointer to the newly created export descriptor.
     * @param [ in ] exporting - The module to add export to.
     * @param [ in ] exported - The module the entity is exported from. In case of local export is the same as
     * `exporting`.
     * @param [ in ] params - Data that is used to create the export.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exporting` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsExportDescriptor *(*moduleAddExportFromArktsV1ToArktsV1)(
        AbckitArktsModule *exporting, AbckitArktsModule *exported,
        const struct AbckitArktsDynamicModuleExportCreateParams *params);

    /**
     * @brief Removes export `ed` from module `m`.
     * @return None.
     * @param [ in ] m - Module to remove the export `e` from.
     * @param [ in ] ed - Export to remove from the module `m`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module `m` does not have the export descriptor `ed`.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void (*moduleRemoveExport)(AbckitArktsModule *m, AbckitArktsExportDescriptor *ed);

    /**
     * @brief Adds new annotation interface to the module `m`.
     * @return Pointer to the newly constructed annotation interface.
     * @param [ in ] m - Module to add annotation interface to.
     * @param [ in ] params - Data that is used to create the annotation interface.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsAnnotationInterface *(*moduleAddAnnotationInterface)(
        AbckitArktsModule *m, const struct AbckitArktsAnnotationInterfaceCreateParams *params);

    /* ========================================
     * Class
     * ======================================== */

    /**
     * @brief Add annotation to the class declaration.
     * @return Pointer to the newly created annotation.
     * @param [ in ] klass - Class to add annotation to.
     * @param [ in ] params - Data that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if class `klass` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsAnnotation *(*classAddAnnotation)(AbckitArktsClass *klass,
                                                 const struct AbckitArktsAnnotationCreateParams *params);

    /**
     * @brief Remove annotation from the class declaration.
     * @return None.
     * @param [ in ] klass - Class to remove annotation from.
     * @param [ in ] anno - Annotation to remove.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if class `klass` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void (*classRemoveAnnotation)(AbckitArktsClass *klass, AbckitArktsAnnotation *anno);

    /* ========================================
     * AnnotationInterface
     * ======================================== */

    /**
     * @brief Add new field to the annotation interface.
     * @return Pointer to the newly created annotation field.
     * @param [ in ] ai - Annotation interface to add new field to.
     * @param [ in ] params -  Data that is used to create the field of the annotation interface `ai`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation interface `ai` doesn't have `ABCKIT_TARGET_ARK_TS_V1`
     * target.
     * @note Allocates
     */
    AbckitArktsAnnotationInterfaceField *(*annotationInterfaceAddField)(
        AbckitArktsAnnotationInterface *ai, const struct AbckitArktsAnnotationInterfaceFieldCreateParams *params);

    /**
     * @brief Remove field from the annotation interface.
     * @return None.
     * @param [ in ] ai - Annotation interface to remove the field `field` from.
     * @param [ in ] field - Field to remove from the annotation interface `ai`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation interface `ai` doesn't have `ABCKIT_TARGET_ARK_TS_V1`
     * target.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if annotation interface `ai` does not have the field `field`.
     */
    void (*annotationInterfaceRemoveField)(AbckitArktsAnnotationInterface *ai,
                                           AbckitArktsAnnotationInterfaceField *field);

    /* ========================================
     * Function
     * ======================================== */

    /**
     * @brief Function to add annotation to.
     * @return Pointer to the newly created annotation.
     * @param [ in ] function - Function to add annotation to.
     * @param [ in ] params -  Data that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if function `function` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsAnnotation *(*functionAddAnnotation)(AbckitArktsFunction *function,
                                                    const struct AbckitArktsAnnotationCreateParams *params);

    /**
     * @brief Remove annotation from the function.
     * @return None.
     * @param [ in ] function - Function to remove annotation `anno` from.
     * @param [ in ] anno - Annotation to remove from the function `function`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if function `function` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void (*functionRemoveAnnotation)(AbckitArktsFunction *function, AbckitArktsAnnotation *anno);

    /* ========================================
     * Annotation
     * ======================================== */

    /**
     * @brief Add annotation element to the existing annotation.
     * @return Pointer to the newly created annotation element.
     * @param [ in ] anno - Annotation to add new element to.
     * @param [ in ] params -  Data that is used to create the annotation element.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation `anno` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsAnnotationElement *(*annotationAddAnnotationElement)(
        AbckitArktsAnnotation *anno, struct AbckitArktsAnnotationElementCreateParams *params);

    /**
     * @brief Remove annotation element `elem` from the annotation `anno`.
     * @return None.
     * @param [ in ] anno - Annotation to remove the annotataion element `elem` from.
     * @param [ in ] elem - Annotation element to remove from the annotation `anno`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `elem` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation `anno` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void (*annotationRemoveAnnotationElement)(AbckitArktsAnnotation *anno, AbckitArktsAnnotationElement *elem);

    /* ========================================
     * Type
     * ======================================== */

    /* ========================================
     * Value
     * ======================================== */

    /* ========================================
     * String
     * ======================================== */

    /* ========================================
     * LiteralArray
     * ======================================== */

    /* ========================================
     * Literal
     * ======================================== */
};

/**
 * @brief Instantiates non-modifying API for Arkts-specific Abckit types.
 * @return Instance of the `AbckitApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
struct AbckitArktsInspectApi const *AbckitGetArktsInspectApiImpl(enum AbckitApiVersion version);

/**
 * @brief Instantiates modifying API for Arkts-specific Abckit types.
 * @return Instance of the `AbckitApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
struct AbckitArktsModifyApi const *AbckitGetArktsModifyApiImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBABCKIT_METADATA_ARKTS_H */
