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
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"

#include "libabckit/src/macros.h"
#include "scoped_timer.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_SAME_TARGET(target1, target2)                  \
    if ((target1) != (target2)) {                                      \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET); \
        /* CC-OFFNXT(G.PRE.05) code generation */                      \
        return nullptr;                                                \
    }

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_SAME_TARGET_VOID(target1, target2)             \
    if ((target1) != (target2)) {                                      \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET); \
        /* CC-OFFNXT(G.PRE.05) code generation */                      \
        return;                                                        \
    }

namespace libabckit {

// ========================================
// File
// ========================================

extern "C" AbckitArktsModule *FileAddExternalModuleArktsV1(AbckitFile *file,
                                                           const struct AbckitArktsV1ExternalModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);

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

extern "C" bool ModuleSetName(AbckitArktsModule *m, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)m;
    (void)name;
    return false;
}

extern "C" AbckitArktsImportDescriptor *ModuleAddImportFromArktsV1ToArktsV1(
    AbckitArktsModule *importing, AbckitArktsModule *imported,
    const AbckitArktsImportFromDynamicModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(importing, nullptr);
    LIBABCKIT_BAD_ARGUMENT(imported, nullptr);

    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->alias, nullptr);

    LIBABCKIT_INTERNAL_ERROR(importing->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(imported->core, nullptr);

    if (importing->core->target != ABCKIT_TARGET_ARK_TS_V1) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }

    LIBABCKIT_CHECK_SAME_TARGET(importing->core->target, imported->core->target);

    return ModuleAddImportFromDynamicModuleDynamic(importing->core, imported->core, params);
}

extern "C" AbckitArktsImportDescriptor *ModuleAddImportFromArktsV2ToArktsV2(
    AbckitArktsModule * /*importing*/, AbckitArktsModule * /*imported*/,
    const AbckitArktsImportFromDynamicModuleCreateParams * /*params*/)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

extern "C" void ModuleRemoveImport(AbckitArktsModule *m, AbckitArktsImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(i)

    LIBABCKIT_INTERNAL_ERROR_VOID(m->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(i->core);

    auto mt = m->core->target;
    auto it = i->core->importingModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(mt, it);

    switch (mt) {
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
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(exporting, nullptr);
    LIBABCKIT_BAD_ARGUMENT(exported, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);

    LIBABCKIT_INTERNAL_ERROR(exporting->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(exported->core, nullptr);

    if (exporting->core->target != ABCKIT_TARGET_ARK_TS_V1) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }

    LIBABCKIT_CHECK_SAME_TARGET(exporting->core->target, exported->core->target);

    return DynamicModuleAddExportDynamic(exporting->core, exported->core, params);
}

extern "C" AbckitArktsExportDescriptor *ModuleAddExportFromArktsV2ToArktsV2(
    AbckitArktsModule * /*exporting*/ /* assert(exporting.target === AbckitTarget_ArkTS_v2) */,
    AbckitArktsModule * /*exported*/ /* assert(exporting.target === AbckitTarget_ArkTS_v2) */,
    const AbckitArktsDynamicModuleExportCreateParams * /*params*/)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

extern "C" void ModuleRemoveExport(AbckitArktsModule *m, AbckitArktsExportDescriptor *e)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(e)

    LIBABCKIT_INTERNAL_ERROR_VOID(m->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(e->core);

    auto mt = m->core->target;
    auto et = e->core->exportingModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(mt, et);

    switch (mt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleRemoveExportDynamic(m->core, e);
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
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);

    LIBABCKIT_INTERNAL_ERROR(m->core, nullptr);

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
// Namespace
// ========================================

extern "C" bool NamespaceSetName(AbckitArktsNamespace *ns, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)ns;
    (void)name;
    return false;
}

// ========================================
// Class
// ========================================

extern "C" AbckitArktsAnnotation *ClassAddAnnotation(AbckitArktsClass *klass,
                                                     const AbckitArktsAnnotationCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->ai, nullptr);

    LIBABCKIT_INTERNAL_ERROR(klass->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(params->ai->core, nullptr);

    auto kt = klass->core->owningModule->target;
    auto at = params->ai->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET(kt, at);

    switch (kt) {
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
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(klass);
    LIBABCKIT_BAD_ARGUMENT_VOID(anno);

    LIBABCKIT_INTERNAL_ERROR_VOID(klass->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(anno->core);

    auto kt = klass->core->owningModule->target;
    auto at = anno->core->ai->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(kt, at);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ClassRemoveAnnotationDynamic(klass->core, anno->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassAddInterface(AbckitArktsClass *klass, AbckitArktsInterface *iface)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)iface;
    return false;
}

extern "C" bool ClassRemoveInterface(AbckitArktsClass *klass, AbckitArktsInterface *iface)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)iface;
    return false;
}

extern "C" bool ClassSetSuperClass(AbckitArktsClass *klass, AbckitArktsClass *superClass)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)superClass;
    return false;
}

extern "C" bool ClassSetName(AbckitArktsClass *klass, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)name;
    return false;
}

extern "C" bool ClassAddField(AbckitArktsClass *klass, AbckitArktsClassField *field)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)field;
    return false;
}

extern "C" bool ClassRemoveField(AbckitArktsClass *klass, AbckitArktsClassField *field)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)field;
    return false;
}

extern "C" bool ClassAddMethod(AbckitArktsClass *klass, AbckitArktsFunction *method)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)method;
    return false;
}

extern "C" bool ClassRemoveMethod(AbckitArktsClass *klass, AbckitArktsFunction *method)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)method;
    return false;
}

extern "C" AbckitArktsClass *CreateClass(const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)name;
    return nullptr;
}

extern "C" bool ClassSetOwningModule(AbckitArktsClass *klass, AbckitArktsModule *module)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)module;
    return false;
}

extern "C" bool ClassSetParentFunction(AbckitArktsClass *klass, AbckitArktsFunction *func)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)func;
    return false;
}

// ========================================
// Interface
// ========================================

extern "C" bool InterfaceAddSuperInterface(AbckitArktsInterface *iface, AbckitArktsInterface *superIface)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)superIface;
    return false;
}

extern "C" bool InterfaceRemoveSuperInterface(AbckitArktsInterface *iface, AbckitArktsInterface *superIface)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)superIface;
    return false;
}

extern "C" bool InterfaceSetName(AbckitArktsInterface *iface, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)name;
    return false;
}

extern "C" bool InterfaceAddField(AbckitArktsInterface *iface, AbckitArktsInterfaceField *field)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)field;
    return false;
}

extern "C" bool InterfaceRemoveField(AbckitArktsInterface *iface, AbckitArktsInterfaceField *field)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)field;
    return false;
}

extern "C" bool InterfaceAddMethod(AbckitArktsInterface *iface, AbckitArktsFunction *method)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)method;
    return false;
}

extern "C" bool InterfaceRemoveMethod(AbckitArktsInterface *iface, AbckitArktsFunction *method)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)method;
    return false;
}

extern "C" bool InterfaceSetOwningModule(AbckitArktsInterface *iface, AbckitArktsModule *module)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)module;
    return false;
}

extern "C" bool InterfaceSetParentFunction(AbckitArktsInterface *iface, AbckitArktsFunction *func)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)func;
    return false;
}

extern "C" AbckitArktsInterface *CreateInterface(const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)name;
    return nullptr;
}

// ========================================
// Enum
// ========================================

extern "C" bool EnumSetName(AbckitArktsEnum *enm, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)enm;
    (void)name;
    return false;
}

// ========================================
// Module Field
// ========================================

extern "C" bool ModuleFieldAddAnnotation(AbckitArktsModuleField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)anno;
    return false;
}

extern "C" bool ModuleFieldRemoveAnnotation(AbckitArktsModuleField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)anno;
    return false;
}

extern "C" bool ModuleFieldSetName(AbckitArktsModuleField *field, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)name;
    return false;
}

extern "C" bool ModuleFieldSetType(AbckitArktsModuleField *field, AbckitType *type)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)type;
    return false;
}

extern "C" bool ModuleFieldSetValue(AbckitArktsModuleField *field, AbckitValue *value)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)value;
    return false;
}

extern "C" AbckitArktsModuleField *CreateModuleField(AbckitArktsModule *module, const char *name, AbckitType *type,
                                                     AbckitValue *value)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)module;
    (void)name;
    (void)type;
    (void)value;
    return nullptr;
}

// ========================================
// NamespaceField
// ========================================

extern "C" bool NamespaceFieldSetName(AbckitArktsNamespaceField *field, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)name;
    return false;
}

// ========================================
// Class Field
// ========================================

extern "C" bool ClassFieldAddAnnotation(AbckitArktsClassField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)anno;
    return false;
}

extern "C" bool ClassFieldRemoveAnnotation(AbckitArktsClassField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)anno;
    return false;
}

extern "C" bool ClassFieldSetName(AbckitArktsClassField *field, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)name;
    return false;
}

extern "C" bool ClassFieldSetType(AbckitArktsClassField *field, AbckitType *type)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)type;
    return false;
}

extern "C" bool ClassFieldSetValue(AbckitArktsClassField *field, AbckitValue *value)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)value;
    return false;
}

extern "C" AbckitArktsClassField *CreateClassField(AbckitArktsClass *klass, const char *name, AbckitType *type,
                                                   AbckitValue *value)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)klass;
    (void)name;
    (void)type;
    (void)value;
    return nullptr;
}

// ========================================
// Interface Field
// ========================================

extern "C" bool InterfaceFieldAddAnnotation(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)anno;
    return false;
}

extern "C" bool InterfaceFieldRemoveAnnotation(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)anno;
    return false;
}

extern "C" bool InterfaceFieldSetName(AbckitArktsInterfaceField *field, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)name;
    return false;
}

extern "C" bool InterfaceFieldSetType(AbckitArktsInterfaceField *field, AbckitType *type)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)type;
    return false;
}

extern "C" AbckitArktsInterfaceField *CreateInterfaceField(AbckitArktsInterface *iface, const char *name,
                                                           AbckitType *type, AbckitValue *value)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)iface;
    (void)name;
    (void)type;
    (void)value;
    return nullptr;
}

// ========================================
// Enum Field
// ========================================

extern "C" bool EnumFieldSetName(AbckitArktsEnumField *field, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)name;
    return false;
}

extern "C" bool EnumFieldSetType(AbckitArktsEnumField *field, AbckitType *type)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)type;
    return false;
}

extern "C" bool EnumFieldSetValue(AbckitArktsEnumField *field, AbckitValue *value)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)field;
    (void)value;
    return false;
}

extern "C" AbckitArktsEnumField *CreateEnumField(AbckitArktsEnum *enm, const char *name, AbckitType *type,
                                                 AbckitValue *value)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)enm;
    (void)name;
    (void)type;
    (void)value;
    return nullptr;
}

// ========================================
// AnnotationInterface
// ========================================

extern "C" bool AnnotationInterfaceSetName(AbckitArktsAnnotationInterface *ai, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)ai;
    (void)name;
    return false;
}

extern "C" AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddField(
    AbckitArktsAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ai, nullptr);

    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->type, nullptr);

    LIBABCKIT_INTERNAL_ERROR(ai->core, nullptr);

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
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(ai);
    LIBABCKIT_BAD_ARGUMENT_VOID(field);

    LIBABCKIT_INTERNAL_ERROR_VOID(ai->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(field->core);

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
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    LIBABCKIT_INTERNAL_ERROR(function->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(params->ai->core, nullptr);

    auto ft = function->core->owningModule->target;
    auto at = params->ai->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET(ft, at);

    switch (ft) {
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
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(function);
    LIBABCKIT_BAD_ARGUMENT_VOID(anno);

    LIBABCKIT_INTERNAL_ERROR_VOID(function->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(anno->core);

    auto ft = function->core->owningModule->target;
    auto at = anno->core->ai->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(ft, at);

    switch (ft) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionRemoveAnnotationDynamic(function->core, anno->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool FunctionSetOwningModule(AbckitArktsFunction *function, AbckitArktsModule *module)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)function;
    (void)module;
    return false;
}

extern "C" bool FunctionSetParentClass(AbckitArktsFunction *function, AbckitArktsClass *klass)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)function;
    (void)klass;
    return false;
}

extern "C" bool FunctionSetParentFunction(AbckitArktsFunction *function, AbckitArktsFunction *parentFunction)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)function;
    (void)parentFunction;
    return false;
}

extern "C" bool FunctionSetName(AbckitArktsFunction *function, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)function;
    (void)name;
    return false;
}

extern "C" bool FunctionAddParameter(AbckitArktsFunction *func, AbckitArktsFunctionParam *param)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)func;
    (void)param;
    return false;
}

extern "C" bool FunctionRemoveParameter(AbckitArktsFunction *func, AbckitArktsFunctionParam *param)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)func;
    (void)param;
    return false;
}

extern "C" bool FunctionSetReturnType(AbckitArktsFunction *func, AbckitType *type)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)func;
    (void)type;
    return false;
}

extern "C" AbckitArktsFunction *CreateFunction(const char *name, AbckitType *returnType)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)name;
    (void)returnType;
    return nullptr;
}

// ========================================
// Annotation
// ========================================

extern "C" bool AnnotationSetName(AbckitArktsAnnotation *anno, const char *name)
{
    LIBABCKIT_UNIMPLEMENTED;
    (void)anno;
    (void)name;
    return false;
}

extern "C" AbckitArktsAnnotationElement *AnnotationAddAnnotationElement(
    AbckitArktsAnnotation *anno, AbckitArktsAnnotationElementCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);

    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->value, nullptr);

    LIBABCKIT_INTERNAL_ERROR(anno->core, nullptr);
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
    LIBABCKIT_TIME_EXEC;

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

    ModuleSetName, ModuleAddImportFromArktsV1ToArktsV1, ModuleRemoveImport, ModuleAddExportFromArktsV1ToArktsV1,
    ModuleRemoveExport, ModuleAddAnnotationInterface,

    // ========================================
    // Namespace
    // ========================================

    NamespaceSetName,

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
    // Enum
    // ========================================

    EnumSetName,

    // ========================================
    // Module Field
    // ========================================

    ModuleFieldAddAnnotation, ModuleFieldRemoveAnnotation, ModuleFieldSetName, ModuleFieldSetType, ModuleFieldSetValue,
    CreateModuleField,

    // ========================================
    // Namespace Field
    // ========================================

    NamespaceFieldSetName,

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

    AnnotationInterfaceSetName, AnnotationInterfaceAddField, AnnotationInterfaceRemoveField,

    // ========================================
    // Function
    // ========================================

    FunctionAddAnnotation, FunctionRemoveAnnotation, FunctionSetOwningModule, FunctionSetParentClass,
    FunctionSetParentFunction, FunctionSetName, FunctionAddParameter, FunctionRemoveParameter, FunctionSetReturnType,
    CreateFunction,

    // ========================================
    // Annotation
    // ========================================

    AnnotationSetName, AnnotationAddAnnotationElement, AnnotationRemoveAnnotationElement,

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
