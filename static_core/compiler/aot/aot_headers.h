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

#ifndef COMPILER_AOT_AOT_HEADERS_H
#define COMPILER_AOT_AOT_HEADERS_H

#include <array>
#include <cstddef>
#include <cstdint>

namespace panda::compiler {

constexpr size_t AOT_HEADER_MAGIC_SIZE = 4;
constexpr size_t AOT_HEADER_VERSION_SIZE = 4;

struct AotHeader {
    alignas(alignof(uint32_t)) std::array<char, AOT_HEADER_MAGIC_SIZE> magic;
    alignas(alignof(uint32_t)) std::array<char, AOT_HEADER_VERSION_SIZE> version;
    uint32_t checksum;
    uint32_t environment_checksum;
    uint32_t arch;
    uint32_t gc_type;
    uint32_t files_count;
    uint32_t files_offset;
    uint32_t class_hash_tables_offset;
    uint32_t classes_offset;
    uint32_t methods_offset;
    uint32_t bitmap_offset;
    uint32_t strtab_offset;
    uint32_t file_name_str;
    uint32_t cmdline_str;
    uint32_t boot_aot;
    uint32_t with_cha;
    uint32_t class_ctx_str;
};

static_assert((sizeof(AotHeader) % sizeof(uint32_t)) == 0);
static_assert(alignof(AotHeader) == alignof(uint32_t));

struct PandaFileHeader {
    uint32_t class_hash_table_size;
    uint32_t class_hash_table_offset;
    uint32_t classes_count;
    uint32_t classes_offset;
    uint32_t methods_count;
    uint32_t methods_offset;
    uint32_t file_checksum;
    uint32_t file_offset;
    uint32_t file_name_str;
};

struct ClassHeader {
    uint32_t class_id;
    uint32_t pab_offset;
    uint32_t methods_count;
    uint32_t methods_offset;
    // Offset to the methods bitmap (aligned as uint32_t)
    uint32_t methods_bitmap_offset;
    // Size of bitmap in bits
    uint32_t methods_bitmap_size;
};

struct MethodHeader {
    uint32_t method_id;
    uint32_t code_offset;
    uint32_t code_size;
};

}  // namespace panda::compiler

#endif  // COMPILER_AOT_AOT_HEADERS_H
