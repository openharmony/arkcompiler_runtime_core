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
#include <taihe/common.h>
#include <taihe/string.abi.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Converts a TString into its corresponding heap-allocated TStringData
// structure.
// # Returns
// - A pointer to the TStringData structure if the TString is heap-allocated.
// - `NULL` if the TString is a reference (TSTRING_REF is set).
TH_INLINE struct TStringData *to_heap(struct TString tstr)
{
    if (tstr.flags & TSTRING_REF) {
        return NULL;
    }
    return (struct TStringData *)((char *)tstr.ptr - offsetof(struct TStringData, buffer));
}

char *tstr_initialize(struct TString *tstr_ptr, uint32_t capacity)
{
    size_t bytes_required = sizeof(struct TStringData) + sizeof(char) * capacity;
    struct TStringData *sh = (struct TStringData *)malloc(bytes_required);
    tref_set(&sh->count, 1);
    tstr_ptr->flags = 0;
    tstr_ptr->ptr = sh->buffer;
    return sh->buffer;
}

struct TString tstr_new(char const *value TH_NONNULL, size_t len)
{
    struct TString tstr;
    char *buf = tstr_initialize(&tstr, len + 1);
    memcpy(buf, value, sizeof(char) * len);
    buf[len] = '\0';
    tstr.length = len;
    return tstr;
}

struct TString tstr_new_ref(char const *buf TH_NONNULL, size_t len)
{
    struct TString tstr;
    tstr.flags = TSTRING_REF;
    tstr.length = len;
    tstr.ptr = buf;
    return tstr;
}

struct TString tstr_dup(struct TString tstr)
{
    struct TStringData *sh = to_heap(tstr);
    if (!sh) {
        return tstr_new(tstr.ptr, tstr.length);
    }
    tref_inc(&sh->count);
    return tstr;
}

void tstr_drop(struct TString tstr)
{
    struct TStringData *sh = to_heap(tstr);
    if (!sh) {
        return;
    }
    if (tref_dec(&sh->count)) {
        free(sh);
    }
}

struct TString tstr_concat(struct TString left, struct TString right)
{
    size_t len = left.length + right.length;
    struct TString tstr;
    char *buf = tstr_initialize(&tstr, len + 1);
    memcpy(buf, left.ptr, sizeof(char) * left.length);
    buf += left.length;
    memcpy(buf, right.ptr, sizeof(char) * right.length);
    buf += right.length;
    *buf = '\0';
    tstr.length = len;
    return tstr;
}

struct TString tstr_substr(struct TString tstr, size_t pos, size_t len)
{
    if (pos > tstr.length) {
        len = 0;
    } else if (pos + len > tstr.length) {
        len = tstr.length - pos;
    }
    return tstr_new_ref(tstr.ptr + pos, len);
}
