/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "intrinsics.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"

namespace ark::ets::intrinsics {
extern "C" ObjectHeader *EtsArrayBufferFrom(EtsObject *obj)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (obj == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return nullptr;
    }
    if (!obj->GetClass()->IsArrayClass() || !obj->GetClass()->GetComponentType()->IsPrimitive()) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowException(ctx, coro, ctx.GetClassCastExceptionClassDescriptor(),
                       utf::CStringAsMutf8("Object is not an array of primitives"));
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsArray> array(coro, reinterpret_cast<EtsArray *>(obj));
    EtsClass *arraybufClass = coro->GetPandaVM()->GetClassLinker()->GetArrayBufferClass();
    EtsHandle<EtsArrayBuffer> buf(coro, reinterpret_cast<EtsArrayBuffer *>(EtsObject::Create(coro, arraybufClass)));
    if (UNLIKELY(buf.GetPtr() == nullptr)) {
        return nullptr;
    }
    auto byteLength = static_cast<EtsInt>(array->GetLength() * array->GetElementSize());
    buf->SetByteLength(byteLength);
    auto *data = EtsByteArray::Create(byteLength);
    buf->SetData(coro, data);
    if (UNLIKELY(buf->GetData() == nullptr)) {
        return nullptr;
    }
    std::copy_n(array->GetData<EtsByte>(), byteLength, buf->GetData()->GetData<EtsByte>());
    return buf.GetPtr();
}
}  // namespace ark::ets::intrinsics
