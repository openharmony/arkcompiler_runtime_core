/*
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
#ifndef TAIHE_CALLBACK_ABI_H
#define TAIHE_CALLBACK_ABI_H

#include <taihe/common.h>

struct TCallbackData {
    TRefCount m_count;
    void (*m_free)(struct TCallbackData *);
    void *m_func;
};

struct TCallback {
    struct TCallbackData *data_ptr;
};

TH_EXPORT void tcallback_init(struct TCallbackData *data_ptr, void *func, void (*free)(struct TCallbackData *));

TH_EXPORT struct TCallbackData *tcallback_dup(struct TCallbackData *data_ptr);

TH_EXPORT void tcallback_drop(struct TCallbackData *data_ptr);

#endif // TAIHE_CALLBACK_ABI_H
