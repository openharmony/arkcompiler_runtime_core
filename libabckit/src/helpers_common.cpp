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

#include "libabckit/src/helpers_common.h"
#include "libabckit/src/macros.h"

namespace libabckit {

void ModuleEnumerateClassesHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    for (auto &[className, klass] : m->ct) {
        if (!cb(klass.get(), data)) {
            return;
        }
    }
}

void ModuleEnumerateTopLevelFunctionsHelper(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(cb)
    for (auto &function : m->functions) {
        if (!cb(function.get(), data)) {
            return;
        }
    }
}

void ModuleEnumerateAnnotationInterfacesHelper(AbckitCoreModule *m, void *data,
                                               bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(cb)
    for (auto &[atName, at] : m->at) {
        if (!cb(at.get(), data)) {
            return;
        }
    }
}

void ClassEnumerateMethodsHelper(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data))
{
    LIBABCKIT_LOG_FUNC;
    for (auto &method : klass->methods) {
        if (!cb(method.get(), data)) {
            return;
        }
    }
}

bool IsDynamic(AbckitTarget target)
{
    return target == ABCKIT_TARGET_JS || target == ABCKIT_TARGET_ARK_TS_V1;
}

}  // namespace libabckit
