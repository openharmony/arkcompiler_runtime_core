/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_annotation.h"

#include "libpandafile/annotation_data_accessor.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "runtime/include/method.h"

namespace panda::ets {

/*static*/
panda_file::File::EntityId EtsAnnotation::FindAsyncAnnotation(Method *method)
{
    panda_file::File::EntityId async_ann_id;
    const panda_file::File &pf = *method->GetPandaFile();
    panda_file::MethodDataAccessor mda(pf, method->GetFileId());
    mda.EnumerateAnnotations([&pf, &async_ann_id](panda_file::File::EntityId ann_id) {
        panda_file::AnnotationDataAccessor ada(pf, ann_id);
        const char *class_name = utf::Mutf8AsCString(pf.GetStringData(ada.GetClassId()).data);
        if (class_name == panda_file_items::class_descriptors::ASYNC) {
            async_ann_id = ann_id;
        }
    });
    return async_ann_id;
}

}  // namespace panda::ets
