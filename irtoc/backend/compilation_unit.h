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

#ifndef PANDA_COMPILATION_ENTRY_H
#define PANDA_COMPILATION_ENTRY_H

#include "compiler/optimizer/ir/ir_constructor.h"
#include "compiler/optimizer/code_generator/relocations.h"
#include "utils/expected.h"
#include "asm_defines.h"
#include "cross_values.h"
#include "runtime/include/managed_thread.h"
#include "source_languages.h"
#include "runtime/include/thread.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace panda::irtoc {

using compiler::Graph;

class CompilationUnit : public compiler::RelocationHandler {
public:
    using Result = Expected<int, const char *>;

    CompilationUnit() {}

    virtual ~CompilationUnit() = default;

    virtual void MakeGraphImpl() = 0;
    virtual const char *GetName() = 0;

    Result Compile(Arch arch);

    auto GetCode()
    {
        return Span(code_);
    }

    Graph *GetGraph()
    {
        return graph_;
    }

    const Graph *GetGraph() const
    {
        return graph_;
    }

    size_t WordSize() const
    {
        return PointerSize(GetArch());
    }

    void AddRelocation(const compiler::RelocationInfo &info) override;

    const auto &GetRelocations() const
    {
        return relocation_entries_;
    }

    const char *GetExternalFunction(size_t index) const
    {
        CHECK_LT(index, external_functions_.size());
        return external_functions_[index].c_str();
    }

    compiler::SourceLanguage GetLanguage() const
    {
        return lang_;
    }

protected:
    Arch GetArch() const
    {
        return GetGraph()->GetArch();
    }

    compiler::RuntimeInterface *GetRuntime()
    {
        return GetGraph()->GetRuntime();
    }

    void SetExternalFunctions(std::initializer_list<std::string> funcs)
    {
        external_functions_ = funcs;
    }

    void SetLanguage(compiler::SourceLanguage lang)
    {
        lang_ = lang;
    }

protected:
    std::unique_ptr<compiler::IrConstructor> builder_;

private:
    Graph *graph_ {nullptr};
    compiler::SourceLanguage lang_ {compiler::SourceLanguage::PANDA_ASSEMBLY};
    std::vector<uint8_t> code_;
    std::vector<std::string> external_functions_;
    std::vector<compiler::RelocationInfo> relocation_entries_;
};

class Compilation {
public:
    using Result = Expected<int, const char *>;
    Result Run(std::string_view output);

    Result MakeElf(std::string_view output);

    template <typename T, typename... Args>
    static int RegisterUnit(Args &&... args)
    {
        static_assert(std::is_base_of_v<CompilationUnit, T>);
        units_.push_back(new T(std::forward<Args>(args)...));
        return 0;
    }

private:
    Result Compile();

private:
    static inline std::vector<CompilationUnit *> units_;
    Arch arch_ {RUNTIME_ARCH};
    std::regex methods_regex_;
};

}  // namespace panda::irtoc

#define COMPILE(name)                                     \
    class name : public CompilationUnit {                 \
    public:                                               \
        using CompilationUnit::CompilationUnit;           \
        void MakeGraphImpl() override;                    \
        const char *GetName() override                    \
        {                                                 \
            return #name;                                 \
        }                                                 \
                                                          \
    private:                                              \
        static int dummy;                                 \
    };                                                    \
    int name ::dummy = Compilation::RegisterUnit<name>(); \
    void name ::MakeGraphImpl()

#endif  // PANDA_COMPILATION_ENTRY_H
