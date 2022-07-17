/**
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

#include "runtime/include/field.h"

#include "libpandabase/utils/hash.h"
#include "libpandafile/field_data_accessor.h"
#include "libpandafile/file-inl.h"
#include "runtime/include/runtime.h"

namespace panda {

panda_file::File::StringData Field::GetName() const
{
    const auto *panda_file = GetPandaFile();
    auto name_id = panda_file::FieldDataAccessor::GetNameId(*panda_file, file_id_);
    return panda_file->GetStringData(name_id);
}

Class *Field::ResolveTypeClass(ClassLinkerErrorHandler *error_handler) const
{
    const auto *panda_file = GetPandaFile();
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*GetClass());
    auto *ext = class_linker->GetExtension(ctx);
    ASSERT(ext != nullptr);

    switch (GetTypeId()) {
        case panda_file::Type::TypeId::U1:
            return ext->GetClassRoot(ClassRoot::U1);
        case panda_file::Type::TypeId::I8:
            return ext->GetClassRoot(ClassRoot::I8);
        case panda_file::Type::TypeId::U8:
            return ext->GetClassRoot(ClassRoot::U8);
        case panda_file::Type::TypeId::I16:
            return ext->GetClassRoot(ClassRoot::I16);
        case panda_file::Type::TypeId::U16:
            return ext->GetClassRoot(ClassRoot::U16);
        case panda_file::Type::TypeId::I32:
            return ext->GetClassRoot(ClassRoot::I32);
        case panda_file::Type::TypeId::U32:
            return ext->GetClassRoot(ClassRoot::U32);
        case panda_file::Type::TypeId::I64:
            return ext->GetClassRoot(ClassRoot::I64);
        case panda_file::Type::TypeId::U64:
            return ext->GetClassRoot(ClassRoot::U64);
        case panda_file::Type::TypeId::F32:
            return ext->GetClassRoot(ClassRoot::F32);
        case panda_file::Type::TypeId::F64:
            return ext->GetClassRoot(ClassRoot::F64);
        case panda_file::Type::TypeId::TAGGED:
            return ext->GetClassRoot(ClassRoot::TAGGED);
        case panda_file::Type::TypeId::REFERENCE:
            return ext->GetClass(*panda_file, panda_file::FieldDataAccessor::GetTypeId(*panda_file, file_id_),
                                 GetClass()->GetLoadContext(), error_handler);
        default:
            UNREACHABLE();
    }
}

const panda_file::File *Field::GetPandaFile() const
{
    return GetClass()->GetPandaFile();
}

}  // namespace panda
