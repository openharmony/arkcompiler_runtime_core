/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "annotation_data_accessor.h"
#include "class_data_accessor.h"
#include "ets_panda_file_items.h"
#include "ets_vm.h"
#include "file.h"
#include "include/managed_thread.h"
#include "include/object_header.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "types/ets_object.h"
#include "types/ets_primitives.h"

namespace ark::ets::intrinsics {

extern "C" EtsBoolean IsLiteralInitializedInterfaceImpl(EtsObject *target)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(ManagedThread::GetCurrent());

    auto *etsClass = target->GetClass();
    auto *runtimeClass = etsClass->GetRuntimeClass();
    const panda_file::File &pf = *runtimeClass->GetPandaFile();
    panda_file::ClassDataAccessor cda(pf, runtimeClass->GetFileId());
    bool retBoolVal = false;

    cda.EnumerateAnnotation(panda_file_items::class_descriptors::INTERFACE_OBJ_LITERAL.data(),
                            [&retBoolVal](panda_file::AnnotationDataAccessor &) {
                                retBoolVal = true;
                                return true;
                            });

    return ToEtsBoolean(retBoolVal);
}

}  // namespace ark::ets::intrinsics
