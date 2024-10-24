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

#ifndef CPP_ABCKIT_BASIC_BLOCK_H
#define CPP_ABCKIT_BASIC_BLOCK_H

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/config.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/instruction.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace abckit {

class BasicBlock final : public View<AbckitBasicBlock *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class Graph;

public:
    BasicBlock(const BasicBlock &other) = default;
    BasicBlock &operator=(const BasicBlock &other) = default;
    BasicBlock(BasicBlock &&other) = default;
    BasicBlock &operator=(BasicBlock &&other) = default;
    ~BasicBlock() override = default;

    uint64_t GetSuccCount() const;
    BasicBlock GetSuccByIdx(int idx) const;
    std::vector<BasicBlock> GetSuccs() const;
    BasicBlock &AddInstFront(const Instruction &inst);
    std::vector<Instruction> GetInstructions() const;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    void GetSuccsInner(std::vector<BasicBlock> &bBs) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<BasicBlock> *, const ApiConfig *>;
        EnumerateData enumerateData(&bBs, conf);

        conf->cGapi_->bbVisitSuccBlocks(GetView(), (void *)&enumerateData,
                                        []([[maybe_unused]] AbckitBasicBlock *bb, AbckitBasicBlock *succ, void *data) {
                                            auto *vec = static_cast<EnumerateData *>(data)->first;
                                            auto *config = static_cast<EnumerateData *>(data)->second;
                                            vec->push_back(BasicBlock(succ, config));
                                        });
    }

    BasicBlock(AbckitBasicBlock *bb, const ApiConfig *conf) : View(bb), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_BASIC_BLOCK_H
