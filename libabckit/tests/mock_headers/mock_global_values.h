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

#ifndef ABCKIT_MOCK_GLOBAL_VALUES
#define ABCKIT_MOCK_GLOBAL_VALUES

#include <queue>
#include <string>
#include <iostream>

#include "libpandabase/macros.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_PATH "abckit.abc"
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_CONST_CHAR "abckit default const char*"
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_BOOL 0x1
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U8 0x11
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U16 0x1111
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U32 0x11111111
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U64 0x1111111122222222
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_SIZE_T 0x1111111133333333
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_FILE ((AbckitFile *)0xdeadffff)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_GRAPH ((AbckitGraph *)0xdeadeeee)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_VOID ((void *)0xdead0001)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CB ((void *)0xdead0002)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_MODULE ((AbckitCoreModule *)0xdead0003)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_TARGET ((AbckitTarget)0xdead0004)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRING ((AbckitString *)0xdead0005)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_NAMESPACE ((AbckitCoreNamespace *)0xdead0006)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_IMPORT_DESCRIPTOR ((AbckitCoreImportDescriptor *)0xdead0006)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_EXPORT_DESCRIPTOR ((AbckitCoreExportDescriptor *)0xdead0007)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_CLASS ((AbckitCoreClass *)0xdead0008)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION_INTERFACE ((AbckitCoreAnnotationInterface *)0xdead0009)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD ((AbckitCoreAnnotationInterfaceField *)0xdead0010)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_VALUE ((AbckitValue *)0xdead0011)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_FUNCTION ((AbckitCoreFunction *)0xdead0012)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION ((AbckitCoreAnnotation *)0xdead0013)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION_ELEMENT ((AbckitCoreAnnotationElement *)0xdead0014)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_TYPE_ID ((AbckitTypeId)0xdead0015)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_DOUBLE ((double)0xdead0016)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_LITERAL_ARRAY ((AbckitLiteralArray *)0xdead0017)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_LITERAL ((AbckitLiteral *)0xdead0018)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_LITERAL_TAG ((AbckitLiteralTag)0xdead0019)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_FLOAT ((float)0xdead0020)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_TYPE ((AbckitType *)0xdead0021)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_FILE_VERSION ((AbckitFileVersion)0xdead0022)

// CC-OFFNXT(G.NAM.03) false positive
static std::queue<std::string> g_calledFuncs;

inline bool CheckMockedApi(const std::string &apiName)
{
    if (g_calledFuncs.empty()) {
        return false;
    }
    auto apiStr = std::move(g_calledFuncs.front());
    g_calledFuncs.pop();
    return apiStr == apiName;
}

#endif  // ABCKIT_MOCK_GLOBAL_VALUES
