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

#ifndef PANDA_DISASSEMBLY_H
#define PANDA_DISASSEMBLY_H

#include <fstream>
#include <string>
#include <variant>
#include <memory>
#include "compiler_options.h"
#include "macros.h"

namespace panda::compiler {

class Codegen;
class Encoder;
class Inst;
class StackMap;

class Disassembly {
public:
    explicit Disassembly(const Codegen *codegen);
    ~Disassembly() = default;
    NO_COPY_SEMANTIC(Disassembly);
    NO_MOVE_SEMANTIC(Disassembly);

    void Init();

    std::ostream &GetStream()
    {
        return *stream_;
    }
    uint32_t GetDepth() const
    {
        return depth_;
    }
    uint32_t GetPosition() const
    {
        return position_;
    }
    void SetPosition(uint32_t pos)
    {
        position_ = pos;
    }
    const Encoder *GetEncoder() const
    {
        return encoder_;
    }
    void SetEncoder(const Encoder *encoder)
    {
        encoder_ = encoder;
    }
    void IncreaseDepth();
    void DecreaseDepth()
    {
        depth_--;
    }
    bool IsEnabled() const
    {
        return is_enabled_;
    }
    bool IsCodeEnabled() const
    {
        return is_code_enabled_;
    }

    void PrintMethodEntry(const Codegen *codegen);
    void PrintCodeInfo(const Codegen *codegen);
    void PrintCodeStatistics(const Codegen *codegen);
    void PrintStackMap(const Codegen *codegen);

private:
    void FlushDisasm(const Codegen *codegen);

private:
    using StreamDeleterType = void (*)(std::ostream *stream);
    const Codegen *codegen_ {nullptr};
    const Encoder *encoder_ {nullptr};
    std::unique_ptr<std::ostream, StreamDeleterType> stream_;
    uint32_t depth_ {0};
    uint32_t position_ {0};
    bool is_enabled_ {false};
    bool is_code_enabled_ {false};

    friend class ScopedDisasmPrinter;
};

class ScopedDisasmPrinter {
public:
    ScopedDisasmPrinter(Codegen *codegen, const std::string &msg);
    ScopedDisasmPrinter(Codegen *codegen, const Inst *inst);
    ~ScopedDisasmPrinter();

    NO_COPY_SEMANTIC(ScopedDisasmPrinter);
    NO_MOVE_SEMANTIC(ScopedDisasmPrinter);

private:
    Disassembly *disasm_ {nullptr};
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISASM_VAR_CONCAT2(a, b) a##b
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISASM_VAR_CONCAT(a, b) DISASM_VAR_CONCAT2(a, b)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SCOPED_DISASM_INST(codegen, inst) ScopedDisasmPrinter DISASM_VAR_CONCAT(disasm_, __LINE__)(codegen, inst)
#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SCOPED_DISASM_STR(codegen, str) ScopedDisasmPrinter DISASM_VAR_CONCAT(disasm_, __LINE__)(codegen, str)
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SCOPED_DISASM_STR(codegen, str) (void)codegen
#endif

}  // namespace panda::compiler

#endif  // PANDA_DISASSEMBLY_H
