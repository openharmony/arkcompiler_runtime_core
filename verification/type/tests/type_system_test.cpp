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

#include "util/tests/environment.h"

#include "util/tests/verifier_test.h"

#include "type/type_system.h"
#include "type/type_sort.h"
#include "type/type_image.h"
#include "type/type_systems.h"

#include "include/runtime.h"

#include "runtime/include/mem/panda_string.h"

#include <gtest/gtest.h>

namespace panda::verifier::test {

TEST_F(VerifierTest, TypeSystemIncrementalClosure)
{
    TypeSystems::Destroy();
    TypeSystems::Initialize(2);
    auto &&typesystem = TypeSystems::Get(TypeSystemKind::PANDA, static_cast<ThreadNum>(1));
    auto paramType = [&typesystem](const auto &name) {
        return typesystem.Parametric(TypeSystems::GetSort(TypeSystemKind::PANDA, static_cast<ThreadNum>(1), name));
    };

    typesystem.SetIncrementalRelationClosureMode(true);
    typesystem.SetDeferIncrementalRelationClosure(false);

    auto bot = typesystem.Bot();
    auto top = typesystem.Top();

    auto i8 = paramType("i8")();
    auto i16 = paramType("i16")();
    auto i32 = paramType("i32")();
    auto i64 = paramType("i64")();

    auto u8 = paramType("u8")();
    auto u16 = paramType("u16")();
    auto u32 = paramType("u32")();
    auto u64 = paramType("u64")();

    auto method = paramType("method");

    auto top_method_of3args = method(-bot >> -bot >> +top);
    auto bot_method_of3args = method(-top >> -top >> +bot);

    auto method1 = method(-i8 >> -i8 >> +i64);
    auto method2 = method(-i32 >> -i16 >> +i32);

    // method2 <: method1
    auto method3 = method(-i16 >> -method2 >> +method1);
    auto method4 = method(-i64 >> -method1 >> +method2);
    // method4 <: method3

    EXPECT_TRUE(bot <= i8);
    EXPECT_TRUE(bot <= u64);

    EXPECT_TRUE(i8 <= top);
    EXPECT_TRUE(u64 <= top);

    i8 << (i16 | i32) << i64;
    (u8 | u16) << (u32 | u64);

    EXPECT_TRUE(i8 <= i64);
    EXPECT_TRUE(i16 <= i64);
    EXPECT_TRUE(i32 <= i64);
    EXPECT_FALSE(i16 <= i32);

    EXPECT_TRUE(u8 <= u64);
    EXPECT_TRUE(u16 <= u64);
    EXPECT_FALSE(u8 <= u16);
    EXPECT_FALSE(u32 <= u64);

    EXPECT_TRUE(method2 <= method1);
    EXPECT_FALSE(method1 <= method2);

    EXPECT_TRUE(method4 <= method3);
    EXPECT_FALSE(method3 <= method4);

    EXPECT_TRUE(bot_method_of3args <= method1);
    EXPECT_TRUE(bot_method_of3args <= method4);

    EXPECT_TRUE(method1 <= top_method_of3args);
    EXPECT_TRUE(method4 <= top_method_of3args);
    TypeSystems::Destroy();
}

TEST_F(VerifierTest, TypeSystemClosureAtTheEnd)
{
    TypeSystems::Destroy();
    TypeSystems::Initialize(1);
    auto &&typesystem = TypeSystems::Get(TypeSystemKind::PANDA, static_cast<ThreadNum>(0));
    auto paramType = [&typesystem](const auto &name) {
        return typesystem.Parametric(TypeSystems::GetSort(TypeSystemKind::PANDA, static_cast<ThreadNum>(0), name));
    };

    typesystem.SetIncrementalRelationClosureMode(false);
    typesystem.SetDeferIncrementalRelationClosure(false);

    auto bot = typesystem.Bot();
    auto top = typesystem.Top();

    auto i8 = paramType("i8")();
    auto i16 = paramType("i16")();
    auto i32 = paramType("i32")();
    auto i64 = paramType("i64")();

    auto u8 = paramType("u8")();
    auto u16 = paramType("u16")();
    auto u32 = paramType("u32")();
    auto u64 = paramType("u64")();

    auto method = paramType("method");

    auto top_method_of3args = method(-bot >> -bot >> +top);
    auto bot_method_of3args = method(-top >> -top >> +bot);

    auto method1 = method(-i8 >> -i8 >> +i64);
    auto method2 = method(-i32 >> -i16 >> +i32);

    // method2 <: method1
    auto method3 = method(-i16 >> -method2 >> +method1);
    auto method4 = method(-i64 >> -method1 >> +method2);
    // method4 <: method3

    i8 << (i16 | i32) << i64;
    (u8 | u16) << (u32 | u64);

    // before closure all methods are unrelated
    EXPECT_FALSE(method2 <= method1);
    EXPECT_FALSE(method1 <= method2);

    EXPECT_FALSE(method4 <= method3);
    EXPECT_FALSE(method3 <= method4);

    EXPECT_FALSE(bot_method_of3args <= method1);
    EXPECT_FALSE(bot_method_of3args <= method4);

    EXPECT_FALSE(method1 <= top_method_of3args);
    EXPECT_FALSE(method4 <= top_method_of3args);

    typesystem.CloseSubtypingRelation();

    // after closure all realations are correct
    EXPECT_TRUE(method2 <= method1);

    EXPECT_TRUE(method4 <= method3);
    EXPECT_TRUE(bot_method_of3args <= method1);
    EXPECT_TRUE(method4 <= top_method_of3args);
    TypeSystems::Destroy();
}

TEST_F(VerifierTest, TypeSystemLeastUpperBound)
{
    TypeSystems::Destroy();
    TypeSystems::Initialize(2);
    auto &&typesystem = TypeSystems::Get(TypeSystemKind::PANDA, static_cast<ThreadNum>(1));
    auto paramType = [&typesystem](const auto &name) {
        return typesystem.Parametric(TypeSystems::GetSort(TypeSystemKind::PANDA, static_cast<ThreadNum>(1), name));
    };

    /*
        G<--
        ^   \
        |    \
        |     \
        |      E<-   .F
        |      ^  \ /  ^
        D      |   X   |
        ^      |  / \  |
        |      | /   \ |
        |      |/     \|
        A      B       C

        NB!!!
        Here is contradiction to conjecture in relation.h about LUB EqClass.
        So here is many object in LUB class but they are not from the same
        class of equivalence.

        In current Panda type system design with Top and Bot, this issue is not
        significant, because in case of such situation (as with E and F),
        LUB will be Top.

        But in general case assumptions that all elements in LUB are from the same
        class of equivalence is wrong. And corresponding functions in relation.h
        should always return full LUB set. And they should be renamed accordingly,
        to do not mislead other developers.
    */

    auto top = typesystem.Top();

    auto a = paramType("A")();
    auto b = paramType("B")();
    auto c = paramType("C")();
    auto d = paramType("D")();
    auto e = paramType("E")();
    auto f = paramType("F")();
    auto g = paramType("G")();

    a << d << g;
    b << e << g;
    b << f;
    c << e;
    c << f;

    auto r = a & b;
    EXPECT_EQ(r, (TypeSet {g, top}));

    r = e & f;
    EXPECT_EQ(r, TypeSet {top});

    r = c & d;
    EXPECT_EQ(r, (TypeSet {g, top}));

    r = a & b & c;
    EXPECT_EQ(r, (TypeSet {g, top}));

    r = a & b & c & f;
    EXPECT_EQ(r, TypeSet {top});

    EXPECT_TRUE(r.TheOnlyType().IsTop());
    TypeSystems::Destroy();
}

}  // namespace panda::verifier::test
