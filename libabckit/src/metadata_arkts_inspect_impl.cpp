/**
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

#include <cassert>
#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"

#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/statuses.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/helpers_common.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_inspect_dynamic.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_ARKTS_TARGET(m)                                                     \
    if ((m)->target != ABCKIT_TARGET_ARK_TS_V1 && (m)->target != ABCKIT_TARGET_ARK_TS_V2) { \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);                      \
        /* CC-OFFNXT(G.PRE.05) code generation */                                           \
        return nullptr;                                                                     \
    }

namespace libabckit {

// ========================================
// File
// ========================================

// ========================================
// Module
// ========================================

extern "C" AbckitCoreModule *ArktsModuleToCoreModule(AbckitArktsModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->core;
}

extern "C" AbckitArktsModule *CoreModuleToArktsModule(AbckitCoreModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(m);
    return m->GetArkTSImpl();
}

/* ========================================
 * Namespace
 * ======================================== */

extern "C" AbckitCoreNamespace *ArktsNamespaceToCoreNamespace(AbckitArktsNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    return n->core;
}

extern "C" AbckitArktsNamespace *CoreNamespaceToArktsNamespace(AbckitCoreNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(n->owningModule);
    return n->GetArkTSImpl();
}

extern "C" AbckitArktsFunction *ArktsV1NamespaceGetConstructor(AbckitArktsNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    if (n->core->owningModule->target != ABCKIT_TARGET_ARK_TS_V1) {
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }
    return n->f->GetArkTSImpl();
}

// ========================================
// ImportDescriptor
// ========================================

extern "C" AbckitCoreImportDescriptor *ArktsImportDescriptorToCoreImportDescriptor(AbckitArktsImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->core;
}

extern "C" AbckitArktsImportDescriptor *CoreImportDescriptorToArktsImportDescriptor(AbckitCoreImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(i->importingModule);
    return i->GetArkTSImpl();
}

// ========================================
// ExportDescriptor
// ========================================

extern "C" AbckitCoreExportDescriptor *ArktsExportDescriptorToCoreExportDescriptor(AbckitArktsExportDescriptor *e)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(e, nullptr);
    return e->core;
}

extern "C" AbckitArktsExportDescriptor *CoreExportDescriptorToArktsExportDescriptor(AbckitCoreExportDescriptor *e)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(e, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(e->exportingModule);
    return e->GetArkTSImpl();
}

// ========================================
// Class
// ========================================

extern "C" AbckitCoreClass *ArktsClassToCoreClass(AbckitArktsClass *c)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(c, nullptr);
    return c->core;
}

extern "C" AbckitArktsClass *CoreClassToArktsClass(AbckitCoreClass *c)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(c, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(c->owningModule);
    return c->GetArkTSImpl();
}

// ========================================
// Function
// ========================================

extern "C" AbckitCoreFunction *ArktsFunctionToCoreFunction(AbckitArktsFunction *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->core;
}

extern "C" AbckitArktsFunction *CoreFunctionToArktsFunction(AbckitCoreFunction *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(m->owningModule);
    return m->GetArkTSImpl();
}

extern "C" bool FunctionIsNative(AbckitArktsFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    switch (function->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionIsNativeDynamic(function->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionIsNativeStatic(function->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Annotation
// ========================================

extern "C" AbckitCoreAnnotation *ArktsAnnotationToCoreAnnotation(AbckitArktsAnnotation *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotation *CoreAnnotationToArktsAnnotation(AbckitCoreAnnotation *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(a->ai->owningModule);
    return a->GetArkTSImpl();
}

// ========================================
// AnnotationElement
// ========================================

extern "C" AbckitCoreAnnotationElement *ArktsAnnotationElementToCoreAnnotationElement(AbckitArktsAnnotationElement *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotationElement *CoreAnnotationElementToArktsAnnotationElement(AbckitCoreAnnotationElement *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(a->ann->ai->owningModule);
    return a->GetArkTSImpl();
}

// ========================================
// AnnotationInterface
// ========================================

extern "C" AbckitCoreAnnotationInterface *ArktsAnnotationInterfaceToCoreAnnotationInterface(
    AbckitArktsAnnotationInterface *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotationInterface *CoreAnnotationInterfaceToArktsAnnotationInterface(
    AbckitCoreAnnotationInterface *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(a->owningModule);
    return a->GetArkTSImpl();
}

// ========================================
// AnnotationInterfaceField
// ========================================

extern "C" AbckitCoreAnnotationInterfaceField *ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(
    AbckitArktsAnnotationInterfaceField *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotationInterfaceField *CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(
    AbckitCoreAnnotationInterfaceField *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(a->ai->owningModule);
    return a->GetArkTSImpl();
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

// ========================================
// Literal
// ========================================

AbckitArktsInspectApi g_arktsInspectApiImpl = {
    // ========================================
    // File
    // ========================================

    // ========================================
    // Module
    // ========================================

    ArktsModuleToCoreModule, CoreModuleToArktsModule,

    /* ========================================
     * Namespace
     * ======================================== */

    ArktsNamespaceToCoreNamespace, CoreNamespaceToArktsNamespace, ArktsV1NamespaceGetConstructor,

    // ========================================
    // ImportDescriptor
    // ========================================

    ArktsImportDescriptorToCoreImportDescriptor, CoreImportDescriptorToArktsImportDescriptor,

    // ========================================
    // ExportDescriptor
    // ========================================

    ArktsExportDescriptorToCoreExportDescriptor, CoreExportDescriptorToArktsExportDescriptor,

    // ========================================
    // Class
    // ========================================

    ArktsClassToCoreClass, CoreClassToArktsClass,

    // ========================================
    // Function
    // ========================================

    ArktsFunctionToCoreFunction, CoreFunctionToArktsFunction, FunctionIsNative,

    // ========================================
    // Annotation
    // ========================================

    ArktsAnnotationToCoreAnnotation, CoreAnnotationToArktsAnnotation,

    // ========================================
    // AnnotationElement
    // ========================================

    ArktsAnnotationElementToCoreAnnotationElement, CoreAnnotationElementToArktsAnnotationElement,

    // ========================================
    // AnnotationInterface
    // ========================================

    ArktsAnnotationInterfaceToCoreAnnotationInterface, CoreAnnotationInterfaceToArktsAnnotationInterface,

    // ========================================
    // AnnotationInterfaceField
    // ========================================

    ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField,
    CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField,

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
    // Literal
    // ========================================
};

// ========================================
// Enumerators
// ========================================

// ========================================
// Module
// ========================================

void ArkTSModuleEnumerateImports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreImportDescriptor *i, void *data))
{
    for (auto &id : m->id) {
        if (!cb(id.get(), data)) {
            return;
        }
    }
}

void ArkTSModuleEnumerateExports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreExportDescriptor *e, void *data))
{
    for (auto &ed : m->ed) {
        if (!cb(ed.get(), data)) {
            return;
        }
    }
}

void ArkTSModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data))
{
    for (auto &n : m->namespaces) {
        if (!cb(n.get(), data)) {
            return;
        }
    }
}

void ArkTSModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool cb(AbckitCoreClass *klass, void *data))
{
    ModuleEnumerateClassesHelper(m, data, cb);
}

void ArkTSModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                           bool (*cb)(AbckitCoreFunction *function, void *data))
{
    ModuleEnumerateTopLevelFunctionsHelper(m, data, cb);
}

void ArkTSModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data))
{
    switch (m->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleEnumerateAnonymousFunctionsDynamic(m, data, cb);
        case ABCKIT_TARGET_ARK_TS_V2:
            return ModuleEnumerateAnonymousFunctionsStatic(m, data, cb);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

void ArkTSModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    ModuleEnumerateAnnotationInterfacesHelper(m, data, cb);
}

// ========================================
// Module
// ========================================

void ArkTSNamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                       bool (*cb)(AbckitCoreNamespace *n, void *data))
{
    for (auto &subn : n->namespaces) {
        if (!cb(subn.get(), data)) {
            return;
        }
    }
}

void ArkTSNamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    for (auto &c : n->classes) {
        if (!cb(c.get(), data)) {
            return;
        }
    }
}

void ArkTSNamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                              bool (*cb)(AbckitCoreFunction *func, void *data))
{
    for (auto &f : n->functions) {
        if (!cb(f.get(), data)) {
            return;
        }
    }
}

// ========================================
// Class
// ========================================

void ArkTSClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data))
{
    ClassEnumerateMethodsHelper(klass, data, cb);
}

void ArkTSClassEnumerateAnnotations(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_BAD_ARGUMENT_VOID(klass)
    LIBABCKIT_BAD_ARGUMENT_VOID(cb)

    for (auto &annotation : klass->annotations) {
        if (!cb(annotation.get(), data)) {
            return;
        }
    }
}

// ========================================
// Function
// ========================================

void ArkTSFunctionEnumerateNestedFunctions([[maybe_unused]] AbckitCoreFunction *function, [[maybe_unused]] void *data,
                                           [[maybe_unused]] bool (*cb)(AbckitCoreFunction *nestedFunc, void *data))
{
    // There is no nested functions in ArkTS
}

void ArkTSFunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                         bool (*cb)(AbckitCoreClass *nestedClass, void *data))
{
    for (auto &c : function->nestedClasses) {
        if (!cb(c.get(), data)) {
            break;
        }
    }
}

void ArkTSFunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                       bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_BAD_ARGUMENT_VOID(function)
    LIBABCKIT_BAD_ARGUMENT_VOID(cb)

    for (auto &annotation : function->annotations) {
        if (!cb(annotation.get(), data)) {
            return;
        }
    }
}

// ========================================
// Annotation
// ========================================

void ArkTSAnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                      bool (*cb)(AbckitCoreAnnotationElement *ae, void *data))
{
    LIBABCKIT_BAD_ARGUMENT_VOID(anno)
    LIBABCKIT_BAD_ARGUMENT_VOID(cb)

    LIBABCKIT_BAD_ARGUMENT_VOID(anno->ai)

    AbckitCoreModule *m = anno->ai->owningModule;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(m->file)

    for (auto &elem : anno->elements) {
        if (!cb(elem.get(), data)) {
            return;
        }
    }
}

// ========================================
// AnnotationInterface
// ========================================

void ArkTSAnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                             bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data))
{
    LIBABCKIT_BAD_ARGUMENT_VOID(ai)
    LIBABCKIT_BAD_ARGUMENT_VOID(cb)

    for (auto &ed : ai->fields) {
        if (!cb(ed.get(), data)) {
            return;
        }
    }
}

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitArktsInspectApi const *AbckitGetArktsInspectApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockArktsInspectApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_arktsInspectApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
