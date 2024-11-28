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

namespace libabckit::mock {

// NOLINTBEGIN(readability-identifier-naming)

// ========================================
// File
// ========================================

inline AbckitFileVersion FileGetVersion(AbckitFile *file)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return DEFAULT_FILE_VERSION;
}

inline bool FileEnumerateModules(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return cb(DEFAULT_CORE_MODULE, data);
}

inline bool FileEnumerateExternalModules(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return cb(DEFAULT_CORE_MODULE, data);
}

// ========================================
// Module
// ========================================

inline AbckitFile *ModuleGetFile(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_FILE;
}

inline AbckitTarget ModuleGetTarget(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_TARGET;
}

inline AbckitString *ModuleGetName(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_STRING;
}

inline bool ModuleIsExternal(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return true;
}

inline bool ModuleEnumerateImports(AbckitCoreModule *m, void *data,
                                   bool (*cb)(AbckitCoreImportDescriptor *i, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_IMPORT_DESCRIPTOR, data);
}

inline bool ModuleEnumerateExports(AbckitCoreModule *m, void *data,
                                   bool (*cb)(AbckitCoreExportDescriptor *e, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_EXPORT_DESCRIPTOR, data);
}

inline bool ModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_NAMESPACE, data);
}

inline bool ModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_CLASS, data);
}

inline bool ModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                             bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

inline bool ModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

inline bool ModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                                bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_ANNOTATION_INTERFACE, data);
}

// ========================================
// Namespace
// ========================================

inline AbckitString *NamespaceGetName(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_STRING;
}

inline AbckitCoreNamespace *NamespaceGetParentNamespace(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_CORE_NAMESPACE;
}

inline bool NamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                         bool (*cb)(AbckitCoreNamespace *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_NAMESPACE, data);
}

inline bool NamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data,
                                      bool (*cb)(AbckitCoreClass *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_CLASS, data);
}

inline bool NamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                                bool (*cb)(AbckitCoreFunction *func, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

// ========================================
// ImportDescriptor
// ========================================

inline AbckitFile *ImportDescriptorGetFile(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_FILE;
}

inline AbckitCoreModule *ImportDescriptorGetImportedModule(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

inline AbckitCoreModule *ImportDescriptorGetImportingModule(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

inline AbckitString *ImportDescriptorGetName(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

inline AbckitString *ImportDescriptorGetAlias(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

// ========================================
// ExportDescriptor
// ========================================

inline AbckitFile *ExportDescriptorGetFile(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_FILE;
}

inline AbckitCoreModule *ExportDescriptorGetExportingModule(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

inline AbckitCoreModule *ExportDescriptorGetExportedModule(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

inline AbckitString *ExportDescriptorGetName(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

inline AbckitString *ExportDescriptorGetAlias(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

// ========================================
// Class
// ========================================

inline AbckitFile *ClassGetFile(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_FILE;
}

inline AbckitCoreModule *ClassGetModule(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_MODULE;
}

inline AbckitString *ClassGetName(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_STRING;
}

inline AbckitCoreFunction *ClassGetParentFunction(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_FUNCTION;
}

inline AbckitCoreNamespace *ClassGetParentNamespace(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_NAMESPACE;
}

inline bool ClassEnumerateMethods(AbckitCoreClass *klass, void *data,
                                  bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

inline bool ClassEnumerateAnnotations(AbckitCoreClass *klass, void *data,
                                      bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

// ========================================
// AnnotationInterface
// ========================================

inline AbckitFile *AnnotationInterfaceGetFile(AbckitCoreAnnotationInterface *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_FILE;
}

inline AbckitCoreModule *AnnotationInterfaceGetModule(AbckitCoreAnnotationInterface *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_CORE_MODULE;
}

inline AbckitString *AnnotationInterfaceGetName(AbckitCoreAnnotationInterface *ai)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ai == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_STRING;
}

inline bool AnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                               bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ai == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return cb(DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD, data);
}

// ========================================
// AnnotationInterfaceField
// ========================================

inline AbckitFile *AnnotationInterfaceFieldGetFile(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_FILE;
}

inline AbckitCoreAnnotationInterface *AnnotationInterfaceFieldGetInterface(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_CORE_ANNOTATION_INTERFACE;
}

inline AbckitString *AnnotationInterfaceFieldGetName(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_STRING;
}

inline AbckitType *AnnotationInterfaceFieldGetType(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_TYPE;
}

inline AbckitValue *AnnotationInterfaceFieldGetDefaultValue(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_VALUE;
}

// ========================================
// Function
// ========================================

inline AbckitFile *FunctionGetFile(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_FILE;
}

inline AbckitCoreModule *FunctionGetModule(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_MODULE;
}

inline AbckitString *FunctionGetName(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_STRING;
}

inline AbckitCoreFunction *FunctionGetParentFunction(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_FUNCTION;
}

inline AbckitCoreClass *FunctionGetParentClass(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_CLASS;
}

inline AbckitCoreNamespace *FunctionGetParentNamespace(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_NAMESPACE;
}

inline bool FunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                             bool (*cb)(AbckitCoreFunction *nestedFunc, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

inline bool FunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                           bool (*cb)(AbckitCoreClass *nestedClass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_CLASS, data);
}

inline bool FunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                         bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

inline AbckitGraph *CreateGraphFromFunction(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_GRAPH;
}

inline bool FunctionIsStatic(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

inline bool FunctionIsCtor(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

inline bool FunctionIsAnonymous(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

// ========================================
// Annotation
// ========================================

inline AbckitFile *AnnotationGetFile(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_FILE;
}
inline AbckitCoreAnnotationInterface *AnnotationGetInterface(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_CORE_ANNOTATION_INTERFACE;
}

inline bool AnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                        bool (*cb)(AbckitCoreAnnotationElement *ae, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return cb(DEFAULT_CORE_ANNOTATION_ELEMENT, data);
}

// ========================================
// AnnotationElement
// ========================================

inline AbckitFile *AnnotationElementGetFile(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_FILE;
}

inline AbckitCoreAnnotation *AnnotationElementGetAnnotation(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_CORE_ANNOTATION;
}

inline AbckitString *AnnotationElementGetName(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_STRING;
}

inline AbckitValue *AnnotationElementGetValue(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_VALUE;
}

// ========================================
// Type
// ========================================

inline AbckitTypeId TypeGetTypeId(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_TYPE_ID;
}

inline AbckitCoreClass *TypeGetReferenceClass(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_CORE_CLASS;
}

// ========================================
// Value
// ========================================

inline AbckitFile *ValueGetFile(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_FILE;
}

inline AbckitType *ValueGetType(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_TYPE;
}

inline bool ValueGetU1(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_BOOL;
}

inline double ValueGetDouble(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_DOUBLE;
}

inline AbckitString *ValueGetString(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_STRING;
}

inline AbckitLiteralArray *ArrayValueGetLiteralArray(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_LITERAL_ARRAY;
}

// ========================================
// String
// ========================================

inline const char *AbckitStringToString(AbckitString *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_STRING);
    return DEFAULT_CONST_CHAR;
}

// ========================================
// LiteralArray
// ========================================

inline bool LiteralArrayEnumerateElements(AbckitLiteralArray *litArr, void *data,
                                          bool (*cb)(AbckitFile *file, AbckitLiteral *v, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(litArr == DEFAULT_LITERAL_ARRAY);
    return cb(DEFAULT_FILE, DEFAULT_LITERAL, data);
}

// ========================================
// Literal
// ========================================

inline AbckitFile *LiteralGetFile(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_FILE;
}

inline AbckitLiteralTag LiteralGetTag(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_LITERAL_TAG;
}

inline bool LiteralGetBool(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_BOOL;
}

inline uint8_t LiteralGetU8(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U8;
}

inline uint16_t LiteralGetU16(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U16;
}

inline uint16_t LiteralGetMethodAffiliate(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U16;
}

inline uint32_t LiteralGetU32(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U32;
}

inline uint64_t LiteralGetU64(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U64;
}

inline float LiteralGetFloat(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_FLOAT;
}

inline double LiteralGetDouble(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_DOUBLE;
}

inline AbckitLiteralArray *LiteralGetLiteralArray(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_LITERAL_ARRAY;
}

inline AbckitString *LiteralGetString(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_STRING;
}

inline AbckitString *LiteralGetMethod(AbckitLiteral *lit)
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

}  // namespace libabckit::mock

AbckitInspectApi const *AbckitGetMockInspectApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::g_inspectApiImpl;
}

#endif  // ABCKIT_INSPECT_IMPL_MOCK
