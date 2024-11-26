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

#ifndef LIBABCKIT_SRC_HELPERS_COMMON_H
#define LIBABCKIT_SRC_HELPERS_COMMON_H

#include "metadata_inspect_impl.h"

namespace libabckit {

bool ModuleEnumerateClassesHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data));
bool ModuleEnumerateTopLevelFunctionsHelper(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data));
bool ModuleEnumerateAnnotationInterfacesHelper(AbckitCoreModule *m, void *data,
                                               bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

bool ClassEnumerateMethodsHelper(AbckitCoreClass *klass, void *data,
                                 bool (*cb)(AbckitCoreFunction *method, void *data));

bool IsDynamic(AbckitTarget target);

AbckitType *GetOrCreateType(AbckitFile *file, AbckitTypeId id, size_t rank, AbckitCoreClass *klass);

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_HELPERS_COMMON_H
