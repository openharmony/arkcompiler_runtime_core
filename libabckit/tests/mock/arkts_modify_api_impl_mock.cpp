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

#include "../../include/libabckit/c/extensions/arkts/metadata_arkts.h"

#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock::arkts_modify_api {

// NOLINTBEGIN(readability-identifier-naming)

AbckitArktsModule *FileAddExternalModuleArktsV1(AbckitFile *file,
                                                const struct AbckitArktsV1ExternalModuleCreateParams *params)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(params != nullptr);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_ARKTS_MODULE;
}

AbckitArktsImportDescriptor *ModuleAddImportFromArktsV1ToArktsV1(
    AbckitArktsModule *importing, AbckitArktsModule *imported,
    const struct AbckitArktsImportFromDynamicModuleCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(importing == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(imported == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    EXPECT_TRUE(strncmp(params->alias, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_ARKTS_IMPORT_DESCRIPTOR;
}

void ModuleRemoveImport(AbckitArktsModule *m, AbckitArktsImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(id == DEFAULT_ARKTS_IMPORT_DESCRIPTOR);
}

AbckitArktsExportDescriptor *ModuleAddExportFromArktsV1ToArktsV1(
    AbckitArktsModule *exporting, AbckitArktsModule *exported,
    const struct AbckitArktsDynamicModuleExportCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(exporting == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(exported == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    EXPECT_TRUE(strncmp(params->alias, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_ARKTS_EXPORT_DESCRIPTOR;
}

void ModuleRemoveExport(AbckitArktsModule *m, AbckitArktsExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(ed == DEFAULT_ARKTS_EXPORT_DESCRIPTOR);
}

AbckitArktsAnnotationInterface *ModuleAddAnnotationInterface(
    AbckitArktsModule *m, const struct AbckitArktsAnnotationInterfaceCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_ANNOTATION_INTERFACE;
}

AbckitArktsAnnotation *ClassAddAnnotation(AbckitArktsClass *klass,
                                          [[maybe_unused]] const struct AbckitArktsAnnotationCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    return DEFAULT_ANNOTATION;
}

void ClassRemoveAnnotation(AbckitArktsClass *klass, AbckitArktsAnnotation *anno)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
}

bool ClassAddInterface(AbckitArktsClass *klass, AbckitArktsInterface *interface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(interface == DEFAULT_ARKTS_INTERFACE);
    return DEFAULT_BOOL;
}

bool ClassRemoveInterface(AbckitArktsClass *klass, AbckitArktsInterface *interface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(interface == DEFAULT_ARKTS_INTERFACE);
    return DEFAULT_BOOL;
}

bool ClassSetSuperClass(AbckitArktsClass *klass, AbckitArktsClass *superClass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(superClass == DEFAULT_ARKTS_CLASS);
    return DEFAULT_BOOL;
}

bool ClassSetName(AbckitArktsClass *klass, const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_BOOL;
}

bool ClassAddField(AbckitArktsClass *klass, AbckitArktsClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassRemoveField(AbckitArktsClass *klass, AbckitArktsClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassAddMethod(AbckitArktsClass *klass, AbckitArktsFunction *method)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(method == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool ClassRemoveMethod(AbckitArktsClass *klass, AbckitArktsFunction *method)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(method == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

AbckitArktsClass *CreateClass(const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_ARKTS_CLASS;
}

bool ClassSetOwningModule(AbckitArktsClass *klass, AbckitArktsModule *module)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(module == DEFAULT_ARKTS_MODULE);
    return DEFAULT_BOOL;
}

bool ClassSetParentFunction(AbckitArktsClass *klass, AbckitArktsFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool InterfaceAddSuperInterface(AbckitArktsInterface *iface, AbckitArktsInterface *superInterface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(superInterface == DEFAULT_ARKTS_INTERFACE);
    return DEFAULT_BOOL;
}

bool InterfaceRemoveSuperInterface(AbckitArktsInterface *iface, AbckitArktsInterface *superInterface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(superInterface == DEFAULT_ARKTS_INTERFACE);
    return DEFAULT_BOOL;
}

bool InterfaceSetName(AbckitArktsInterface *iface, const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_BOOL;
}

bool InterfaceAddField(AbckitArktsInterface *iface, AbckitArktsInterfaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    return DEFAULT_BOOL;
}

bool InterfaceRemoveField(AbckitArktsInterface *iface, AbckitArktsInterfaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    return DEFAULT_BOOL;
}

bool InterfaceAddMethod(AbckitArktsInterface *iface, AbckitArktsFunction *method)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(method == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool InterfaceRemoveMethod(AbckitArktsInterface *iface, AbckitArktsFunction *method)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(method == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool InterfaceSetOwningModule(AbckitArktsInterface *iface, AbckitArktsModule *module)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(module == DEFAULT_ARKTS_MODULE);
    return DEFAULT_BOOL;
}

bool InterfaceSetParentFunction(AbckitArktsInterface *iface, AbckitArktsFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

AbckitArktsInterface *CreateInterface(const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_ARKTS_INTERFACE;
}

bool ModuleFieldAddAnnotation(AbckitArktsModuleField *field, AbckitArktsAnnotation *annotation)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_MODULE_FIELD);
    EXPECT_TRUE(annotation == DEFAULT_ANNOTATION);
    return DEFAULT_BOOL;
}

bool ModuleFieldRemoveAnnotation(AbckitArktsModuleField *field, AbckitArktsAnnotation *annotation)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_MODULE_FIELD);
    EXPECT_TRUE(annotation == DEFAULT_ANNOTATION);
    return DEFAULT_BOOL;
}

bool ModuleFieldSetName(AbckitArktsModuleField *field, const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_MODULE_FIELD);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_BOOL;
}

bool ModuleFieldSetType(AbckitArktsModuleField *field, AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_MODULE_FIELD);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_BOOL;
}

bool ModuleFieldSetValue(AbckitArktsModuleField *field, AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_MODULE_FIELD);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_BOOL;
}

AbckitArktsModuleField *CreateModuleField(AbckitArktsModule *module, const char *name, AbckitType *type,
                                          AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(module == DEFAULT_ARKTS_MODULE);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_ARKTS_MODULE_FIELD;
}

bool ClassFieldAddAnnotation(AbckitArktsClassField *field, AbckitArktsAnnotation *annotation)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    EXPECT_TRUE(annotation == DEFAULT_ANNOTATION);
    return DEFAULT_BOOL;
}

bool ClassFieldRemoveAnnotation(AbckitArktsClassField *field, AbckitArktsAnnotation *annotation)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    EXPECT_TRUE(annotation == DEFAULT_ANNOTATION);
    return DEFAULT_BOOL;
}

bool ClassFieldSetName(AbckitArktsClassField *field, const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_BOOL;
}

bool ClassFieldSetType(AbckitArktsClassField *field, AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_BOOL;
}

bool ClassFieldSetValue(AbckitArktsClassField *field, AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_BOOL;
}

AbckitArktsClassField *CreateClassField(AbckitArktsClass *klass, const char *name, AbckitType *type, AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_ARKTS_CLASS_FIELD;
}

bool InterfaceFieldAddAnnotation(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *annotation)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    EXPECT_TRUE(annotation == DEFAULT_ANNOTATION);
    return DEFAULT_BOOL;
}

bool InterfaceFieldRemoveAnnotation(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *annotation)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    EXPECT_TRUE(annotation == DEFAULT_ANNOTATION);
    return DEFAULT_BOOL;
}

bool InterfaceFieldSetName(AbckitArktsInterfaceField *field, const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_BOOL;
}

bool InterfaceFieldSetType(AbckitArktsInterfaceField *field, AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_BOOL;
}

AbckitArktsInterfaceField *CreateInterfaceField(AbckitArktsInterface *iface, const char *name, AbckitType *type,
                                                AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_ARKTS_INTERFACE_FIELD;
}

bool EnumFieldSetName(AbckitArktsEnumField *field, const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_ENUM_FIELD);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_BOOL;
}

bool EnumFieldSetType(AbckitArktsEnumField *field, AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_ENUM_FIELD);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_BOOL;
}

bool EnumFieldSetValue(AbckitArktsEnumField *field, AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_ENUM_FIELD);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_BOOL;
}

AbckitArktsEnumField *CreateEnumField(AbckitArktsEnum *enm, const char *name, AbckitType *type, AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_ARKTS_ENUM);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_ARKTS_ENUM_FIELD;
}

AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddField(
    AbckitArktsAnnotationInterface *ai, const struct AbckitArktsAnnotationInterfaceFieldCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ai == DEFAULT_ANNOTATION_INTERFACE);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    EXPECT_TRUE(params->type == DEFAULT_TYPE);
    return DEFAULT_ANNOTATION_INTERFACE_FIELD;
}

void AnnotationInterfaceRemoveField(AbckitArktsAnnotationInterface *ai, AbckitArktsAnnotationInterfaceField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ai == DEFAULT_ANNOTATION_INTERFACE);
    EXPECT_TRUE(field == DEFAULT_ANNOTATION_INTERFACE_FIELD);
}

AbckitArktsAnnotation *FunctionAddAnnotation(AbckitArktsFunction *function,
                                             [[maybe_unused]] const struct AbckitArktsAnnotationCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_ANNOTATION;
}

void FunctionRemoveAnnotation(AbckitArktsFunction *function, AbckitArktsAnnotation *anno)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
}

bool FunctionSetOwningModule(AbckitArktsFunction *func, AbckitArktsModule *module)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(func == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(module == DEFAULT_ARKTS_MODULE);
    return DEFAULT_BOOL;
}

bool FunctionSetParentClass(AbckitArktsFunction *func, AbckitArktsClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(func == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    return DEFAULT_BOOL;
}

bool FunctionSetParentFunction(AbckitArktsFunction *func, AbckitArktsFunction *parentFunction)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(func == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(parentFunction == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionSetName(AbckitArktsFunction *func, const char *name)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(func == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    return DEFAULT_BOOL;
}

bool FunctionAddParameter(AbckitArktsFunction *func, AbckitArktsFunctionParam *param)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(func == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(param == DEFAULT_ARKTS_FUNCTION_PARAM);
    return DEFAULT_BOOL;
}

bool FunctionRemoveParameter(AbckitArktsFunction *func, AbckitArktsFunctionParam *param)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(func == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(param == DEFAULT_ARKTS_FUNCTION_PARAM);
    return DEFAULT_BOOL;
}

bool FunctionSetReturnType(AbckitArktsFunction *func, AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(func == DEFAULT_ARKTS_FUNCTION);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_BOOL;
}

AbckitArktsFunction *CreateFunction(const char *name, AbckitType *returnType)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
    EXPECT_TRUE(returnType == DEFAULT_TYPE);
    return DEFAULT_ARKTS_FUNCTION;
}

AbckitArktsAnnotationElement *AnnotationAddAnnotationElement(AbckitArktsAnnotation *anno,
                                                             struct AbckitArktsAnnotationElementCreateParams *params)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_ANNOTATION_ELEMENT;
}

void AnnotationRemoveAnnotationElement(AbckitArktsAnnotation *anno, AbckitArktsAnnotationElement *elem)
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

    ClassAddAnnotation, ClassRemoveAnnotation, ClassAddInterface, ClassRemoveInterface, ClassSetSuperClass,
    ClassSetName, ClassAddField, ClassRemoveField, ClassAddMethod, ClassRemoveMethod, CreateClass, ClassSetOwningModule,
    ClassSetParentFunction,

    // ========================================
    // Interface
    // ========================================

    InterfaceAddSuperInterface, InterfaceRemoveSuperInterface, InterfaceSetName, InterfaceAddField,
    InterfaceRemoveField, InterfaceAddMethod, InterfaceRemoveMethod, InterfaceSetOwningModule,
    InterfaceSetParentFunction, CreateInterface,

    // ========================================
    // Module Field
    // ========================================

    ModuleFieldAddAnnotation, ModuleFieldRemoveAnnotation, ModuleFieldSetName, ModuleFieldSetType, ModuleFieldSetValue,
    CreateModuleField,

    // ========================================
    // Class Field
    // ========================================

    ClassFieldAddAnnotation, ClassFieldRemoveAnnotation, ClassFieldSetName, ClassFieldSetType, ClassFieldSetValue,
    CreateClassField,

    // ========================================
    // Interface Field
    // ========================================

    InterfaceFieldAddAnnotation, InterfaceFieldRemoveAnnotation, InterfaceFieldSetName, InterfaceFieldSetType,
    CreateInterfaceField,

    // ========================================
    // Enum Field
    // ========================================

    EnumFieldSetName, EnumFieldSetType, EnumFieldSetValue, CreateEnumField,

    // ========================================
    // AnnotationInterface
    // ========================================

    AnnotationInterfaceAddField, AnnotationInterfaceRemoveField,

    // ========================================
    // Function
    // ========================================

    FunctionAddAnnotation, FunctionRemoveAnnotation, FunctionSetOwningModule, FunctionSetParentClass,
    FunctionSetParentFunction, FunctionSetName, FunctionAddParameter, FunctionRemoveParameter, FunctionSetReturnType,
    CreateFunction,

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

}  // namespace libabckit::mock::arkts_modify_api

AbckitArktsModifyApi const *AbckitGetMockArktsModifyApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::arkts_modify_api::g_arktsModifyApiImpl;
}
