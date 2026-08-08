/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

// Compile-time contract: the format constants the offline rawheap_translate
// reader keeps a private copy of (common.h — the CLI tool cannot depend on
// ETS hprof) must stay in sync with the authoritative ark::tooling::hprof
// definitions in dump_format.h. A drift is a silent format-compat break;
// these static_asserts turn it into a build error.

#include "rawheap_translate.h"  // rawheap_translate::* (reader copies; brings common.h)
#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"  // ark::tooling::hprof::* (authoritative)

namespace {
// Version string "3.0.0\0\0\0" must match byte-for-byte.
constexpr bool VersionStringsMatch()
{
    for (size_t i = 0; i < rawheap_translate::STATIC_VERSION_SIZE; ++i) {
        if (rawheap_translate::STATIC_SNAPSHOT_VERSION[i] != ark::tooling::hprof::HYBRID_DUMP_VERSION[i]) {
            return false;
        }
    }
    return true;
}
static_assert(VersionStringsMatch(), "version string mismatch: STATIC_SNAPSHOT_VERSION vs HYBRID_DUMP_VERSION");
}  // namespace

// Record tags (TAG_* mirror ark::tooling::hprof::TAG_*).
static_assert(rawheap_translate::TAG_STRING_IN_UTF8 == ark::tooling::hprof::TAG_STRING_IN_UTF8, "tag");
static_assert(rawheap_translate::TAG_LOAD_CLASS == ark::tooling::hprof::TAG_LOAD_CLASS, "tag");
static_assert(rawheap_translate::TAG_STATIC_CLASS_DUMP == ark::tooling::hprof::TAG_STATIC_CLASS_DUMP, "tag");
static_assert(rawheap_translate::TAG_ROOT_RECORD == ark::tooling::hprof::TAG_ROOT_RECORD, "tag");
static_assert(rawheap_translate::TAG_STATIC_INSTANCE_DUMP == ark::tooling::hprof::TAG_STATIC_INSTANCE_DUMP, "tag");
static_assert(rawheap_translate::TAG_STATIC_ARRAY_DUMP == ark::tooling::hprof::TAG_STATIC_ARRAY_DUMP, "tag");
static_assert(rawheap_translate::TAG_STATIC_STRING_DUMP == ark::tooling::hprof::TAG_STATIC_STRING_DUMP, "tag");
static_assert(rawheap_translate::TAG_XREF_EDGE == ark::tooling::hprof::TAG_XREF_EDGE, "tag");
static_assert(rawheap_translate::TAG_HEAP_SUMMARY == ark::tooling::hprof::TAG_HEAP_SUMMARY, "tag");
static_assert(rawheap_translate::TAG_PARTIAL_MARKER == ark::tooling::hprof::TAG_PARTIAL_MARKER, "tag");

// Root type.
static_assert(rawheap_translate::ROOT_TYPE_STATIC_OBJECT ==
                  static_cast<uint8_t>(ark::tooling::hprof::RootType::STATIC_OBJECT),
              "root type");

// Field types (StaFieldType mirrors FieldType).
#define CHECK_FIELD(F)                                                         \
    static_assert(static_cast<uint8_t>(rawheap_translate::StaFieldType::F) ==  \
                      static_cast<uint8_t>(ark::tooling::hprof::FieldType::F), \
                  "field type")
CHECK_FIELD(UNKNOWN);
CHECK_FIELD(BOOLEAN);
CHECK_FIELD(CHAR);
CHECK_FIELD(FLOAT);
CHECK_FIELD(DOUBLE);
CHECK_FIELD(BYTE);
CHECK_FIELD(SHORT);
CHECK_FIELD(INT);
CHECK_FIELD(LONG);
CHECK_FIELD(OBJECT);
CHECK_FIELD(ARRAY);
CHECK_FIELD(TAGGED);
CHECK_FIELD(WEAK_OBJECT);
#undef CHECK_FIELD

// XRef direction (XREF_* mirror XREF_DIR_*).
static_assert(rawheap_translate::XREF_DYN_TO_STA == ark::tooling::hprof::XREF_DIR_DYN_TO_STA, "xref dir");
static_assert(rawheap_translate::XREF_STA_TO_DYN == ark::tooling::hprof::XREF_DIR_STA_TO_DYN, "xref dir");
static_assert(rawheap_translate::XREF_BIDIR == ark::tooling::hprof::XREF_DIR_BIDIR, "xref dir");

// Sizes (STATIC_* mirror HYBRID_DUMP_* / *_BODY_SIZE / STATIC_OBJECT_ID_SIZE).
static_assert(rawheap_translate::STATIC_VERSION_SIZE == ark::tooling::hprof::HYBRID_DUMP_VERSION_SIZE, "size");
static_assert(rawheap_translate::STATIC_HEADER_SIZE == ark::tooling::hprof::HYBRID_DUMP_HEADER_SIZE, "size");
static_assert(rawheap_translate::STATIC_IDENTIFIER_SIZE == ark::tooling::hprof::STATIC_OBJECT_ID_SIZE, "size");
static_assert(rawheap_translate::STATIC_RECORD_HDR_SIZE == ark::tooling::hprof::RECORD_HEADER_SIZE, "size");
static_assert(rawheap_translate::STATIC_ROOT_BODY_SIZE == ark::tooling::hprof::ROOT_RECORD_BODY_SIZE, "size");
static_assert(rawheap_translate::STATIC_XREF_BODY_SIZE == ark::tooling::hprof::XREF_EDGE_BODY_SIZE, "size");
static_assert(rawheap_translate::STATIC_ARRAY_PREFIX_BODY_SIZE == ark::tooling::hprof::ARRAY_INSTANCE_FIXED_BODY_SIZE,
              "size");
static_assert(rawheap_translate::STATIC_STRING_PREFIX_BODY_SIZE == ark::tooling::hprof::STRING_INSTANCE_FIXED_BODY_SIZE,
              "size");

// Record-header field offsets (record envelope is the core shared structure).
static_assert(rawheap_translate::STATIC_RECORD_HDR_TAG_OFF == ark::tooling::hprof::TAG_OFFSET, "rec hdr off");
static_assert(rawheap_translate::STATIC_RECORD_HDR_TIME_OFF == ark::tooling::hprof::TIMESTAMP_OFFSET, "rec hdr off");
static_assert(rawheap_translate::STATIC_RECORD_HDR_LENGTH_OFF == ark::tooling::hprof::SIZE_OFFSET, "rec hdr off");
static_assert(rawheap_translate::STATIC_RECORD_HDR_COUNT_OFF == ark::tooling::hprof::COUNT_OFFSET, "rec hdr off");
