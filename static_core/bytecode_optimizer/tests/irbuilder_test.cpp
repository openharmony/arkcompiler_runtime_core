/**
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

#include "common.h"

namespace panda::bytecodeopt::test {

TEST_F(IrBuilderTest, CallVirtShort)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0, i32 a1) <noimpl>
        .function i32 B.foo(B a0, i32 a1) {
            ldai 0
            return
        }

        .function u1 main() {
            movi v1, 1
            newobj v0, B
            call.virt.short A.foo, v0, v1
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;  // NOLINT(google-build-using-namespace)
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5}, {INT32, 0}, {NO_TYPE, 4}});
            INST(7, Opcode::Return).b().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, CallVirt)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0, i32 a1, i32 a2) <noimpl>
        .function i32 B.foo(B a0, i32 a1, i32 a2) {
            ldai 0
            return
        }

        .function u1 main() {
            movi v1, 1
            movi v2, 2
            newobj v0, B
            call.virt A.foo, v0, v1, v2
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);
        CONSTANT(1, 2);

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;  // NOLINT(google-build-using-namespace)
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::NullCheck).ref().Inputs(4, 5);
            INST(7, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 6}, {INT32, 0}, {INT32, 1}, {NO_TYPE, 5}});
            INST(8, Opcode::Return).b().Inputs(7);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, CallVirtRange)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0, i32 a1, i32 a2) <noimpl>
        .function i32 B.foo(B a0, i32 a1, i32 a2) {
            ldai 0
            return
        }

        .function u1 main() {
            movi v1, 1
            movi v2, 2
            newobj v0, B
            call.virt.range A.foo, v0
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);
        CONSTANT(1, 2);

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;  // NOLINT(google-build-using-namespace)
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::NullCheck).ref().Inputs(4, 5);
            INST(7, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 6}, {INT32, 0}, {INT32, 1}, {NO_TYPE, 5}});
            INST(8, Opcode::Return).b().Inputs(7);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

}  // namespace panda::bytecodeopt::test