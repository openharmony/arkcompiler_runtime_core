/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this File except in compliance with the License.
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

#ifndef ABCKIT_INSPECT_IMPL_MOCK
#define ABCKIT_INSPECT_IMPL_MOCK

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../../include/c/metadata_core.h"

#include <gtest/gtest.h>

namespace libabckit::mock::metadata_inspect {

// NOLINTBEGIN(readability-identifier-naming)

// ========================================
// File
// ========================================

AbckitFileVersion FileGetVersion(AbckitFile *file)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return DEFAULT_FILE_VERSION;
}

bool FileEnumerateModules(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return cb(DEFAULT_CORE_MODULE, data);
}

bool FileEnumerateExternalModules(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return cb(DEFAULT_CORE_MODULE, data);
}

// ========================================
// Module
// ========================================

AbckitFile *ModuleGetFile(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_FILE;
}

AbckitTarget ModuleGetTarget(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_TARGET;
}

AbckitString *ModuleGetName(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_STRING;
}

bool ModuleIsExternal(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return true;
}

bool ModuleEnumerateImports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreImportDescriptor *i, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_IMPORT_DESCRIPTOR, data);
}

bool ModuleEnumerateExports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreExportDescriptor *e, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_EXPORT_DESCRIPTOR, data);
}

bool ModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_NAMESPACE, data);
}

bool ModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool ModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                      bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool ModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                       bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool ModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                         bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_ANNOTATION_INTERFACE, data);
}

// ========================================
// Namespace
// ========================================

AbckitString *NamespaceGetName(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_STRING;
}

AbckitCoreNamespace *NamespaceGetParentNamespace(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_CORE_NAMESPACE;
}

bool NamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                  bool (*cb)(AbckitCoreNamespace *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_NAMESPACE, data);
}

bool NamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool NamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                         bool (*cb)(AbckitCoreFunction *func, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

// ========================================
// ImportDescriptor
// ========================================

AbckitFile *ImportDescriptorGetFile(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_FILE;
}

AbckitCoreModule *ImportDescriptorGetImportedModule(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitCoreModule *ImportDescriptorGetImportingModule(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitString *ImportDescriptorGetName(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

AbckitString *ImportDescriptorGetAlias(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

// ========================================
// ExportDescriptor
// ========================================

AbckitFile *ExportDescriptorGetFile(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_FILE;
}

AbckitCoreModule *ExportDescriptorGetExportingModule(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitCoreModule *ExportDescriptorGetExportedModule(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitString *ExportDescriptorGetName(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

AbckitString *ExportDescriptorGetAlias(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

// ========================================
// Class
// ========================================

AbckitFile *ClassGetFile(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_FILE;
}

AbckitCoreModule *ClassGetModule(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_MODULE;
}

AbckitString *ClassGetName(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_STRING;
}

AbckitCoreFunction *ClassGetParentFunction(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_FUNCTION;
}

AbckitCoreNamespace *ClassGetParentNamespace(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_NAMESPACE;
}

bool ClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool ClassEnumerateAnnotations(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

// ========================================
// AnnotationInterface
// ========================================

AbckitFile *AnnotationInterfaceGetFile(AbckitCoreAnnotationInterface *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_FILE;
}

AbckitCoreModule *AnnotationInterfaceGetModule(AbckitCoreAnnotationInterface *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_CORE_MODULE;
}

AbckitString *AnnotationInterfaceGetName(AbckitCoreAnnotationInterface *ai)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ai == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_STRING;
}

bool AnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                        bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ai == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return cb(DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD, data);
}

// ========================================
// AnnotationInterfaceField
// ========================================

AbckitFile *AnnotationInterfaceFieldGetFile(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_FILE;
}

AbckitCoreAnnotationInterface *AnnotationInterfaceFieldGetInterface(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_CORE_ANNOTATION_INTERFACE;
}

AbckitString *AnnotationInterfaceFieldGetName(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_STRING;
}

AbckitType *AnnotationInterfaceFieldGetType(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_TYPE;
}

AbckitValue *AnnotationInterfaceFieldGetDefaultValue(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_VALUE;
}

// ========================================
// Function
// ========================================

AbckitFile *FunctionGetFile(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_FILE;
}

AbckitCoreModule *FunctionGetModule(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_MODULE;
}

AbckitString *FunctionGetName(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_STRING;
}

AbckitCoreFunction *FunctionGetParentFunction(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_FUNCTION;
}

AbckitCoreClass *FunctionGetParentClass(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_CLASS;
}

AbckitCoreNamespace *FunctionGetParentNamespace(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_NAMESPACE;
}

bool FunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                      bool (*cb)(AbckitCoreFunction *nestedFunc, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool FunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                    bool (*cb)(AbckitCoreClass *nestedClass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool FunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                  bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

AbckitGraph *CreateGraphFromFunction(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_GRAPH;
}

bool FunctionIsStatic(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsCtor(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsAnonymous(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

// ========================================
// Annotation
// ========================================

AbckitFile *AnnotationGetFile(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_FILE;
}
AbckitCoreAnnotationInterface *AnnotationGetInterface(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_CORE_ANNOTATION_INTERFACE;
}

bool AnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                 bool (*cb)(AbckitCoreAnnotationElement *ae, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return cb(DEFAULT_CORE_ANNOTATION_ELEMENT, data);
}

// ========================================
// AnnotationElement
// ========================================

AbckitFile *AnnotationElementGetFile(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_FILE;
}

AbckitCoreAnnotation *AnnotationElementGetAnnotation(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_CORE_ANNOTATION;
}

AbckitString *AnnotationElementGetName(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_STRING;
}

AbckitValue *AnnotationElementGetValue(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_VALUE;
}

// ========================================
// Type
// ========================================

AbckitTypeId TypeGetTypeId(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_TYPE_ID;
}

AbckitCoreClass *TypeGetReferenceClass(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_CORE_CLASS;
}

// ========================================
// Value
// ========================================

AbckitFile *ValueGetFile(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_FILE;
}

AbckitType *ValueGetType(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_TYPE;
}

bool ValueGetU1(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_BOOL;
}

double ValueGetDouble(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_DOUBLE;
}

AbckitString *ValueGetString(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_STRING;
}

AbckitLiteralArray *ArrayValueGetLiteralArray(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_LITERAL_ARRAY;
}

// ========================================
// String
// ========================================

const char *AbckitStringToString(AbckitString *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_STRING);
    return DEFAULT_CONST_CHAR;
}

// ========================================
// LiteralArray
// ========================================

bool LiteralArrayEnumerateElements(AbckitLiteralArray *litArr, void *data,
                                   bool (*cb)(AbckitFile *file, AbckitLiteral *v, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(litArr == DEFAULT_LITERAL_ARRAY);
    return cb(DEFAULT_FILE, DEFAULT_LITERAL, data);
}

// ========================================
// Literal
// ========================================

AbckitFile *LiteralGetFile(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_FILE;
}

AbckitLiteralTag LiteralGetTag(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_LITERAL_TAG;
}

bool LiteralGetBool(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_BOOL;
}

uint8_t LiteralGetU8(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U8;
}

uint16_t LiteralGetU16(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U16;
}

uint16_t LiteralGetMethodAffiliate(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U16;
}

uint32_t LiteralGetU32(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U32;
}

uint64_t LiteralGetU64(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U64;
}

float LiteralGetFloat(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_FLOAT;
}

double LiteralGetDouble(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_DOUBLE;
}

AbckitLiteralArray *LiteralGetLiteralArray(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_LITERAL_ARRAY;
}

AbckitString *LiteralGetString(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_STRING;
}

AbckitString *LiteralGetMethod(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_STRING;
}

static AbckitInspectApi g_inspectApiImpl = {
    // ========================================
    // File
    // ========================================

    FileGetVersion,
    FileEnumerateModules,
    FileEnumerateExternalModules,

    // ========================================
    // String
    // ========================================

    AbckitStringToString,

    // ========================================
    // Type
    // ========================================

    TypeGetTypeId,
    TypeGetReferenceClass,

    // ========================================
    // Value
    // ========================================

    ValueGetFile,
    ValueGetType,
    ValueGetU1,
    ValueGetDouble,
    ValueGetString,
    ArrayValueGetLiteralArray,

    // ========================================
    // Literal
    // ========================================

    LiteralGetFile,
    LiteralGetTag,
    LiteralGetBool,
    LiteralGetU8,
    LiteralGetU16,
    LiteralGetMethodAffiliate,
    LiteralGetU32,
    LiteralGetU64,
    LiteralGetFloat,
    LiteralGetDouble,
    LiteralGetLiteralArray,
    LiteralGetString,
    LiteralGetMethod,

    // ========================================
    // LiteralArray
    // ========================================

    LiteralArrayEnumerateElements,

    // ========================================
    // Module
    // ========================================

    ModuleGetFile,
    ModuleGetTarget,
    ModuleGetName,
    ModuleIsExternal,
    ModuleEnumerateImports,
    ModuleEnumerateExports,
    ModuleEnumerateNamespaces,
    ModuleEnumerateClasses,
    ModuleEnumerateTopLevelFunctions,
    ModuleEnumerateAnonymousFunctions,
    ModuleEnumerateAnnotationInterfaces,

    // ========================================
    // Namespace
    // ========================================

    NamespaceGetName,
    NamespaceGetParentNamespace,
    NamespaceEnumerateNamespaces,
    NamespaceEnumerateClasses,
    NamespaceEnumerateTopLevelFunctions,

    // ========================================
    // ImportDescriptor
    // ========================================

    ImportDescriptorGetFile,
    ImportDescriptorGetImportingModule,
    ImportDescriptorGetImportedModule,
    ImportDescriptorGetName,
    ImportDescriptorGetAlias,

    // ========================================
    // ExportDescriptor
    // ========================================

    ExportDescriptorGetFile,
    ExportDescriptorGetExportingModule,
    ExportDescriptorGetExportedModule,
    ExportDescriptorGetName,
    ExportDescriptorGetAlias,

    // ========================================
    // Class
    // ========================================

    ClassGetFile,
    ClassGetModule,
    ClassGetName,
    ClassGetParentFunction,
    ClassGetParentNamespace,
    ClassEnumerateMethods,
    ClassEnumerateAnnotations,

    // ========================================
    // Function
    // ========================================

    FunctionGetFile,
    FunctionGetModule,
    FunctionGetName,
    FunctionGetParentFunction,
    FunctionGetParentClass,
    FunctionGetParentNamespace,
    FunctionEnumerateNestedFunctions,
    FunctionEnumerateNestedClasses,
    FunctionEnumerateAnnotations,
    CreateGraphFromFunction,
    FunctionIsStatic,
    FunctionIsCtor,
    FunctionIsAnonymous,

    // ========================================
    // Annotation
    // ========================================

    AnnotationGetFile,
    AnnotationGetInterface,
    AnnotationEnumerateElements,
    AnnotationElementGetFile,
    AnnotationElementGetAnnotation,
    AnnotationElementGetName,
    AnnotationElementGetValue,

    // ========================================
    // AnnotationInterface
    // ========================================

    AnnotationInterfaceGetFile,
    AnnotationInterfaceGetModule,
    AnnotationInterfaceGetName,
    AnnotationInterfaceEnumerateFields,

    // ========================================
    // AnnotationInterfaceField
    // ========================================

    AnnotationInterfaceFieldGetFile,
    AnnotationInterfaceFieldGetInterface,
    AnnotationInterfaceFieldGetName,
    AnnotationInterfaceFieldGetType,
    AnnotationInterfaceFieldGetDefaultValue,
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock::metadata_inspect

AbckitInspectApi const *AbckitGetMockInspectApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::metadata_inspect::g_inspectApiImpl;
}

#endif  // ABCKIT_INSPECT_IMPL_MOCK
