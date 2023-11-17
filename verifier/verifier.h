/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef VERIFIER_VERIFIER_H
#define VERIFIER_VERIFIER_H

#include <string>

#include <bytecode_instruction-inl.h>
#include "file.h"

namespace panda::verifier {
class Verifier {
public:
    explicit Verifier(const std::string &filename);
    ~Verifier() = default;

    bool Verify();
    bool VerifyRegisterIndex();
    bool VerifyChecksum();
    bool VerifyConstantPool();

private:
    void GetMethodIds();
    void GetLiteralIds();
    bool CheckConstantPool();
    bool VerifyMethodId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id);
    bool VerifyLiteralId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id,
                         size_t idx);
    bool VerifyStringId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id);
    bool CheckConstantPoolInfo(const panda_file::File::EntityId &method_id);

    std::unique_ptr<const panda_file::File> file_;
    std::vector<panda_file::File::EntityId> method_ids_;
    std::vector<uint32_t> literal_ids_;

    static constexpr uint32_t FILE_CONTENT_OFFSET = 12U;
};
} // namespace panda::verifier

#endif
