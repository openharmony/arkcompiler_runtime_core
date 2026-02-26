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

// Tests for wrong target/mode branches in metadata_arkts_modify_impl.cpp
// Covers: IsDynamic() branches, wrong ABCKIT_TARGET switch cases,
// WRONG_TARGET checks, and completely uncovered functions.

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

class LibAbcKitWrongModeTestsArktsModifyApiImpl0 : public ::testing::Test {};

// ========================================
// File
// ========================================

// Cover: FileAddExternalModuleArktsV1 -> case Mode::STATIC: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, FileAddExternalModuleArktsV1_StaticFile)
{
    AbckitFile dummyFile = {};
    dummyFile.frontend = libabckit::Mode::STATIC;
    AbckitArktsV1ExternalModuleCreateParams params = {};
    params.name = "testModule";
    auto *res = g_arktsModifyApi->fileAddExternalModuleArktsV1(&dummyFile, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: FileAddExternalModuleArktsV2 -> case Mode::DYNAMIC: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, FileAddExternalModuleArktsV2_DynamicFile)
{
    AbckitFile dummyFile = {};
    dummyFile.frontend = libabckit::Mode::DYNAMIC;
    auto *res = g_arktsModifyApi->fileAddExternalModuleArktsV2(&dummyFile, "testModule");
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Module
// ========================================

// Cover: ModuleSetName -> if (IsDynamic(m->core->target)) { UNSUPPORTED }
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitArktsModule m = {};
    m.core = &dynamicModule;

    bool res = g_arktsModifyApi->moduleSetName(&m, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ModuleImportStaticFunctionFromArktsV2ToArktsV2 -> WRONG_TARGET
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleImportStaticFunction_WrongTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitArktsModule m = {};
    m.core = &dynamicModule;

    const char *params[] = {"int"};
    auto *res = g_arktsModifyApi->moduleImportStaticFunctionFromArktsV2ToArktsV2(&m, "funcName", "void", params, 1);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}

// Cover: ModuleImportClassMethodFromArktsV2ToArktsV2 -> WRONG_TARGET
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleImportClassMethod_WrongTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitArktsModule m = {};
    m.core = &dynamicModule;

    const char *params[] = {"int"};
    auto *res =
        g_arktsModifyApi->moduleImportClassMethodFromArktsV2ToArktsV2(&m, "ClassName", "methodName", "void", params, 1);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}

// Cover: ModuleImportClassFromArktsV2ToArktsV2 -> WRONG_TARGET
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleImportClass_WrongTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitArktsModule m = {};
    m.core = &dynamicModule;

    auto *res = g_arktsModifyApi->moduleImportClassFromArktsV2ToArktsV2(&m, "ClassName");
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}

// Cover: ModuleRemoveImport -> case ABCKIT_TARGET_ARK_TS_V2: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleRemoveImport_V2Target)
{
    AbckitCoreModule modCore = {};
    modCore.target = ABCKIT_TARGET_ARK_TS_V2;
    AbckitArktsModule m = {};
    m.core = &modCore;

    AbckitCoreModule importingModCore = {};
    importingModCore.target = ABCKIT_TARGET_ARK_TS_V2;
    AbckitCoreImportDescriptor iCore = {};
    iCore.importingModule = &importingModCore;
    AbckitArktsImportDescriptor i = {};
    i.core = &iCore;

    g_arktsModifyApi->moduleRemoveImport(&m, &i);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ModuleRemoveExport -> case ABCKIT_TARGET_ARK_TS_V2: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleRemoveExport_V2Target)
{
    AbckitCoreModule modCore = {};
    modCore.target = ABCKIT_TARGET_ARK_TS_V2;
    AbckitArktsModule m = {};
    m.core = &modCore;

    AbckitCoreModule exportingModCore = {};
    exportingModCore.target = ABCKIT_TARGET_ARK_TS_V2;
    AbckitCoreExportDescriptor eCore = {};
    eCore.exportingModule = &exportingModCore;
    AbckitArktsExportDescriptor e = {};
    e.core = &eCore;

    g_arktsModifyApi->moduleRemoveExport(&m, &e);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ModuleAddField -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleAddField_V1Target)
{
    AbckitCoreModule modCore = {};
    modCore.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitArktsModule m = {};
    m.core = &modCore;
    AbckitArktsFieldCreateParams params = {};

    auto *res = g_arktsModifyApi->moduleAddField(&m, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Namespace
// ========================================

// Cover: NamespaceSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, NamespaceSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreNamespace ns(&dynamicModule);
    AbckitArktsNamespace arktsNs = {};
    arktsNs.core = &ns;

    bool res = g_arktsModifyApi->namespaceSetName(&arktsNs, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Class
// ========================================

// Cover: ClassSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;

    bool res = g_arktsModifyApi->classSetName(&klass, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassAddInterface -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassAddInterface_V1Target)
{
    AbckitFile dummyFile = {};
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    dynamicModule.file = &dummyFile;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;

    bool res = g_arktsModifyApi->classAddInterface(&klass, &iface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassRemoveInterface -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassRemoveInterface_V1Target)
{
    AbckitFile dummyFile = {};
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    dynamicModule.file = &dummyFile;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;

    bool res = g_arktsModifyApi->classRemoveInterface(&klass, &iface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassSetSuperClass -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassSetSuperClass_V1Target)
{
    AbckitFile dummyFile = {};
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    dynamicModule.file = &dummyFile;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitCoreClass superCore = {};
    superCore.owningModule = &dynamicModule;
    AbckitArktsClass superClass(static_cast<ark::pandasm::Record *>(nullptr));
    superClass.core = &superCore;

    bool res = g_arktsModifyApi->classSetSuperClass(&klass, &superClass);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassAddField -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassAddField_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsFieldCreateParams params = {};

    auto *res = g_arktsModifyApi->classAddField(&klass, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassRemoveField -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassRemoveField_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitCoreClass fieldOwnerClass = {};
    fieldOwnerClass.owningModule = &dynamicModule;
    AbckitCoreClassField cfieldCore = {};
    cfieldCore.owner = &fieldOwnerClass;
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &cfieldCore;

    bool res = g_arktsModifyApi->classRemoveField(&klass, &field);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassRemoveMethod -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassRemoveMethod_V1Target)
{
    AbckitFile dummyFile = {};
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    dynamicModule.file = &dummyFile;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitCoreFunction funcCore = {};
    funcCore.owningModule = &dynamicModule;
    AbckitArktsFunction method = {};
    method.core = &funcCore;

    bool res = g_arktsModifyApi->classRemoveMethod(&klass, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: CreateClass -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, CreateClass_V1Target)
{
    AbckitFile dummyFile = {};
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    dynamicModule.file = &dummyFile;
    AbckitArktsModule m = {};
    m.core = &dynamicModule;

    auto *res = g_arktsModifyApi->createClass(&m, "ClassName");
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassSetOwningModule -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassSetOwningModule_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    AbckitArktsModule module = {};
    module.core = &dynamicModule;

    bool res = g_arktsModifyApi->classSetOwningModule(&klass, &module);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassAddMethod -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassAddMethod_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitArktsClass klass(static_cast<ark::pandasm::Record *>(nullptr));
    klass.core = &klassCore;
    ArktsMethodCreateParams params = {};

    auto *res = g_arktsModifyApi->classAddMethod(&klass, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Interface
// ========================================

// Cover: InterfaceSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;

    bool res = g_arktsModifyApi->interfaceSetName(&iface, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceAddField -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceAddField_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitArktsInterfaceFieldCreateParams params = {};

    auto *res = g_arktsModifyApi->interfaceAddField(&iface, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceRemoveField -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceRemoveField_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitCoreInterface fieldOwner = {};
    fieldOwner.owningModule = &dynamicModule;
    AbckitCoreInterfaceField ifieldCore = {};
    ifieldCore.owner = &fieldOwner;
    AbckitArktsInterfaceField field;
    field.core = &ifieldCore;

    bool res = g_arktsModifyApi->interfaceRemoveField(&iface, &field);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceAddMethod -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceAddMethod_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitCoreFunction funcCore = {};
    funcCore.owningModule = &dynamicModule;
    AbckitArktsFunction method = {};
    method.core = &funcCore;

    bool res = g_arktsModifyApi->interfaceAddMethod(&iface, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceRemoveMethod -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceRemoveMethod_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitCoreFunction funcCore = {};
    funcCore.owningModule = &dynamicModule;
    AbckitArktsFunction method = {};
    method.core = &funcCore;

    bool res = g_arktsModifyApi->interfaceRemoveMethod(&iface, &method);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceSetOwningModule -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceSetOwningModule_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitArktsModule module = {};
    module.core = &dynamicModule;

    bool res = g_arktsModifyApi->interfaceSetOwningModule(&iface, &module);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceAddSuperInterface -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceAddSuperInterface_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitCoreInterface superCore = {};
    superCore.owningModule = &dynamicModule;
    AbckitArktsInterface superIface(static_cast<ark::pandasm::Record *>(nullptr));
    superIface.core = &superCore;

    bool res = g_arktsModifyApi->interfaceAddSuperInterface(&iface, &superIface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceRemoveSuperInterface -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceRemoveSuperInterface_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitArktsInterface iface(static_cast<ark::pandasm::Record *>(nullptr));
    iface.core = &ifaceCore;
    AbckitCoreInterface superCore = {};
    superCore.owningModule = &dynamicModule;
    AbckitArktsInterface superIface(static_cast<ark::pandasm::Record *>(nullptr));
    superIface.core = &superCore;

    bool res = g_arktsModifyApi->interfaceRemoveSuperInterface(&iface, &superIface);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: CreateInterface -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, CreateInterface_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitArktsModule m = {};
    m.core = &dynamicModule;

    auto *res = g_arktsModifyApi->createInterface(&m, "InterfaceName");
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Enum
// ========================================

// Cover: EnumSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, EnumSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreEnum enmCore = {};
    enmCore.owningModule = &dynamicModule;
    AbckitArktsEnum enm(static_cast<ark::pandasm::Record *>(nullptr));
    enm.core = &enmCore;

    bool res = g_arktsModifyApi->enumSetName(&enm, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: EnumAddField -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, EnumAddField_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreEnum enmCore = {};
    enmCore.owningModule = &dynamicModule;
    AbckitArktsEnum enm(static_cast<ark::pandasm::Record *>(nullptr));
    enm.core = &enmCore;
    AbckitArktsFieldCreateParams params = {};

    auto *res = g_arktsModifyApi->enumAddField(&enm, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Module Field
// ========================================

// Cover: ModuleFieldSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleFieldSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreModuleField fieldCore = {};
    fieldCore.owner = &dynamicModule;
    AbckitArktsModuleField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &fieldCore;

    bool res = g_arktsModifyApi->moduleFieldSetName(&field, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ModuleFieldSetType -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleFieldSetType_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreModuleField fieldCore = {};
    fieldCore.owner = &dynamicModule;
    AbckitArktsModuleField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &fieldCore;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto *type = reinterpret_cast<AbckitType *>(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    bool res = g_arktsModifyApi->moduleFieldSetType(&field, type);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ModuleFieldSetValue -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleFieldSetValue_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreModuleField fieldCore = {};
    fieldCore.owner = &dynamicModule;
    AbckitArktsModuleField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &fieldCore;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto *value = reinterpret_cast<AbckitValue *>(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    bool res = g_arktsModifyApi->moduleFieldSetValue(&field, value);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Namespace Field
// ========================================

// Cover: NamespaceFieldSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, NamespaceFieldSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreNamespace ns(&dynamicModule);
    AbckitCoreNamespaceField nsFieldCore = {};
    nsFieldCore.owner = &ns;
    AbckitArktsNamespaceField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &nsFieldCore;

    bool res = g_arktsModifyApi->namespaceFieldSetName(&field, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Class Field
// ========================================

// Cover: ClassFieldSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassFieldSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitCoreClassField cfieldCore = {};
    cfieldCore.owner = &klassCore;
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &cfieldCore;

    bool res = g_arktsModifyApi->classFieldSetName(&field, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassFieldSetType -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassFieldSetType_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitCoreClassField cfieldCore = {};
    cfieldCore.owner = &klassCore;
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &cfieldCore;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto *type = reinterpret_cast<AbckitType *>(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    bool res = g_arktsModifyApi->classFieldSetType(&field, type);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassFieldSetValue -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassFieldSetValue_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitCoreClassField cfieldCore = {};
    cfieldCore.owner = &klassCore;
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &cfieldCore;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto *value = reinterpret_cast<AbckitValue *>(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    bool res = g_arktsModifyApi->classFieldSetValue(&field, value);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassFieldAddAnnotation -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassFieldAddAnnotation_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitCoreClassField cfieldCore = {};
    cfieldCore.owner = &klassCore;
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &cfieldCore;
    AbckitCoreAnnotationInterface annoIfaceCore = {};
    annoIfaceCore.owningModule = &dynamicModule;
    AbckitArktsAnnotationInterface ai;
    ai.core = &annoIfaceCore;
    AbckitArktsAnnotationCreateParams params = {};
    params.ai = &ai;

    bool res = g_arktsModifyApi->classFieldAddAnnotation(&field, &params);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ClassFieldRemoveAnnotation -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ClassFieldRemoveAnnotation_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreClass klassCore = {};
    klassCore.owningModule = &dynamicModule;
    AbckitCoreClassField cfieldCore = {};
    cfieldCore.owner = &klassCore;
    AbckitArktsClassField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &cfieldCore;
    AbckitCoreAnnotationInterface annoIfaceCore = {};
    annoIfaceCore.owningModule = &dynamicModule;
    AbckitCoreAnnotation annoCore = {};
    annoCore.ai = &annoIfaceCore;
    AbckitArktsAnnotation anno = {};
    anno.core = &annoCore;

    bool res = g_arktsModifyApi->classFieldRemoveAnnotation(&field, &anno);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Interface Field
// ========================================

// Cover: InterfaceFieldSetName -> IsDynamic branch (always returns true)
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceFieldSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitCoreInterfaceField ifieldCore = {};
    ifieldCore.owner = &ifaceCore;
    AbckitArktsInterfaceField field;
    field.core = &ifieldCore;

    bool res = g_arktsModifyApi->interfaceFieldSetName(&field, "newName");
    ASSERT_EQ(res, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceFieldSetType -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceFieldSetType_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitCoreInterfaceField ifieldCore = {};
    ifieldCore.owner = &ifaceCore;
    AbckitArktsInterfaceField field;
    field.core = &ifieldCore;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto *type = reinterpret_cast<AbckitType *>(0x1);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    bool res = g_arktsModifyApi->interfaceFieldSetType(&field, type);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceFieldAddAnnotation -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceFieldAddAnnotation_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitCoreInterfaceField ifieldCore = {};
    ifieldCore.owner = &ifaceCore;
    AbckitArktsInterfaceField field;
    field.core = &ifieldCore;
    AbckitCoreAnnotationInterface annoIfaceCore = {};
    annoIfaceCore.owningModule = &dynamicModule;
    AbckitArktsAnnotationInterface ai;
    ai.core = &annoIfaceCore;
    AbckitArktsAnnotationCreateParams params = {};
    params.ai = &ai;

    bool res = g_arktsModifyApi->interfaceFieldAddAnnotation(&field, &params);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: InterfaceFieldRemoveAnnotation -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, InterfaceFieldRemoveAnnotation_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreInterface ifaceCore = {};
    ifaceCore.owningModule = &dynamicModule;
    AbckitCoreInterfaceField ifieldCore = {};
    ifieldCore.owner = &ifaceCore;
    AbckitArktsInterfaceField field;
    field.core = &ifieldCore;
    AbckitCoreAnnotationInterface annoIfaceCore = {};
    annoIfaceCore.owningModule = &dynamicModule;
    AbckitCoreAnnotation annoCore = {};
    annoCore.ai = &annoIfaceCore;
    AbckitArktsAnnotation anno = {};
    anno.core = &annoCore;

    bool res = g_arktsModifyApi->interfaceFieldRemoveAnnotation(&field, &anno);
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Enum Field
// ========================================

// Cover: EnumFieldSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, EnumFieldSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreEnum enmCore = {};
    enmCore.owningModule = &dynamicModule;
    AbckitCoreEnumField efieldCore = {};
    efieldCore.owner = &enmCore;
    AbckitArktsEnumField field(static_cast<ark::pandasm::Field *>(nullptr));
    field.core = &efieldCore;

    bool res = g_arktsModifyApi->enumFieldSetName(&field, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// AnnotationInterface
// ========================================

// Cover: AnnotationInterfaceSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, AnnotationInterfaceSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreAnnotationInterface aiCore = {};
    aiCore.owningModule = &dynamicModule;
    AbckitArktsAnnotationInterface ai;
    ai.core = &aiCore;

    bool res = g_arktsModifyApi->annotationInterfaceSetName(&ai, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Function
// ========================================

// Cover: FunctionSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, FunctionSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreFunction funcCore = {};
    funcCore.owningModule = &dynamicModule;
    AbckitArktsFunction func = {};
    func.core = &funcCore;

    bool res = g_arktsModifyApi->functionSetName(&func, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// Cover: ModuleAddFunction -> case ABCKIT_TARGET_ARK_TS_V1: UNSUPPORTED
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, ModuleAddFunction_V1Target)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitArktsModule m = {};
    m.core = &dynamicModule;
    AbckitArktsFunctionCreateParams params = {};

    auto *res = g_arktsModifyApi->moduleAddFunction(&m, &params);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// Annotation
// ========================================

// Cover: AnnotationSetName -> IsDynamic branch
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, AnnotationSetName_DynamicTarget)
{
    AbckitCoreModule dynamicModule = {};
    dynamicModule.target = ABCKIT_TARGET_ARK_TS_V1;
    AbckitCoreAnnotationInterface aiCore = {};
    aiCore.owningModule = &dynamicModule;
    AbckitCoreAnnotation annoCore = {};
    annoCore.ai = &aiCore;
    AbckitArktsAnnotation anno = {};
    anno.core = &annoCore;

    bool res = g_arktsModifyApi->annotationSetName(&anno, "newName");
    ASSERT_EQ(res, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}

// ========================================
// AbckitGetArktsModifyApiImpl version check
// ========================================

// Cover: AbckitGetArktsModifyApiImpl -> default: UNKNOWN_API_VERSION
TEST_F(LibAbcKitWrongModeTestsArktsModifyApiImpl0, GetArktsModifyApiImpl_UnknownVersion)
{
    auto *api = AbckitGetArktsModifyApiImpl(static_cast<AbckitApiVersion>(999));
    ASSERT_EQ(api, nullptr);
}

}  // namespace libabckit::test
