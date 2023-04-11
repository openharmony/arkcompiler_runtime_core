/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_MEM_TEST_UTILS_H
#define PANDA_RUNTIME_MEM_TEST_UTILS_H

#include "runtime/include/runtime.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/class_root.h"

namespace panda::mem {
[[maybe_unused]] static ObjectHeader *AllocateNullifiedPayloadString(size_t size)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    ASSERT(vm != nullptr);
    auto *string_class = Runtime::GetCurrent()->GetClassLinker()->GetExtension(
        vm->GetLanguageContext())->GetClassRoot(ClassRoot::STRING);
    ASSERT(string_class != nullptr);
    mem::HeapManager *heap_manager = vm->GetHeapManager();
    ASSERT(heap_manager != nullptr);
    return heap_manager->AllocateObject(string_class, size);
}

class ObjectAllocator {
public:
    ObjectAllocator() {}
    ~ObjectAllocator() {}

    coretypes::Array *AllocArray(size_t length, ClassRoot class_root, bool nonmovable)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        SpaceType space_type = SpaceType::SPACE_TYPE_OBJECT;
        auto *klass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(class_root);
        if (nonmovable) {
            space_type = SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT;
        }
        return coretypes::Array::Create(klass, length, space_type);
    }

    coretypes::String *AllocString(size_t length)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        PandaVector<uint8_t> data;
        data.resize(length);
        return coretypes::String::CreateFromMUtf8(data.data(), length, length, true, ctx, runtime->GetPandaVM());
    }

    ObjectHeader *AllocObjectInYoung()
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    }
};
}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_TEST_UTILS_H
