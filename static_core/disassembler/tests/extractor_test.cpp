/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include "assembly-parser.h"
#include "disasm_backed_debug_info_extractor.h"

TEST(ExtractorTest, LineNumberTable)
{
    auto program = panda::pandasm::Parser().Parse(R"(
        .function void func() {

            nop

        label:

            nop

            jmp label

            return

        }
    )");
    ASSERT(program);

    auto pf = panda::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    panda::panda_file::File::EntityId method_id;
    std::string_view source_name;
    panda::disasm::DisasmBackedDebugInfoExtractor extractor(*pf, [&method_id, &source_name](auto id, auto sn) {
        method_id = id;
        source_name = sn;
    });

    auto id_list = extractor.GetMethodIdList();
    ASSERT_EQ(id_list.size(), 1);

    auto id = id_list[0];
    ASSERT_NE(extractor.GetSourceCode(id), nullptr);

    ASSERT_EQ(id, method_id);
    ASSERT_EQ(extractor.GetSourceFile(id), source_name);

    auto line_table = extractor.GetLineNumberTable(id);
    ASSERT_EQ(line_table.size(), 4);

    ASSERT_EQ(line_table[0].offset, 0);
    ASSERT_EQ(line_table[0].line, 2);

    ASSERT_EQ(line_table[1].offset, 1);
    ASSERT_EQ(line_table[1].line, 4);

    ASSERT_EQ(line_table[2].offset, 2);
    ASSERT_EQ(line_table[2].line, 5);

    ASSERT_EQ(line_table[3].offset, 4);
    ASSERT_EQ(line_table[3].line, 6);
}
