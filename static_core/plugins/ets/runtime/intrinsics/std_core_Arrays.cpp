/*
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

#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_language_context.h"

namespace panda::ets::intrinsics {

template <typename T>
static EtsVoid *StdCoreCopyTo(coretypes::Array *src, coretypes::Array *dst, int32_t dst_start, int32_t src_start,
                              int32_t src_end)
{
    auto src_len = static_cast<int32_t>(src->GetLength());
    auto dst_len = static_cast<int32_t>(dst->GetLength());
    const char *errmsg = nullptr;

    if (src_start < 0 || src_start > src_end || src_end > src_len) {
        errmsg = "copyTo: src bounds verification failed";
    } else if (dst_start < 0 || dst_start > dst_len) {
        errmsg = "copyTo: dst bounds verification failed";
    } else if ((src_end - src_start) > (dst_len - dst_start)) {
        errmsg = "copyTo: Destination array doesn't have enough space";
    }

    if (errmsg == nullptr) {
        auto src_addr = ToVoidPtr(ToUintPtr(src->GetData()) + src_start * sizeof(T));
        auto dst_addr = ToVoidPtr(ToUintPtr(dst->GetData()) + dst_start * sizeof(T));
        auto size = static_cast<size_t>((src_end - src_start) * sizeof(T));
        if (size != 0) {  // cannot be less than zero at this point
            if (memmove_s(dst_addr, size, src_addr, size) != EOK) {
                errmsg = "copyTo: copying data failed";
            }
        }
    }

    if (errmsg != nullptr) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION, errmsg);
    }

    return static_cast<EtsVoid *>(nullptr);
}

extern "C" EtsVoid *StdCoreBoolCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                      int32_t src_end)
{
    return StdCoreCopyTo<uint8_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

extern "C" EtsVoid *StdCoreCharCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                      int32_t src_end)
{
    return StdCoreCopyTo<uint16_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

extern "C" EtsVoid *StdCoreShortCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                       int32_t src_end)
{
    return StdCoreCopyTo<uint16_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

extern "C" EtsVoid *StdCoreByteCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                      int32_t src_end)
{
    return StdCoreCopyTo<uint8_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

extern "C" EtsVoid *StdCoreIntCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                     int32_t src_end)
{
    return StdCoreCopyTo<uint32_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

extern "C" EtsVoid *StdCoreLongCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                      int32_t src_end)
{
    return StdCoreCopyTo<uint64_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

extern "C" EtsVoid *StdCoreFloatCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                       int32_t src_end)
{
    return StdCoreCopyTo<uint32_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

extern "C" EtsVoid *StdCoreDoubleCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dst_start, int32_t src_start,
                                        int32_t src_end)
{
    return StdCoreCopyTo<uint64_t>(src->GetCoreType(), dst->GetCoreType(), dst_start, src_start, src_end);
}

}  // namespace panda::ets::intrinsics
