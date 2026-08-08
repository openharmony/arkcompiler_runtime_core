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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file dump_format.h
 * @brief Hybrid heap dump binary format - single canonical source for all
 * format constants, enums, tag definitions, and packed-struct sizes.
 *
 * Defines the wire format: Language id, file header, record tags, XRef
 * directions, field/class/root type enums, and record body sizes/offsets.
 * All writers (CommonWriter, StaticWriter, DynamicWriter) and parsers
 * (RawHeapTranslateV3) reference these instead of duplicating them locally.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_FORMAT_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_FORMAT_H

#include <array>
#include <cstddef>
#include <cstdint>

// ============================================================================
// Static binary dump - file format overview (dump-side)
// ============================================================================
//
// Authoritative overview of the static/hybrid binary dump file AS THE DUMPER
// WRITES IT. Per-item byte layouts live next to the writer methods
// (static_writer.h StaticWriter::WriteXxxItem; common_writer.h CommonWriter).
// The *_BODY_SIZE / *_OFF constants below are the authoritative sizes/offsets
// those layouts reference. Reader-side parsing (Collect/Build, two-file merge)
// is documented in static_rawheap_translate.h; out of scope here.
//
// Legend: u1=1 byte, u2=2 bytes, u4=4 bytes (LE), u8=8 bytes (LE). All object
// ids are 4-byte nodeIds.
//
// -- 1. FILE OVERVIEW ------------------------------------------------------
//
// A static binary dump file = file header + a stream of records (EOF-driven):
//
//        +---------------------+  offset 0
//        |  HybridDumpHeader   |  33 bytes                 (see 2)
//        +----------+----------+
//                   |
//                   v
//   +----------+----------+   +-------------+   +----------+----------+
//   | RecordHeader (17 B) |-->|   body      |-->| RecordHeader (17 B) |--> ... EOF
//   |                     |   | =count*item |   |                     |
//   +---------------------+   +-------------+   +---------------------+
//
// NOTE: header.recordCount is a SUMMARY metric (totalObj + totalClass), NOT
// the on-disk record count. The reader loops RecordHeader + body until EOF.
//
// -- 2. FILE HEADER (HybridDumpHeader, 33 bytes) ---------------------------
//
//   offset  field           size  value / meaning
//   ----------------------------------------------------------------------
//     0     version          8    "3.0.0\0\0\0"  -- major 3 = static/hybrid
//                                              old V1/V2 tools see ver<3, exit
//     8     identifierSize   4    4   (static-side nodeId width; see STATIC_OBJECT_ID_SIZE)
//    12     timestamp        8    ms since epoch, LE
//    20     language         1    STATIC=1 / HYBRID=2
//    21     headerSize       4    33  (self-describing)
//    25     recordCount      4    totalObj+totalClass  (summary, NOT rec count)
//    29     featureFlags     4    0   (reserved)
//   ----------------------------------------------------------------------
//                                          total 33 bytes
//
//   identifierSize = static-side nodeId width (4 bytes). It governs the
//   static object/class ids only; the 8-byte heap-summary counters and the
//   4-byte XRef dynAddr/staNodeId live in their own record bodies and are
//   NOT governed by this header field.
//
// -- 3. RECORD ENVELOPE (RecordHeader, 17 bytes) + body --------------------
//
//   +-----+--------+--------+--------+------------------------------------+
//   |tag  | time   | length | count  |  body = count x items (same tag)  |
//   | 1B  | 8B LE  | 4B LE  | 4B LE  |                                    |
//   +-----+--------+--------+--------+------------------------------------+
//                                          +---------------------------+
//                                          | item 1 (per-tag layout)  |
//                                          +---------------------------+
//                                          | item 2                   |
//                                          +---------------------------+
//                                          | ...                       |
//                                          +---------------------------+
//
//   - tag selects the item layout (see 4).
//   - length = total body size in bytes.
//   - count  = number of items packed in the body.
//   - Batching: N same-tag items share one 17 B header -> overhead amortised
//     to ~0. Writer flushes a record when its body buffer crosses a threshold
//     after EndItem(); each flushed record is complete and independently
//     parseable.
//   *** The per-tag layouts in 4 describe ONE item; body = count x item. ***
//
// -- 4. TAG CATALOG (emission order, grouped by purpose) ------------------
//
//   Each row: tag | name | item shape | writer method.
//
//   group            tag    name                  item shape        writer
//   ----------------------------------------------------------------------
//   pool/class meta
//                    0x01   STRING_IN_UTF8        id+len+bytes      CommonWriter::WriteStringItem
//                    0x02   LOAD_CLASS            21 fixed          StaticWriter::WriteLoadClassItem
//                    0x0B   STATIC_CLASS_DUMP     22 prefix + var   StaticWriter::WriteClassDumpItem
//   object dumps
//                    0x10   ROOT_RECORD           5 fixed           StaticWriter::WriteRootItem
//                    0x14   STATIC_INSTANCE_DUMP  18 prefix + var   StaticWriter::WriteInstanceDumpItem
//                    0x15   STATIC_ARRAY_DUMP     21 prefix + var   StaticWriter::WriteArrayDumpItem
//                    0x16   STATIC_STRING_DUMP    16 prefix + var   StaticWriter::WriteStringDumpItem
//   cross-VM (hybrid only; appended after BOTH VM lifecycles finish)
//                    0x30   XREF_EDGE             9 fixed           CommonWriter::WriteXRefEdge
//   trailer
//                    0xFE   HEAP_SUMMARY          56 fixed (1 item) CommonWriter::WriteHeapSummary
//                    0xFF   PARTIAL_MARKER        n/a (reader skip) -
//
//   Dynamic-side tags 0xE0..0xE2 go to the dynamic file (V1/V2 rawheap),
//   never the static stream; listed for completeness only.
//
// -- 5. EMISSION ORDER & INVARIANTS ---------------------------------------
//
//   The dumper writes the static stream in this order:
//
//     header
//       |
//       v
//     STRING_IN_UTF8 (string pool)        -- must precede any string-id reference
//       |
//       v
//     LOAD_CLASS
//       |
//       v
//     STATIC_INSTANCE_DUMP / ARRAY_DUMP / STRING_DUMP   (Prepare iteration order)
//       |
//       v
//     STATIC_CLASS_DUMP                   -- DumpClass in StaticDump::Finalize;
//       |                                  all class descriptors precede XRef/Summary
//       v
//     XREF_EDGE  (hybrid only)            -- written AFTER both VM lifecycles complete;
//       |                                  dynNodeId resolved from dynamic entryIdMap
//       v                                  at dump time
//     HEAP_SUMMARY                        -- always last
//
//   Invariants (beyond the order shown above):
//   - LOAD_CLASS + STATIC_CLASS_DUMP together describe every class. An
//     instance/array record may appear before its class descriptor (reader
//     does Collect-then-Build), but all class descriptors precede XRef/Summary.
//   - XREF_EDGE's dynNodeId is resolved from the dynamic entryIdMap, which
//     the dynamic thread populates during its Execute; reading it earlier
//     races the insert (see HeapDumpCoordinator::WriteXRefAndSummary).
// ============================================================================

namespace ark::tooling::hprof {

// ============================================================================
// Language identifier - canonical VM-side identifier
// ============================================================================
//
// Numeric values are stable and written directly to the hybrid dump binary
// format file header (language field). They must not be changed.

/**
 * @brief Canonical language identifier for the hybrid dump system.
 *
 * DYNAMIC = ArkTS-Dyn / JS  (dynamic VM)
 * STATIC  = ArkTS-Sta / ETS (static VM)
 * HYBRID  = both virtual machines together (combined dump)
 */
enum class Language : uint8_t { DYNAMIC = 0, STATIC = 1, HYBRID = 2 };

// ============================================================================
// File header constants
// ============================================================================
//
// The format starts with an 8-byte version string at offset 0, matching the
// V1/V2 rawheap convention. Old V1/V2 tools reading a v3.x file see
// VERSION(2,0,0) < Version(3,0,0) and exit at ParseRawheap (nullptr -> graceful
// error). Header field layout: see section 2 above.

/**
 *  Format version written by the current static dumper: "3.0.0" followed by
 *  three padding nulls. The field occupies 8 bytes. Static snapshot readers
 *  accept well-formed 3.x.x versions after validating the remaining header
 *  fields and feature flags.
 */
static constexpr std::array<char, 8> HYBRID_DUMP_VERSION = {'3', '.', '0', '.', '0', '\0', '\0', '\0'};

/** Size of HYBRID_DUMP_VERSION in bytes (8). */
static constexpr size_t HYBRID_DUMP_VERSION_SIZE = 8;

/**
 *  Size of the 8-byte fields used by heap-summary counters.
 *  (The XRef dynNodeId is 4 bytes - see STATIC_OBJECT_ID_SIZE - so this
 *  constant is now heap-summary-only, but is kept = 8 so HEAP_SUMMARY_BODY_SIZE
 *  and the HS_*_OFF offsets stay correct.)
 */
static constexpr uint32_t HYBRID_DUMP_IDENTIFIER_SIZE = 8;

/**
 *  Static-side object/class nodeId size (4 bytes). Used for all object ids in
 *  ROOT/LOAD_CLASS/STATIC_CLASS_DUMP/STATIC_INSTANCE_DUMP/STATIC_ARRAY_DUMP
 *  and for the XRef staNodeId. Mirrors rawheap_translate::STATIC_IDENTIFIER_SIZE
 *  and the reader's ReadU32-based field reads.
 */
static constexpr uint32_t STATIC_OBJECT_ID_SIZE = sizeof(uint32_t);  // 4

/** Feature flags - reserved for future use, currently 0. */
static constexpr uint32_t HYBRID_DUMP_FEATURE_FLAGS = 0;

/**
 *  File header size in bytes (33).
 *  version(8) + identifierSize(4) + timestamp(8) + language(1)
 *  + headerSize(4) + recordCount(4) + featureFlags(4).
 */
static constexpr uint32_t HYBRID_DUMP_HEADER_SIZE = 33;

/** Milliseconds per second - used for wall-clock timestamp conversion. */
static constexpr uint64_t MS_PER_SEC = 1000ULL;

// ============================================================================
// Record format - shared between all writers and parsers
// ============================================================================
//
// Record envelope and batching are specified in section 3 above. The
// constants below name the 17-byte RecordHeader field sizes/offsets (tag,
// time, length, count) referenced by writer and parser code.

/** Record header size: tag(1) + time(8) + length(4) + count(4) = 17 bytes. */
static constexpr size_t RECORD_HEADER_SIZE = 17;

/** Byte offset of tag field within record header. */
static constexpr size_t TAG_OFFSET = 0;

/** Byte offset of timestamp field within record header (after 1-byte tag). */
static constexpr size_t TIMESTAMP_OFFSET = TAG_OFFSET + 1;

/** Byte offset of body-size field within record header (after tag + 8-byte timestamp). */
static constexpr size_t SIZE_OFFSET = TIMESTAMP_OFFSET + sizeof(uint64_t);

/** Byte offset of count field within record header (after tag + time + length). */
static constexpr size_t COUNT_OFFSET = SIZE_OFFSET + sizeof(uint32_t);

// ============================================================================
// Record tags - shared between all writers and parsers
// ============================================================================

/** Tag for STRING_IN_UTF8 record. */
static constexpr uint8_t TAG_STRING_IN_UTF8 = 0x01;

/** Tag for LOAD_CLASS record (static-side). */
static constexpr uint8_t TAG_LOAD_CLASS = 0x02;

/** Tag for STATIC_CLASS_DUMP record. */
static constexpr uint8_t TAG_STATIC_CLASS_DUMP = 0x0B;

/** Tag for ROOT record (static-side). */
static constexpr uint8_t TAG_ROOT_RECORD = 0x10;

/** Tag for STATIC_INSTANCE_DUMP record. */
static constexpr uint8_t TAG_STATIC_INSTANCE_DUMP = 0x14;

/** Tag for STATIC_ARRAY_DUMP record. */
static constexpr uint8_t TAG_STATIC_ARRAY_DUMP = 0x15;

/**
 *  Tag for STATIC_STRING_DUMP record.
 *  String objects carry their UTF-8 content so the translator can emit a
 *  STRING-typed node named by the content (INSTANCE_DUMP has no per-instance
 *  name field, so without this tag every std.core.String instance would show
 *  up as a generic object named after its class). Unknown tags are skipped
 *  by DispatchRecord -> SkipBody for forward-compatibility.
 */
static constexpr uint8_t TAG_STATIC_STRING_DUMP = 0x16;

/** Tag for XREF_EDGE record. */
static constexpr uint8_t TAG_XREF_EDGE = 0x30;

/** Tag for DYNAMIC_ROOT record. */
static constexpr uint8_t TAG_DYNAMIC_ROOT = 0xE0;

/** Tag for DYNAMIC_INSTANCE_DUMP record. */
static constexpr uint8_t TAG_DYNAMIC_INSTANCE = 0xE1;

/** Tag for DYNAMIC_CLASS_DUMP record (legacy, no longer produced). */
static constexpr uint8_t TAG_DYNAMIC_CLASS_DUMP = 0xE2;

/** Tag for HEAP_SUMMARY record. */
static constexpr uint8_t TAG_HEAP_SUMMARY = 0xFE;

/** Tag for PARTIAL_MARKER record (internal chunk boundary marker). */
static constexpr uint8_t TAG_PARTIAL_MARKER = 0xFF;

// ============================================================================
// XRef direction constants
// ============================================================================

/** Dynamic address -> static address. */
static constexpr uint8_t XREF_DIR_DYN_TO_STA = 0;

/** Static address -> dynamic address. */
static constexpr uint8_t XREF_DIR_STA_TO_DYN = 1;

/** Bidirectional reference (both virtual machines share the object state). */
static constexpr uint8_t XREF_DIR_BIDIR = 2;

// ============================================================================
// Field / class / root type enums - shared vocabulary
// ============================================================================

enum class FieldFlags : uint16_t {
    IS_PUBLIC = 0x0001,
    IS_PRIVATE = 0x0002,
    IS_PROTECTED = 0x0004,
    IS_STATIC = 0x0008,
    IS_FINAL = 0x0010,
    IS_VOLATILE = 0x0020,
    IS_TRANSIENT = 0x0040,
    IS_SYNTHETIC = 0x0080,
};

enum class FieldType : uint8_t {
    UNKNOWN = 0x00,
    BOOLEAN = 0x01,
    CHAR = 0x02,
    FLOAT = 0x03,
    DOUBLE = 0x04,
    BYTE = 0x05,
    SHORT = 0x06,
    INT = 0x07,
    LONG = 0x08,
    OBJECT = 0x09,
    ARRAY = 0x0A,
    TAGGED = 0x0B,
    WEAK_OBJECT = 0x0C,
};

enum class ClassFlags : uint32_t {
    IS_ARRAY = 0x0001,
    IS_INTERFACE = 0x0002,
    IS_ABSTRACT = 0x0004,
    IS_FINAL = 0x0008,
    IS_PUBLIC = 0x0010,
    IS_ANONYMOUS = 0x0020,
    IS_LOCAL = 0x0040,
    IS_MEMBER = 0x0080,
    IS_PRIMITIVE = 0x0100,
    IS_SYNTHETIC = 0x0200,
    IS_SHARED = 0x0400,
};

enum class RootType : uint8_t { STATIC_OBJECT = 0x00 };

// ============================================================================
// Record body size constants
// ============================================================================
//
// Document the expected wire-format sizes of record bodies (excluding
// the 17-byte record header). Used by writers and parsers for format
// verification.

/** HEAP_SUMMARY record body size (7 x 8-byte fields). */
static constexpr size_t HEAP_SUMMARY_BODY_SIZE = 7 * HYBRID_DUMP_IDENTIFIER_SIZE;  // 56

/**
 *  XRef edge record body size: fromAddr(4, dynNodeId) + toAddr(4, staNodeId) + direction(1).
 *  Both endpoints are 4-byte nodeIds (symmetric): the dynamic nodeId is the
 *  32-bit entry-id resolved from the JS heap address at dump time.
 */
static constexpr size_t XREF_EDGE_BODY_SIZE = STATIC_OBJECT_ID_SIZE + STATIC_OBJECT_ID_SIZE + sizeof(uint8_t);  // 9

/** Root record body size: rootType(1) + objectNodeId(4) = 5 bytes. */
static constexpr size_t ROOT_RECORD_BODY_SIZE = sizeof(uint8_t) + STATIC_OBJECT_ID_SIZE;  // 5

/** File header size in bytes (same as HYBRID_DUMP_HEADER_SIZE, alias for convenience). */
static constexpr size_t HYBRID_DUMP_FILE_HEADER_SIZE = HYBRID_DUMP_HEADER_SIZE;

/** Field descriptor size: nameId(4) + type(1) + offset(4) + flags(2) = 11 bytes. */
static constexpr size_t FIELD_DESCRIPTOR_SIZE =
    sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t);  // 11

/** LOAD_CLASS item body size.
 *  serial(4) + classObjId(4) + stackTrace(4) + nameId(4) + language(1) + flags(4) = 21 bytes. */
static constexpr size_t LOAD_CLASS_BODY_SIZE = sizeof(uint32_t) + STATIC_OBJECT_ID_SIZE + sizeof(uint32_t) +
                                               sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t);  // 21

/**
 *  STATIC_CLASS_DUMP fixed prefix size.
 *  classObjId(4) + stackTrace(4) + superId(4) + loaderId(4) + instSize(4) + staticCount(2) = 22 bytes.
 *  The variable tail follows the prefix and is NOT counted here:
 *    [staticCount x FIELD_DESCRIPTOR(11)] + [instanceCount:u2] + [instanceCount x FIELD_DESCRIPTOR(11)]
 *    + [staticValueCount:u2] + staticValueCount x FieldValue (1-byte type + variable value,
 *        in static-field descriptor order; OBJECT/ARRAY/WEAK_OBJECT value is
 *        a 4-byte nodeId and TAGGED value is the original 8-byte tagged payload)
 *    + [methodCount:u2] + methodCount x methodNameId(4)
 */
static constexpr size_t CLASS_DUMP_FIXED_BODY_SIZE = STATIC_OBJECT_ID_SIZE + sizeof(uint32_t) + STATIC_OBJECT_ID_SIZE +
                                                     STATIC_OBJECT_ID_SIZE + sizeof(uint32_t) + sizeof(uint16_t);  // 22

/** Size of a method-name id entry in the STATIC_CLASS_DUMP method section (4 bytes). */
static constexpr size_t STATIC_METHOD_NAMEID_SIZE = sizeof(uint32_t);  // 4

/** STATIC_INSTANCE_DUMP fixed prefix size.
 *  objId(4) + classObjId(4) + stackTrace(4) + instSize(4) + fieldCount(2) = 18 bytes. */
static constexpr size_t INSTANCE_DUMP_FIXED_BODY_SIZE =
    2 * STATIC_OBJECT_ID_SIZE + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t);  // 18

/** STATIC_ARRAY_DUMP fixed prefix size.
 *  objId(4) + classObjId(4) + stackTrace(4) + instSize(4) + arrayLen(4) + elemType(1) = 21 bytes. */
static constexpr size_t ARRAY_INSTANCE_FIXED_BODY_SIZE =
    2 * STATIC_OBJECT_ID_SIZE + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t);  // 21

/**
 *  STATIC_STRING_DUMP fixed prefix size.
 *  objId(4) + classObjId(4) + instSize(4) + valueLen(4) = 16 bytes.
 *  Followed by valueLen bytes of UTF-8 string content.
 */
static constexpr size_t STRING_INSTANCE_FIXED_BODY_SIZE = 3 * STATIC_OBJECT_ID_SIZE + sizeof(uint32_t);  // 16

// ============================================================================
// Record body field offsets (relative to body start)
// ============================================================================
//
// Named offsets for each record type's wire-format field layout. Static-side
// id fields (and the XRef dynNodeId) use STATIC_OBJECT_ID_SIZE (4); heap-summary
// counters use HYBRID_DUMP_IDENTIFIER_SIZE (8).

// --- File header field offsets (relative to header start) ---
static constexpr size_t HDR_IDENTIFIER_SIZE_OFF = HYBRID_DUMP_VERSION_SIZE;                                 // 8
static constexpr size_t HDR_TIMESTAMP_OFF = HYBRID_DUMP_VERSION_SIZE + sizeof(uint32_t);                    // 12
static constexpr size_t HDR_LANGUAGE_OFF = HYBRID_DUMP_VERSION_SIZE + sizeof(uint32_t) + sizeof(uint64_t);  // 20
static constexpr size_t HDR_HEADER_SIZE_OFF = HDR_LANGUAGE_OFF + sizeof(uint8_t);                           // 21
static constexpr size_t HDR_RECORD_COUNT_OFF = HDR_HEADER_SIZE_OFF + sizeof(uint32_t);                      // 25
static constexpr size_t HDR_FEATURE_FLAGS_OFF = HDR_RECORD_COUNT_OFF + sizeof(uint32_t);                    // 29

// --- LOAD_CLASS body field offsets ---
static constexpr size_t LC_SERIAL_OFF = 0;
static constexpr size_t LC_CLASSOBJ_OFF = sizeof(uint32_t);                            // 4
static constexpr size_t LC_STACKTRACE_OFF = sizeof(uint32_t) + STATIC_OBJECT_ID_SIZE;  // 8
static constexpr size_t LC_NAMEID_OFF = LC_STACKTRACE_OFF + sizeof(uint32_t);          // 12
static constexpr size_t LC_LANGUAGE_OFF = LC_NAMEID_OFF + sizeof(uint32_t);            // 16
static constexpr size_t LC_FLAGS_OFF = LC_LANGUAGE_OFF + sizeof(uint8_t);              // 17

// --- CLASS_DUMP body field offsets ---
static constexpr size_t CD_CLASSOBJ_OFF = 0;
static constexpr size_t CD_STACKTRACE_OFF = STATIC_OBJECT_ID_SIZE;                       // 4
static constexpr size_t CD_SUPERCLASS_OFF = CD_STACKTRACE_OFF + sizeof(uint32_t);        // 8
static constexpr size_t CD_CLASSLOADER_OFF = CD_SUPERCLASS_OFF + STATIC_OBJECT_ID_SIZE;  // 12
static constexpr size_t CD_INSTSIZE_OFF = CD_CLASSLOADER_OFF + STATIC_OBJECT_ID_SIZE;    // 16
static constexpr size_t CD_STATIC_COUNT_OFF = CD_INSTSIZE_OFF + sizeof(uint32_t);        // 20

// --- INSTANCE_DUMP body field offsets ---
static constexpr size_t ID_OBJADDR_OFF = 0;
static constexpr size_t ID_CLASSOBJ_OFF = STATIC_OBJECT_ID_SIZE;                      // 4
static constexpr size_t ID_STACKTRACE_OFF = ID_CLASSOBJ_OFF + STATIC_OBJECT_ID_SIZE;  // 8
static constexpr size_t ID_INSTSIZE_OFF = ID_STACKTRACE_OFF + sizeof(uint32_t);       // 12
static constexpr size_t ID_FIELD_COUNT_OFF = ID_INSTSIZE_OFF + sizeof(uint32_t);      // 16

// --- ARRAY_DUMP body field offsets ---
static constexpr size_t AD_OBJADDR_OFF = 0;
static constexpr size_t AD_CLASSOBJ_OFF = STATIC_OBJECT_ID_SIZE;                      // 4
static constexpr size_t AD_STACKTRACE_OFF = AD_CLASSOBJ_OFF + STATIC_OBJECT_ID_SIZE;  // 8
static constexpr size_t AD_INSTSIZE_OFF = AD_STACKTRACE_OFF + sizeof(uint32_t);       // 12
static constexpr size_t AD_ARRAY_LENGTH_OFF = AD_INSTSIZE_OFF + sizeof(uint32_t);     // 16
static constexpr size_t AD_ELEM_TYPE_OFF = AD_ARRAY_LENGTH_OFF + sizeof(uint32_t);    // 20

// --- STATIC_STRING_DUMP body field offsets ---
static constexpr size_t SSD_OBJADDR_OFF = 0;
static constexpr size_t SSD_CLASSOBJ_OFF = STATIC_OBJECT_ID_SIZE;      // 4
static constexpr size_t SSD_INSTSIZE_OFF = 2 * STATIC_OBJECT_ID_SIZE;  // 8
static constexpr size_t SSD_VALUELEN_OFF = 3 * STATIC_OBJECT_ID_SIZE;  // 12

// --- XREF_EDGE body field offsets ---
// fromAddr is the dynamic-side nodeId (4 bytes); toAddr is the static nodeId (4 bytes).
static constexpr size_t XREF_FROM_OFF = 0;
static constexpr size_t XREF_TO_OFF = STATIC_OBJECT_ID_SIZE;                 // 4
static constexpr size_t XREF_DIR_OFF = XREF_TO_OFF + STATIC_OBJECT_ID_SIZE;  // 8

// --- HEAP_SUMMARY body field offsets ---
static constexpr size_t HS_TOTAL_LIVE_BYTES_OFF = 0;
static constexpr size_t HS_TOTAL_LIVE_INST_OFF = HYBRID_DUMP_IDENTIFIER_SIZE;       // 8
static constexpr size_t HS_TOTAL_ALLOC_OFF = 2 * HYBRID_DUMP_IDENTIFIER_SIZE;       // 16
static constexpr size_t HS_TOTAL_INST_ALLOC_OFF = 3 * HYBRID_DUMP_IDENTIFIER_SIZE;  // 24
static constexpr size_t HS_STATIC_OBJ_OFF = 4 * HYBRID_DUMP_IDENTIFIER_SIZE;        // 32
static constexpr size_t HS_DYNAMIC_OBJ_OFF = 5 * HYBRID_DUMP_IDENTIFIER_SIZE;       // 40
static constexpr size_t HS_CLASS_COUNT_OFF = 6 * HYBRID_DUMP_IDENTIFIER_SIZE;       // 48

// --- ROOT_RECORD body field offsets ---
static constexpr size_t ROOT_TYPE_OFF = 0;
static constexpr size_t ROOT_OBJADDR_OFF = sizeof(uint8_t);  // 1

// --- FIELD_DESCRIPTOR field offsets ---
static constexpr size_t FD_NAMEID_OFF = 0;
static constexpr size_t FD_TYPE_OFF = sizeof(uint32_t);                   // 4
static constexpr size_t FD_OFFSET_OFF = FD_TYPE_OFF + sizeof(uint8_t);    // 5
static constexpr size_t FD_FLAGS_OFF = FD_OFFSET_OFF + sizeof(uint32_t);  // 9

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_FORMAT_H
