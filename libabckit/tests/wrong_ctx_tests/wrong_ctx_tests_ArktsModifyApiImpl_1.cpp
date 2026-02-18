/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

// Tests for INTERNAL_ERROR and deeper BAD_ARGUMENT branches
// in metadata_arkts_modify_impl.cpp

#include "libabckit/c/abckit.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/ir_core.h"

#include "helpers/helpers.h"
#include "metadata_inspect_impl.h"

#include <gtest/gtest.h>
#include <cstring>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_arktsModifyApi = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitWrongCtxTestsArktsModifyApiImpl1 : public ::testing::Test {};

// ========================================
// FileAddExternalModuleArktsV1 - deeper BAD_ARGUMENT
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->name, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FileAddExternalModuleArktsV1_ParamsNameNull)
{
    AbckitFile dummyFile = {};
    AbckitArktsV1ExternalModuleCreateParams params = {};
    params.name = nullptr;
    auto *res = g_arktsModifyApi->fileAddExternalModuleArktsV1(&dummyFile, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// ========================================
// ModuleAddImportFromArktsV1ToArktsV1 - deeper BAD_ARGUMENT + INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->name, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddImportV1_ParamsNameNull)
{
    AbckitArktsModule importing = {};
    AbckitArktsModule imported = {};
    AbckitArktsImportFromDynamicModuleCreateParams params = {};
    params.name = nullptr;
    params.alias = "alias";
    auto *res = g_arktsModifyApi->moduleAddImportFromArktsV1ToArktsV1(&importing, &imported, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_BAD_ARGUMENT(params->alias, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddImportV1_ParamsAliasNull)
{
    AbckitArktsModule importing = {};
    AbckitArktsModule imported = {};
    AbckitArktsImportFromDynamicModuleCreateParams params = {};
    params.name = "name";
    params.alias = nullptr;
    auto *res = g_arktsModifyApi->moduleAddImportFromArktsV1ToArktsV1(&importing, &imported, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(importing->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddImportV1_ImportingCoreNull)
{
    AbckitArktsModule importing = {};
    importing.core = nullptr;
    AbckitCoreModule importedCore = {};
    AbckitArktsModule imported = {};
    imported.core = &importedCore;
    AbckitArktsImportFromDynamicModuleCreateParams params = {};
    params.name = "name";
    params.alias = "alias";
    auto *res = g_arktsModifyApi->moduleAddImportFromArktsV1ToArktsV1(&importing, &imported, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(imported->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddImportV1_ImportedCoreNull)
{
    AbckitCoreModule importingCore = {};
    AbckitArktsModule importing = {};
    importing.core = &importingCore;
    AbckitArktsModule imported = {};
    imported.core = nullptr;
    AbckitArktsImportFromDynamicModuleCreateParams params = {};
    params.name = "name";
    params.alias = "alias";
    auto *res = g_arktsModifyApi->moduleAddImportFromArktsV1ToArktsV1(&importing, &imported, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ModuleAddExportFromArktsV1ToArktsV1 - deeper BAD_ARGUMENT + INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->name, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddExportV1_ParamsNameNull)
{
    AbckitArktsModule exporting = {};
    AbckitArktsModule exported = {};
    AbckitArktsDynamicModuleExportCreateParams params = {};
    params.name = nullptr;
    auto *res = g_arktsModifyApi->moduleAddExportFromArktsV1ToArktsV1(&exporting, &exported, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(exporting->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddExportV1_ExportingCoreNull)
{
    AbckitArktsModule exporting = {};
    exporting.core = nullptr;
    AbckitCoreModule exportedCore = {};
    AbckitArktsModule exported = {};
    exported.core = &exportedCore;
    AbckitArktsDynamicModuleExportCreateParams params = {};
    params.name = "name";
    auto *res = g_arktsModifyApi->moduleAddExportFromArktsV1ToArktsV1(&exporting, &exported, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(exported->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddExportV1_ExportedCoreNull)
{
    AbckitCoreModule exportingCore = {};
    AbckitArktsModule exporting = {};
    exporting.core = &exportingCore;
    AbckitArktsModule exported = {};
    exported.core = nullptr;
    AbckitArktsDynamicModuleExportCreateParams params = {};
    params.name = "name";
    auto *res = g_arktsModifyApi->moduleAddExportFromArktsV1ToArktsV1(&exporting, &exported, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ModuleRemoveImport - INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_INTERNAL_ERROR_VOID(m->core)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleRemoveImport_MCoreNull)
{
    AbckitArktsModule m = {};
    m.core = nullptr;
    AbckitCoreImportDescriptor iCore = {};
    AbckitArktsImportDescriptor i = {};
    i.core = &iCore;
    g_arktsModifyApi->moduleRemoveImport(&m, &i);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// Cover: LIBABCKIT_INTERNAL_ERROR_VOID(i->core)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleRemoveImport_ICoreNull)
{
    AbckitCoreModule mCore = {};
    AbckitArktsModule m = {};
    m.core = &mCore;
    AbckitArktsImportDescriptor i = {};
    i.core = nullptr;
    g_arktsModifyApi->moduleRemoveImport(&m, &i);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ModuleRemoveExport - INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_INTERNAL_ERROR_VOID(m->core)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleRemoveExport_MCoreNull)
{
    AbckitArktsModule m = {};
    m.core = nullptr;
    AbckitCoreExportDescriptor eCore = {};
    AbckitArktsExportDescriptor e = {};
    e.core = &eCore;
    g_arktsModifyApi->moduleRemoveExport(&m, &e);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// Cover: LIBABCKIT_INTERNAL_ERROR_VOID(e->core)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleRemoveExport_ECoreNull)
{
    AbckitCoreModule mCore = {};
    AbckitArktsModule m = {};
    m.core = &mCore;
    AbckitArktsExportDescriptor e = {};
    e.core = nullptr;
    g_arktsModifyApi->moduleRemoveExport(&m, &e);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ModuleAddAnnotationInterface - deeper BAD_ARGUMENT + INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->name, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddAnnotationInterface_ParamsNameNull)
{
    AbckitArktsModule m = {};
    AbckitArktsAnnotationInterfaceCreateParams params = {};
    params.name = nullptr;
    auto *res = g_arktsModifyApi->moduleAddAnnotationInterface(&m, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(m->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddAnnotationInterface_MCoreNull)
{
    AbckitArktsModule m = {};
    m.core = nullptr;
    AbckitArktsAnnotationInterfaceCreateParams params = {};
    params.name = "name";
    auto *res = g_arktsModifyApi->moduleAddAnnotationInterface(&m, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ModuleAddField - INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_INTERNAL_ERROR(m->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleAddField_MCoreNull)
{
    AbckitArktsModule m = {};
    m.core = nullptr;
    AbckitArktsFieldCreateParams params = {};
    auto *res = g_arktsModifyApi->moduleAddField(&m, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassAddAnnotation - deeper BAD_ARGUMENT + INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->ai, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassAddAnnotation_ParamsAiNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    AbckitArktsAnnotationCreateParams params = {};
    params.ai = nullptr;
    auto *res = g_arktsModifyApi->classAddAnnotation(&klass, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(klass->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassAddAnnotation_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitArktsAnnotationInterface ai = {};
    AbckitArktsAnnotationCreateParams params = {};
    params.ai = &ai;
    auto *res = g_arktsModifyApi->classAddAnnotation(&klass, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(params->ai->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassAddAnnotation_AiCoreNull)
{
    AbckitCoreClass klassCore = {};
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsAnnotationInterface ai = {};
    ai.core = nullptr;
    AbckitArktsAnnotationCreateParams params = {};
    params.ai = &ai;
    auto *res = g_arktsModifyApi->classAddAnnotation(&klass, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassRemoveAnnotation - INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_INTERNAL_ERROR_VOID(klass->core)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveAnnotation_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitCoreAnnotation annoCore = {};
    AbckitArktsAnnotation anno = {};
    anno.core = &annoCore;
    g_arktsModifyApi->classRemoveAnnotation(&klass, &anno);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// Cover: LIBABCKIT_INTERNAL_ERROR_VOID(anno->core)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveAnnotation_AnnoCoreNull)
{
    AbckitCoreClass klassCore = {};
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsAnnotation anno = {};
    anno.core = nullptr;
    g_arktsModifyApi->classRemoveAnnotation(&klass, &anno);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassAddInterface - INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_INTERNAL_ERROR(klass->core, false)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassAddInterface_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitCoreInterface ifaceCore = {};
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    bool res = g_arktsModifyApi->classAddInterface(&klass, &iface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(iface->core, false)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassAddInterface_IfaceCoreNull)
{
    AbckitCoreClass klassCore = {};
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    bool res = g_arktsModifyApi->classAddInterface(&klass, &iface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassRemoveInterface - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveInterface_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitCoreInterface ifaceCore = {};
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    bool res = g_arktsModifyApi->classRemoveInterface(&klass, &iface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveInterface_IfaceCoreNull)
{
    AbckitCoreClass klassCore = {};
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    bool res = g_arktsModifyApi->classRemoveInterface(&klass, &iface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassSetSuperClass - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassSetSuperClass_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitCoreClass superCore = {};
    AbckitArktsClass superClass = {};
    superClass.core = &superCore;
    bool res = g_arktsModifyApi->classSetSuperClass(&klass, &superClass);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassSetSuperClass_SuperCoreNull)
{
    AbckitCoreClass klassCore = {};
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsClass superClass = {};
    superClass.core = nullptr;
    bool res = g_arktsModifyApi->classSetSuperClass(&klass, &superClass);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassAddField - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassAddField_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitArktsFieldCreateParams params = {};
    auto *res = g_arktsModifyApi->classAddField(&klass, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassRemoveField - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveField_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitCoreClassField fieldCore = {};
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &fieldCore;
    bool res = g_arktsModifyApi->classRemoveField(&klass, &field);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveField_FieldCoreNull)
{
    AbckitCoreClass klassCore = {};
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = nullptr;
    bool res = g_arktsModifyApi->classRemoveField(&klass, &field);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassRemoveMethod - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveMethod_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitCoreFunction methodCore = {};
    AbckitArktsFunction method = {};
    method.core = &methodCore;
    bool res = g_arktsModifyApi->classRemoveMethod(&klass, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassRemoveMethod_MethodCoreNull)
{
    AbckitCoreClass klassCore = {};
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsFunction method = {};
    method.core = nullptr;
    bool res = g_arktsModifyApi->classRemoveMethod(&klass, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// CreateClass - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, CreateClass_MCoreNull)
{
    AbckitArktsModule m = {};
    m.core = nullptr;
    auto *res = g_arktsModifyApi->createClass(&m, "ClassName");
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassSetOwningModule - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassSetOwningModule_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    AbckitCoreModule modCore = {};
    AbckitArktsModule module = {};
    module.core = &modCore;
    bool res = g_arktsModifyApi->classSetOwningModule(&klass, &module);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceAddSuperInterface - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceAddSuperInterface_IfaceCoreNull)
{
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    AbckitCoreInterface superCore = {};
    AbckitArktsInterface superIface = {};
    superIface.core = &superCore;
    bool res = g_arktsModifyApi->interfaceAddSuperInterface(&iface, &superIface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceRemoveSuperInterface - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceRemoveSuperInterface_IfaceCoreNull)
{
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    AbckitCoreInterface superCore = {};
    AbckitArktsInterface superIface = {};
    superIface.core = &superCore;
    bool res = g_arktsModifyApi->interfaceRemoveSuperInterface(&iface, &superIface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceAddField - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceAddField_IfaceCoreNull)
{
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    AbckitArktsInterfaceFieldCreateParams params = {};
    auto *res = g_arktsModifyApi->interfaceAddField(&iface, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceRemoveField - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceRemoveField_IfaceCoreNull)
{
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    AbckitCoreInterfaceField fieldCore = {};
    AbckitArktsInterfaceField field;
    field.core = &fieldCore;
    bool res = g_arktsModifyApi->interfaceRemoveField(&iface, &field);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceRemoveField_FieldCoreNull)
{
    AbckitCoreInterface ifaceCore = {};
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitArktsInterfaceField field;
    field.core = nullptr;
    bool res = g_arktsModifyApi->interfaceRemoveField(&iface, &field);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceAddMethod - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceAddMethod_IfaceCoreNull)
{
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    AbckitCoreFunction methodCore = {};
    AbckitArktsFunction method = {};
    method.core = &methodCore;
    bool res = g_arktsModifyApi->interfaceAddMethod(&iface, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceAddMethod_MethodCoreNull)
{
    AbckitCoreInterface ifaceCore = {};
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitArktsFunction method = {};
    method.core = nullptr;
    bool res = g_arktsModifyApi->interfaceAddMethod(&iface, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceRemoveMethod - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceRemoveMethod_IfaceCoreNull)
{
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    AbckitCoreFunction methodCore = {};
    AbckitArktsFunction method = {};
    method.core = &methodCore;
    bool res = g_arktsModifyApi->interfaceRemoveMethod(&iface, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceSetOwningModule - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceSetOwningModule_IfaceCoreNull)
{
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = nullptr;
    AbckitCoreModule modCore = {};
    AbckitArktsModule module = {};
    module.core = &modCore;
    bool res = g_arktsModifyApi->interfaceSetOwningModule(&iface, &module);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// CreateInterface - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, CreateInterface_MCoreNull)
{
    AbckitArktsModule m = {};
    m.core = nullptr;
    auto *res = g_arktsModifyApi->createInterface(&m, "InterfaceName");
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// EnumAddField - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, EnumAddField_EnmCoreNull)
{
    AbckitArktsEnum enm(static_cast<ark::pandasm::Record *>(nullptr));
    enm.core = nullptr;
    AbckitArktsFieldCreateParams params = {};
    auto *res = g_arktsModifyApi->enumAddField(&enm, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ModuleFieldSetType - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleFieldSetType_FieldCoreNull)
{
    AbckitArktsModuleField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    AbckitType *type = (AbckitType *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    bool res = g_arktsModifyApi->moduleFieldSetType(&field, type);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ModuleFieldSetValue - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ModuleFieldSetValue_FieldCoreNull)
{
    AbckitArktsModuleField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    AbckitValue *value = (AbckitValue *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    bool res = g_arktsModifyApi->moduleFieldSetValue(&field, value);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassFieldAddAnnotation - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassFieldAddAnnotation_FieldCoreNull)
{
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = nullptr;
    AbckitArktsAnnotationCreateParams params = {};
    bool res = g_arktsModifyApi->classFieldAddAnnotation(&field, &params);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassFieldRemoveAnnotation - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassFieldRemoveAnnotation_FieldCoreNull)
{
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = nullptr;
    AbckitCoreAnnotation annoCore = {};
    AbckitArktsAnnotation anno = {};
    anno.core = &annoCore;
    bool res = g_arktsModifyApi->classFieldRemoveAnnotation(&field, &anno);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassFieldSetType - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassFieldSetType_FieldCoreNull)
{
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    AbckitType *type = (AbckitType *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    bool res = g_arktsModifyApi->classFieldSetType(&field, type);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassFieldSetValue - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassFieldSetValue_FieldCoreNull)
{
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    AbckitValue *value = (AbckitValue *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    bool res = g_arktsModifyApi->classFieldSetValue(&field, value);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceFieldAddAnnotation - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceFieldAddAnnotation_FieldCoreNull)
{
    AbckitArktsInterfaceField field;
    field.core = nullptr;
    AbckitArktsAnnotationCreateParams params = {};
    bool res = g_arktsModifyApi->interfaceFieldAddAnnotation(&field, &params);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceFieldRemoveAnnotation - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceFieldRemoveAnnotation_FieldCoreNull)
{
    AbckitArktsInterfaceField field;
    field.core = nullptr;
    AbckitCoreAnnotation annoCore = {};
    AbckitArktsAnnotation anno = {};
    anno.core = &annoCore;
    bool res = g_arktsModifyApi->interfaceFieldRemoveAnnotation(&field, &anno);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// InterfaceFieldSetType - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, InterfaceFieldSetType_FieldCoreNull)
{
    AbckitArktsInterfaceField field;
    field.core = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    AbckitType *type = (AbckitType *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    bool res = g_arktsModifyApi->interfaceFieldSetType(&field, type);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// AnnotationInterfaceAddField - deeper BAD_ARGUMENT + INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->name, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationInterfaceAddField_ParamsNameNull)
{
    AbckitArktsAnnotationInterface ai = {};
    AbckitArktsAnnotationInterfaceFieldCreateParams params = {};
    params.name = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    params.type = (AbckitType *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    auto *res = g_arktsModifyApi->annotationInterfaceAddField(&ai, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_BAD_ARGUMENT(params->type, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationInterfaceAddField_ParamsTypeNull)
{
    AbckitArktsAnnotationInterface ai = {};
    AbckitArktsAnnotationInterfaceFieldCreateParams params = {};
    params.name = "name";
    params.type = nullptr;
    auto *res = g_arktsModifyApi->annotationInterfaceAddField(&ai, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(ai->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationInterfaceAddField_AiCoreNull)
{
    AbckitArktsAnnotationInterface ai = {};
    ai.core = nullptr;
    AbckitArktsAnnotationInterfaceFieldCreateParams params = {};
    params.name = "name";
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    params.type = (AbckitType *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    auto *res = g_arktsModifyApi->annotationInterfaceAddField(&ai, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// AnnotationInterfaceRemoveField - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationInterfaceRemoveField_AiCoreNull)
{
    AbckitArktsAnnotationInterface ai = {};
    ai.core = nullptr;
    AbckitCoreAnnotationInterfaceField fieldCore = {};
    AbckitArktsAnnotationInterfaceField field = {};
    field.core = &fieldCore;
    g_arktsModifyApi->annotationInterfaceRemoveField(&ai, &field);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationInterfaceRemoveField_FieldCoreNull)
{
    AbckitCoreAnnotationInterface aiCore = {};
    AbckitArktsAnnotationInterface ai = {};
    ai.core = &aiCore;
    AbckitArktsAnnotationInterfaceField field = {};
    field.core = nullptr;
    g_arktsModifyApi->annotationInterfaceRemoveField(&ai, &field);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// FunctionAddAnnotation - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionAddAnnotation_FuncCoreNull)
{
    AbckitArktsFunction func = {};
    func.core = nullptr;
    AbckitArktsAnnotationCreateParams params = {};
    auto *res = g_arktsModifyApi->functionAddAnnotation(&func, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// FunctionRemoveAnnotation - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionRemoveAnnotation_FuncCoreNull)
{
    AbckitArktsFunction func = {};
    func.core = nullptr;
    AbckitCoreAnnotation annoCore = {};
    AbckitArktsAnnotation anno = {};
    anno.core = &annoCore;
    g_arktsModifyApi->functionRemoveAnnotation(&func, &anno);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionRemoveAnnotation_AnnoCoreNull)
{
    AbckitCoreFunction funcCore = {};
    AbckitArktsFunction func = {};
    func.core = &funcCore;
    AbckitArktsAnnotation anno = {};
    anno.core = nullptr;
    g_arktsModifyApi->functionRemoveAnnotation(&func, &anno);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// FunctionSetName - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionSetName_FuncCoreNull)
{
    AbckitArktsFunction func = {};
    func.core = nullptr;
    bool res = g_arktsModifyApi->functionSetName(&func, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// FunctionAddParameter - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionAddParameter_FuncCoreNull)
{
    AbckitArktsFunction func = {};
    func.core = nullptr;
    AbckitCoreFunctionParam paramCore = {};
    AbckitArktsFunctionParam param = {};
    param.core = &paramCore;
    bool res = g_arktsModifyApi->functionAddParameter(&func, &param);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionAddParameter_ParamCoreNull)
{
    AbckitCoreFunction funcCore = {};
    AbckitArktsFunction func = {};
    func.core = &funcCore;
    AbckitArktsFunctionParam param = {};
    param.core = nullptr;
    bool res = g_arktsModifyApi->functionAddParameter(&func, &param);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// FunctionRemoveParameter - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionRemoveParameter_FuncCoreNull)
{
    AbckitArktsFunction func = {};
    func.core = nullptr;
    bool res = g_arktsModifyApi->functionRemoveParameter(&func, 0);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// FunctionSetReturnType - INTERNAL_ERROR
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, FunctionSetReturnType_FuncCoreNull)
{
    AbckitArktsFunction func = {};
    func.core = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    AbckitType *type = (AbckitType *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    bool res = g_arktsModifyApi->functionSetReturnType(&func, type);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// ClassAddMethod - INTERNAL_ERROR (also covers the completely uncovered function)
// ========================================

TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, ClassAddMethod_KlassCoreNull)
{
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = nullptr;
    ArktsMethodCreateParams params = {};
    auto *res = g_arktsModifyApi->classAddMethod(&klass, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// AnnotationAddAnnotationElement - deeper BAD_ARGUMENT + INTERNAL_ERROR
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->name, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationAddAnnotationElement_ParamsNameNull)
{
    AbckitArktsAnnotation anno = {};
    AbckitArktsAnnotationElementCreateParams params = {};
    params.name = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    params.value = (AbckitValue *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    auto *res = g_arktsModifyApi->annotationAddAnnotationElement(&anno, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_BAD_ARGUMENT(params->value, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationAddAnnotationElement_ParamsValueNull)
{
    AbckitArktsAnnotation anno = {};
    AbckitArktsAnnotationElementCreateParams params = {};
    params.name = "name";
    params.value = nullptr;
    auto *res = g_arktsModifyApi->annotationAddAnnotationElement(&anno, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_INTERNAL_ERROR(anno->core, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, AnnotationAddAnnotationElement_AnnoCoreNull)
{
    AbckitArktsAnnotation anno = {};
    anno.core = nullptr;
    AbckitArktsAnnotationElementCreateParams params = {};
    params.name = "name";
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    params.value = (AbckitValue *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    auto *res = g_arktsModifyApi->annotationAddAnnotationElement(&anno, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}

// ========================================
// CreateFunctionParameter - deeper BAD_ARGUMENT
// ========================================

// Cover: LIBABCKIT_BAD_ARGUMENT(params->name, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, CreateFunctionParameter_ParamsNameNull)
{
    AbckitFile dummyFile = {};
    AbckitArktsFunctionParamCreatedParams params = {};
    params.name = nullptr;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
    params.type = (AbckitType *)(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
    auto *res = g_arktsModifyApi->createFunctionParameter(&dummyFile, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

// Cover: LIBABCKIT_BAD_ARGUMENT(params->type, nullptr)
TEST_F(LibAbcKitWrongCtxTestsArktsModifyApiImpl1, CreateFunctionParameter_ParamsTypeNull)
{
    AbckitFile dummyFile = {};
    AbckitArktsFunctionParamCreatedParams params = {};
    params.name = "name";
    params.type = nullptr;
    auto *res = g_arktsModifyApi->createFunctionParameter(&dummyFile, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

}  // namespace libabckit::test
