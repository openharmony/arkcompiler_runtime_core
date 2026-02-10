/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_INST_MODIFIER_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_INST_MODIFIER_H

#include <functional>
#include <unordered_map>
#include "static_core/assembler/assembly-program.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit {

class InstModifier {
public:
    explicit InstModifier(AbckitFile *file) : file_(file) {}

    void Modify();

private:
    void EnumerateInst(const std::function<void(ark::pandasm::Ins &ins)> &visitor) const;

    AbckitCoreFunction *GetFunction(const std::string &name) const;

    // Build pandasm::Function* -> AbckitCoreFunction* index to avoid O(N) scan when GetFunction fails after rename.
    void EnsurePandasmToFunctionIndex() const;

    void ModifyInstFunction(ark::pandasm::Ins &ins) const;

    void ModifyInstField(ark::pandasm::Ins &ins) const;

    void ModifyInstClass(ark::pandasm::Ins &ins) const;

    static void RefreshFunctions(std::unordered_map<std::string, AbckitCoreFunction *> &nameToFunction);

    void RefreshFields();

    void RefreshClasses();

    AbckitFile *file_ = nullptr;
    // Reverse index: pandasm::Function* -> AbckitCoreFunction* for O(1) lookup when instruction uses old name.
    mutable std::unordered_map<ark::pandasm::Function *, AbckitCoreFunction *> pandasmToFunction_;
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_ADAPTER_STATIC_INST_MODIFIER_H
