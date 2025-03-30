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

#include "helpers_nullptr.h"

#include <gtest/gtest.h>
#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/extensions/js/metadata_js.h"
#include "libabckit/include/c/statuses.h"

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

namespace libabckit::test::helpers_nullptr {

// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
[[maybe_unused]] static AbckitFile *g_abckitFile = reinterpret_cast<AbckitFile *>(0x1);
[[maybe_unused]] static AbckitGraph *g_abckitGraph = reinterpret_cast<AbckitGraph *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationElement *g_abckitArktsAnnotationelement =
    reinterpret_cast<AbckitArktsAnnotationElement *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationInterfaceField *g_abckitArktsAnnotationinterfacefield =
    reinterpret_cast<AbckitArktsAnnotationInterfaceField *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationInterface *g_abckitArktsAnnotationinterface =
    reinterpret_cast<AbckitArktsAnnotationInterface *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotation *g_abckitArktsAnnotation = reinterpret_cast<AbckitArktsAnnotation *>(0x1);
[[maybe_unused]] static AbckitArktsClass *g_abckitArktsClass = reinterpret_cast<AbckitArktsClass *>(0x1);
[[maybe_unused]] static AbckitArktsExportDescriptor *g_abckitArktsExportdescriptor =
    reinterpret_cast<AbckitArktsExportDescriptor *>(0x1);
[[maybe_unused]] static AbckitArktsImportDescriptor *g_abckitArktsImportdescriptor =
    reinterpret_cast<AbckitArktsImportDescriptor *>(0x1);
[[maybe_unused]] static AbckitArktsFunction *g_abckitArktsMethod = reinterpret_cast<AbckitArktsFunction *>(0x1);
[[maybe_unused]] static AbckitArktsModule *g_abckitArktsModule = reinterpret_cast<AbckitArktsModule *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotationElement *g_abckitAnnotationelement =
    reinterpret_cast<AbckitCoreAnnotationElement *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotationInterfaceField *g_abckitAnnotationinterfacefield =
    reinterpret_cast<AbckitCoreAnnotationInterfaceField *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotationInterface *g_abckitAnnotationinterface =
    reinterpret_cast<AbckitCoreAnnotationInterface *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotation *g_abckitAnnotation = reinterpret_cast<AbckitCoreAnnotation *>(0x1);
[[maybe_unused]] static AbckitCoreClass *g_abckitClass = reinterpret_cast<AbckitCoreClass *>(0x1);
[[maybe_unused]] static AbckitCoreExportDescriptor *g_abckitExportdescriptor =
    reinterpret_cast<AbckitCoreExportDescriptor *>(0x1);
[[maybe_unused]] static AbckitCoreImportDescriptor *g_abckitImportdescriptor =
    reinterpret_cast<AbckitCoreImportDescriptor *>(0x1);
[[maybe_unused]] static AbckitCoreFunction *g_abckitFunction = reinterpret_cast<AbckitCoreFunction *>(0x1);
[[maybe_unused]] static AbckitCoreModule *g_abckitModule = reinterpret_cast<AbckitCoreModule *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationElementCreateParams *g_abckitArktsAnnotationelementcreateparams =
    reinterpret_cast<AbckitArktsAnnotationElementCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsAnnotationInterfaceFieldCreateParams
    *g_constAbckitArktsAnnotationinterfacefieldcreateparams =
        reinterpret_cast<const AbckitArktsAnnotationInterfaceFieldCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsAnnotationCreateParams *g_constAbckitArktsAnnotationcreateparams =
    reinterpret_cast<const AbckitArktsAnnotationCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsAnnotationInterfaceCreateParams
    *g_constAbckitArktsAnnotationinterfacecreateparams =
        reinterpret_cast<const AbckitArktsAnnotationInterfaceCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsDynamicModuleExportCreateParams
    *g_constAbckitArktsDynamicmoduleexportcreateparams =
        reinterpret_cast<const AbckitArktsDynamicModuleExportCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsImportFromDynamicModuleCreateParams
    *g_constAbckitArktsImportfromdynamicmodulecreateparams =
        reinterpret_cast<const AbckitArktsImportFromDynamicModuleCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsV1ExternalModuleCreateParams *g_constAbckitArktsV1ExternalModuleCreateParams =
    reinterpret_cast<const AbckitArktsV1ExternalModuleCreateParams *>(0x1);
[[maybe_unused]] static AbckitBasicBlock *g_abckitBasicblock = reinterpret_cast<AbckitBasicBlock *>(0x1);
[[maybe_unused]] static AbckitInst *g_abckitInst = reinterpret_cast<AbckitInst *>(0x1);
[[maybe_unused]] static void *g_void = reinterpret_cast<void *>(0x1);
[[maybe_unused]] static AbckitLiteralArray *g_abckitLiteralarray = reinterpret_cast<AbckitLiteralArray *>(0x1);
[[maybe_unused]] static AbckitString *g_abckitString = reinterpret_cast<AbckitString *>(0x1);
[[maybe_unused]] static AbckitValue *g_abckitValue = reinterpret_cast<AbckitValue *>(0x1);
[[maybe_unused]] static AbckitLiteral *g_abckitLiteral = reinterpret_cast<AbckitLiteral *>(0x1);
[[maybe_unused]] static AbckitCoreNamespace *g_abckitNamespace = reinterpret_cast<AbckitCoreNamespace *>(0x1);
[[maybe_unused]] static AbckitType *g_abckitType = reinterpret_cast<AbckitType *>(0x1);
[[maybe_unused]] static AbckitJsClass *g_abckitJsClass = reinterpret_cast<AbckitJsClass *>(0x1);
[[maybe_unused]] static AbckitJsFunction *g_abckitJsMethod = reinterpret_cast<AbckitJsFunction *>(0x1);
[[maybe_unused]] static AbckitJsModule *g_abckitJsModule = reinterpret_cast<AbckitJsModule *>(0x1);
[[maybe_unused]] static const AbckitJsDynamicModuleExportCreateParams *g_constAbckitJsDynamicmoduleexportcreateparams =
    reinterpret_cast<const AbckitJsDynamicModuleExportCreateParams *>(0x1);
[[maybe_unused]] static const AbckitJsImportFromDynamicModuleCreateParams
    *g_constAbckitJsImportfromdynamicmodulecreateparams =
        reinterpret_cast<const AbckitJsImportFromDynamicModuleCreateParams *>(0x1);
[[maybe_unused]] static AbckitJsExportDescriptor *g_abckitJsExportdescriptor =
    reinterpret_cast<AbckitJsExportDescriptor *>(0x1);
[[maybe_unused]] static AbckitJsImportDescriptor *g_abckitJsImportdescriptor =
    reinterpret_cast<AbckitJsImportDescriptor *>(0x1);
[[maybe_unused]] static const AbckitJsExternalModuleCreateParams *g_constAbckitJsExternalmodulecreateparams =
    reinterpret_cast<const AbckitJsExternalModuleCreateParams *>(0x1);
// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

static const char *GetConstChar()
{
    return reinterpret_cast<const char *>(0x1);
}

void TestNullptr(void (*apiToCheck)(AbckitFile *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitGraph *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitFile *, const char *, size_t))
{
    apiToCheck(nullptr, GetConstChar(), 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationElement *(*apiToCheck)(AbckitArktsAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterfaceField *(*apiToCheck)(AbckitArktsAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitArktsAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotation *(*apiToCheck)(AbckitArktsAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitArktsClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitArktsExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitArktsImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitArktsFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitArktsModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationElement *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationInterfaceField *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitCoreAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsClass *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsExportDescriptor *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsImportDescriptor *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsFunction *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsModule *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsFunction *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationElement *(*apiToCheck)(AbckitArktsAnnotation *,
                                                             AbckitArktsAnnotationElementCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsAnnotationelementcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsAnnotation, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationInterfaceField *(*apiToCheck)(
    AbckitArktsAnnotationInterface *, const AbckitArktsAnnotationInterfaceFieldCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationinterfacefieldcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsAnnotationinterface, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsAnnotationInterface *, AbckitArktsAnnotationInterfaceField *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotationinterfacefield);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsAnnotationinterface, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsAnnotation *, AbckitArktsAnnotationElement *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotationelement);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsAnnotation, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitArktsClass *, const AbckitArktsAnnotationCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsClass *, AbckitArktsAnnotation *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsClass, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitArktsFunction *, const AbckitArktsAnnotationCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsMethod, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsFunction *, AbckitArktsAnnotation *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsMethod, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationInterface *(*apiToCheck)(AbckitArktsModule *,
                                                               const AbckitArktsAnnotationInterfaceCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationinterfacecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsExportDescriptor *(*apiToCheck)(AbckitArktsModule *, AbckitArktsModule *,
                                                            const AbckitArktsDynamicModuleExportCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsModule, g_constAbckitArktsDynamicmoduleexportcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsImportDescriptor *(*apiToCheck)(AbckitArktsModule *, AbckitArktsModule *,
                                                            const AbckitArktsImportFromDynamicModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsModule, g_constAbckitArktsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr, g_constAbckitArktsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsModule *, AbckitArktsExportDescriptor *))
{
    apiToCheck(nullptr, g_abckitArktsExportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsModule *, AbckitArktsImportDescriptor *))
{
    apiToCheck(nullptr, g_abckitArktsImportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitInst *))
{
    apiToCheck(nullptr, g_abckitInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *, AbckitInst *, bool))
{
    apiToCheck(nullptr, g_abckitInst, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *))
{
    apiToCheck(nullptr, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *))
{
    apiToCheck(nullptr, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitGraph *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitBasicBlock *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, uint32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, int32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitBasicBlock *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitGraph *(*apiToCheck)(AbckitBasicBlock *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *, uint32_t))
{
    apiToCheck(nullptr, g_abckitBasicblock, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitInst *, bool))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *, void *, bool (*cb)(AbckitBasicBlock *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitBasicBlock *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, void *, bool (*cb)(AbckitBasicBlock *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitBasicBlock *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, double))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, int32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, int64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitGraph *, int32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitGraph *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitGraph *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitGraph *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *, AbckitBasicBlock *, AbckitBasicBlock *))
{
    apiToCheck(nullptr, g_abckitBasicblock, g_abckitBasicblock, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr, g_abckitBasicblock, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_abckitBasicblock, nullptr, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_abckitBasicblock, g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaType (*apiToCheck)(AbckitGraph *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitGraph *, void *, bool (*cb)(AbckitBasicBlock *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitBasicBlock *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitGraph, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitInst *))
{
    apiToCheck(nullptr, g_abckitInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *, AbckitInst *))
{
    apiToCheck(nullptr, g_abckitInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, int32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(double (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(int64_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(int32_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitInst *, size_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitInst *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, size_t, uint64_t))
{
    apiToCheck(nullptr, 0, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBitImmSize (*apiToCheck)(AbckitInst *, size_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitInst *, uint32_t))
{
    apiToCheck(nullptr, g_abckitInst, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, size_t, ...))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitLiteralArray *))
{
    apiToCheck(nullptr, g_abckitLiteralarray);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreFunction *))
{
    apiToCheck(nullptr, g_abckitFunction);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitString *))
{
    apiToCheck(nullptr, g_abckitString);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *, void *, bool (*cb)(AbckitInst *, size_t, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitInst *, size_t, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *, void *, bool (*cb)(AbckitInst *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitInst *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotation *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotation *, void *, bool (*cb)(AbckitCoreAnnotationElement *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotationElement *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitAnnotation, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotationInterface *, void *,
                                    bool (*cb)(AbckitCoreAnnotationInterfaceField *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotationInterfaceField *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitAnnotationinterface, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitClass, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitClass, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitFile *, void *, bool (*cb)(AbckitCoreModule *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreModule *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitFile *, void *, bool (*cb)(AbckitCoreModule *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreModule *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFileVersion (*apiToCheck)(AbckitFile *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitLiteralArray *, void *, bool (*cb)(AbckitFile *, AbckitLiteral *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitFile *, AbckitLiteral *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitLiteralarray, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(double (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitLiteral *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(float (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitLiteral *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralTag (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint16_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint8_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitGraph *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreAnnotationInterface *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotationInterface *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreExportDescriptor *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreExportDescriptor *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreImportDescriptor *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreImportDescriptor *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreNamespace *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreNamespace *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitTarget (*apiToCheck)(AbckitCoreModule *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreNamespace *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreNamespace *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitType *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitTypeId (*apiToCheck)(AbckitType *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(double (*apiToCheck)(AbckitValue *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitValue *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(const char *(*apiToCheck)(AbckitString *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, uint64_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, AbckitLiteralArray *))
{
    ASSERT_EQ(apiToCheck(nullptr, 0, g_abckitLiteralarray), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, uint64_t, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, 0, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, AbckitLiteralArray *, uint64_t,
                                           AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction, g_abckitLiteralarray, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitLiteralarray, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitFunction, nullptr, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitFunction, g_abckitLiteralarray, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitInst *,
                                           AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitBasicBlock *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitBasicblock, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitLiteralArray *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitLiteralarray), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, 0, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, 0, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, 0, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitString *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitString, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitString *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitString, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitString, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitString, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreFunction *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitModule), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitIsaApiDynamicConditionCode))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitString *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitString), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitImportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitExportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitString *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitString), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitString), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitExportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitExportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiDynamicConditionCode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiDynamicOpcode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitIsaApiDynamicConditionCode))
{
    apiToCheck(nullptr, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreExportDescriptor *))
{
    apiToCheck(nullptr, g_abckitExportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreImportDescriptor *))
{
    apiToCheck(nullptr, g_abckitImportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreModule *))
{
    apiToCheck(nullptr, g_abckitModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreFunction *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitType *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitType), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitType), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitIsaApiStaticConditionCode))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE),
              nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreClass *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitClass, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitClass, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitClass), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiStaticConditionCode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiStaticOpcode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitTypeId (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreClass *))
{
    apiToCheck(nullptr, g_abckitClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitIsaApiStaticConditionCode))
{
    apiToCheck(nullptr, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitTypeId))
{
    apiToCheck(nullptr, ABCKIT_TYPE_ID_INVALID);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, ABCKIT_TYPE_ID_INVALID);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsClass *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsFunction *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsModule *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitJsClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitJsFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitJsModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsImportDescriptor *(*apiToCheck)(AbckitJsModule *, AbckitJsModule *,
                                                         const AbckitJsImportFromDynamicModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitJsModule, g_constAbckitJsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitJsModule, nullptr, g_constAbckitJsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitJsModule, g_abckitJsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitJsModule *, AbckitJsExportDescriptor *))
{
    apiToCheck(nullptr, g_abckitJsExportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitJsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitJsModule *, AbckitJsImportDescriptor *))
{
    apiToCheck(nullptr, g_abckitJsImportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitJsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitFile *, AbckitLiteral **, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, &g_abckitLiteral, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, AbckitValue **, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, &g_abckitValue, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, bool))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, double))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, float))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar(), 1), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint16_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint8_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitFile *, AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitClass), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitFile *, const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar(), 1), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitFile *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, double))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar(), 1), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, bool))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsModule *(*apiToCheck)(AbckitFile *, const AbckitArktsV1ExternalModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsV1ExternalModuleCreateParams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsModule *(*apiToCheck)(AbckitFile *, const AbckitJsExternalModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitJsExternalmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitCoreFunction *, AbckitGraph *))
{
    apiToCheck(nullptr, g_abckitGraph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsExportDescriptor *(*apiToCheck)(AbckitJsModule *, AbckitJsModule *,
                                                         const AbckitJsDynamicModuleExportCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitJsModule, g_constAbckitJsDynamicmoduleexportcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitJsModule, g_abckitJsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitArktsNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitArktsNamespace *(*apiToCheck)(AbckitArktsNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitArktsFunction *(*apiToCheck)(AbckitArktsNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitLiteral *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *,
                                    bool (*cb)(AbckitCoreFunction *nestedFunc, void *data)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitJsExportDescriptor *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitJsImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitJsExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitJsImportDescriptor *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, AbckitLiteralArray *))
{
    apiToCheck(nullptr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitArktsNamespace *(*apiToCheck)(AbckitCoreNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

}  // namespace libabckit::test::helpers_nullptr
