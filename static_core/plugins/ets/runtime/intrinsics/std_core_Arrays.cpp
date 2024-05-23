/*
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

#include "ets_class_root.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "types/ets_array.h"

namespace ark::ets::intrinsics {

template <typename T>
static void StdCoreCopyTo(coretypes::Array *src, coretypes::Array *dst, int32_t dstStart, int32_t srcStart,
                          int32_t srcEnd)
{
    auto srcLen = static_cast<int32_t>(src->GetLength());
    auto dstLen = static_cast<int32_t>(dst->GetLength());
    const char *errmsg = nullptr;

    if (srcStart < 0 || srcStart > srcEnd || srcEnd > srcLen) {
        errmsg = "copyTo: src bounds verification failed";
    } else if (dstStart < 0 || dstStart > dstLen) {
        errmsg = "copyTo: dst bounds verification failed";
    } else if ((srcEnd - srcStart) > (dstLen - dstStart)) {
        errmsg = "copyTo: Destination array doesn't have enough space";
    }

    if (errmsg == nullptr) {
        auto srcAddr = ToVoidPtr(ToUintPtr(src->GetData()) + srcStart * sizeof(T));
        auto dstAddr = ToVoidPtr(ToUintPtr(dst->GetData()) + dstStart * sizeof(T));
        auto size = static_cast<size_t>((srcEnd - srcStart) * sizeof(T));
        if (size != 0) {  // cannot be less than zero at this point
            if (memmove_s(dstAddr, size, srcAddr, size) != EOK) {
                errmsg = "copyTo: copying data failed";
            }
        }
    }

    if (errmsg != nullptr) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ARRAY_INDEX_OUT_OF_BOUNDS_ERROR, errmsg);
    }
}

extern "C" ObjectHeader *StdCoreAllocGenericArray(ets_int len, EtsObject *sample)
{
    EtsClass *klass;
    if (sample != nullptr) {
        klass = sample->GetClass()->GetComponentType();
    } else {
        klass = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(EtsClassRoot::OBJECT);
    }
    return EtsObjectArray::Create(klass, len)->GetCoreType();
}

extern "C" void StdCoreBoolCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint8_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreCharCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint16_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreShortCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                   int32_t srcEnd)
{
    StdCoreCopyTo<uint16_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreByteCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint8_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreIntCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                 int32_t srcEnd)
{
    StdCoreCopyTo<uint32_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreLongCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint64_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreFloatCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                   int32_t srcEnd)
{
    StdCoreCopyTo<uint32_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreDoubleCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                    int32_t srcEnd)
{
    StdCoreCopyTo<uint64_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

}  // namespace ark::ets::intrinsics
