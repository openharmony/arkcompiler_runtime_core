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

#ifndef COMPILER_TOOLS_DEBUG_JIT_WRITER_H
#define COMPILER_TOOLS_DEBUG_JIT_WRITER_H

#ifdef PANDA_COMPILER_CFI

#include "aot/compiled_method.h"
#include "aot/aot_file.h"
#include "aot/aot_builder/elf_builder.h"
#include "utils/arch.h"
#include "utils/arena_containers.h"
#include "utils/bit_vector.h"
#include "optimizer/ir/runtime_interface.h"
#include <string>
#include <vector>
#include "mem/gc/gc_types.h"

namespace panda::panda_file {
class File;
}  // namespace panda::panda_file

namespace panda {
class Class;
}  // namespace panda

namespace panda::compiler {
template <Arch arch, bool is_jit_mode>
class ElfBuilder;

class JitDebugWriter : public ElfWriter {
public:
    // NOLINTNEXTLINE(modernize-pass-by-value)
    JitDebugWriter(Arch arch, RuntimeInterface *runtime, CodeAllocator *code_allocator, const std::string &method_name)
        : code_allocator_(code_allocator), method_name_(method_name)
    {
        SetArch(arch);
        SetRuntime(runtime);
        SetEmitDebugInfo(true);
    }

    JitDebugWriter() = delete;

    bool Write();

    void Start();
    void End();

    Span<uint8_t> GetElf()
    {
        return elf_;
    }

    Span<uint8_t> GetCode()
    {
        return code_;
    }

private:
    template <Arch arch>
    bool WriteImpl();

private:
    Span<uint8_t> elf_;
    Span<uint8_t> code_;
    CodeAllocator *code_allocator_ {nullptr};

    const std::string &method_name_;
    friend class CodeDataProvider;
    friend class JitCodeDataProvider;
};

}  // namespace panda::compiler

// Next "C"-code need for enable interaction with gdb
// Please read "JIT Compilation Interface" from gdb-documentation for more information
extern "C" {
// NOLINTNEXTLINE(modernize-use-using)
typedef enum { JIT_NOACTION = 0, JIT_REGISTER_FN, JIT_UNREGISTER_FN } jit_actions_t;

// NOLINTNEXTLINE(modernize-use-using)
typedef struct jit_code_entry jit_code_entry;
struct jit_code_entry {
    jit_code_entry *next_entry;
    jit_code_entry *prev_entry;
    const char *symfile_addr;
    uint64_t symfile_size;
};

// NOLINTNEXTLINE(modernize-use-using)
typedef struct jit_descriptor {
    uint32_t version;
    uint32_t action_flag;
    jit_code_entry *relevant_entry;
    jit_code_entry *first_entry;
} jit_descriptor;
}

#endif  // PANDA_COMPILER_CFI
#endif  // COMPILER_TOOLS_DEBUG_JIT_WRITER_H
