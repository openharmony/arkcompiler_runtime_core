/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_INSTRUCTION_H
#define CPP_ABCKIT_INSTRUCTION_H

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/config.h"
#include "cpp/headers/declarations.h"

#include <memory>

namespace abckit {

class Instruction final : public View<AbckitInst *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class BasicBlock;
    friend class StaticIsa;
    friend class DynamicIsa;

public:
    Instruction(const Instruction &other) = default;
    Instruction &operator=(const Instruction &other) = default;
    Instruction(Instruction &&other) = default;
    Instruction &operator=(Instruction &&other) = default;
    ~Instruction() override = default;

    Instruction &InsertAfter(const Instruction &inst);
    Instruction &InsertBefore(const Instruction &inst);
    AbckitIsaApiDynamicOpcode GetOpcodeDyn() const;
    AbckitIsaApiStaticOpcode GetOpcodeStat() const;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    Instruction(AbckitInst *inst, const ApiConfig *conf) : View(inst), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_INSTRUCTION_H
