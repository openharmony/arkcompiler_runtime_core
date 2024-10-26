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

#include "c/abckit.h"
#include "c/extensions/arkts/metadata_arkts.h"
#include "c/metadata_core.h"
#include "c/ir_core.h"
#include "c/statuses.h"

#include <iostream>
#include <functional>

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static void TransformMethod(AbckitCoreFunction *method)
{
    if (g_implI->moduleGetTarget(g_implI->functionGetModule(method)) == ABCKIT_TARGET_ARK_TS_V2) {
        auto *arktsMethod = g_implArkI->coreFunctionToArktsFunction(method);
        if (g_implArkI->functionIsNative(arktsMethod)) {
            return;
        }
    }
    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    if (g_impl->getLastError() != ABCKIT_STATUS_NO_ERROR) {
        return;
    }
    g_implM->functionSetGraph(method, graph);
}

// CC-OFFNXT(超大函数[C++] Oversized function) huge_function
extern "C" int Entry(AbckitFile *file)
{
    bool hasError = false;

    std::function<bool(AbckitCoreFunction *)> cbFunc = [&](AbckitCoreFunction *f) {
        g_implI->functionEnumerateNestedFunctions(f, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(f);
        });

        TransformMethod(f);
        if (g_impl->getLastError() != ABCKIT_STATUS_NO_ERROR) {
            hasError = true;
        }

        return !hasError;
    };

    std::function<bool(AbckitCoreClass *)> cbClass = [&](AbckitCoreClass *c) {
        g_implI->classEnumerateMethods(c, (void *)&cbFunc, [](AbckitCoreFunction *m, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(m);
        });
        return !hasError;
    };

    std::function<void(AbckitCoreNamespace *)> cbNamespce = [&](AbckitCoreNamespace *n) {
        g_implI->namespaceEnumerateNamespaces(n, &cbNamespce, [](AbckitCoreNamespace *n, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreNamespace *)> *>(cb))(n);
        });
        g_implI->namespaceEnumerateClasses(n, &cbClass, [](AbckitCoreClass *c, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
        });
        g_implI->namespaceEnumerateTopLevelFunctions(n, (void *)&cbFunc, [](AbckitCoreFunction *f, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(f);
        });
        return !hasError;
    };

    std::function<void(AbckitCoreModule *)> cbModule = [&](AbckitCoreModule *m) {
        g_implI->moduleEnumerateNamespaces(m, &cbNamespce, [](AbckitCoreNamespace *n, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreNamespace *)> *>(cb))(n);
        });
        g_implI->moduleEnumerateClasses(m, &cbClass, [](AbckitCoreClass *c, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
        });
        g_implI->moduleEnumerateTopLevelFunctions(m, (void *)&cbFunc, [](AbckitCoreFunction *m, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(m);
        });
        return !hasError;
    };

    g_implI->fileEnumerateModules(file, &cbModule, [](AbckitCoreModule *m, void *cb) {
        return (*reinterpret_cast<std::function<bool(AbckitCoreModule *)> *>(cb))(m);
    });

    return hasError ? 1 : 0;
}
