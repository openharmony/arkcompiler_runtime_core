/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    ClassCastError in bridge methods for generic overrides with deep hierarchy with interfaces
---*/

interface Level1<T> {
    process(p: T): T
}
interface Level2<T extends Object> extends Level1<T> {
    process(p: T): T
}
class Level3 implements Level2<number> {
    process(p: number): number { return p * 2 }
}
class Level4 extends Level3 {
    process(p: number): number { return p * 4 }
}

interface Prime1<T> {
    process(p: T): T
}
interface Prime2<T extends Object> extends Prime1<T> {
    process(p: T): T
}
abstract class Prime3 implements Prime2<string> {
    abstract process(p: string): string
}
class Prime4 extends Prime3 {
    process(p: string): string { return p + p }
}

interface Base<T extends Object> {
    process(p: T): T
}
interface Middle<T extends string|boolean|bigint> extends Base<T> {
    process(p: T): T { return p }
}
class Bottom implements Middle<bigint> {
    process(p: bigint): bigint { return p + p }
}

interface SiblingBase<T> {
    process(p: T): T
}
interface SiblingMiddle<T extends Object> extends SiblingBase<T> {
}
class SiblingString implements SiblingMiddle<string> {
    process(p: string): string { return p + p }
}
class SiblingObject implements SiblingMiddle<Object> {
    process(p: Object): Object { return p }
}

interface Stage1<T> {
    process(p: T): T
}
interface Stage2 extends Stage1<byte> {
    process(p: byte): byte
}
class Stage3 implements Stage2 {
    process(p: byte): byte { return ++p }
}

let tmp: Any

function main(): int {
    let suite = new arktest.ArkTestsuite("ClassCastError in Bridge Methods");

    suite.addTest("Bridge method expects Number", () => {
        tmp = new Level4();
        let d: Level1<Object> = tmp as Level1<Object>;
        arktest.assertEQ(d.process(1.0), 4.0);
        arktest.expectError(
            () => { d.process("A") },
            "std.core.String cannot be cast to std.core.Double"
        )
    });

    suite.addTest("Bridge method expects String", () => {
        tmp = new Prime4();
        let d: Prime1<Object> = tmp as Prime1<Object>;
        arktest.assertEQ(d.process("A"), "AA");
        arktest.expectError(
            () => { d.process(1) },
            "std.core.Int cannot be cast to std.core.String"
        )
    });

    suite.addTest("Bridge method expects BigInt", () => {
        tmp = new Bottom();
        let d: Base<Object> = tmp as Base<Object>;
        arktest.assertEQ(d.process(100n), 200n);
        arktest.expectError(
            () => { d.process(1) },
            "std.core.Int cannot be cast to std.core.BigInt"
        )
    });

    suite.addTest("Bridge method expects String", () => {
        tmp = new SiblingString();
        let a: SiblingBase<Object> = tmp as SiblingBase<Object>;
        arktest.expectError(
            () => { a.process(1) },
            "std.core.Int cannot be cast to std.core.String"
        )
    });

    suite.addTest("Bridge method expects Object", () => {
        tmp = new SiblingObject();
        let a: SiblingBase<Any> = tmp as SiblingBase<Any>;
        arktest.assertEQ(a.process(1), 1)
        arktest.expectError(
            () => { a.process(null) },
            "std.core.Null cannot be cast to std.core.Object"
        )
    });

    suite.addTest("CCE on casting to sibling with different specialization", () => {
        let a = new SiblingObject() as SiblingBase<Object>;
        let b = new SiblingString() as SiblingBase<String>;
        tmp = a as SiblingBase<Object> // OK
        tmp = a as SiblingBase<String> // OK
        tmp = b as SiblingBase<Object> // OK
        tmp = b as SiblingBase<String> // OK
        arktest.expectError(
            () => { tmp = b as SiblingObject },
            "interface_bridge4.SiblingString cannot be cast to interface_bridge4.SiblingObject"
        )
        arktest.expectError(
            () => { tmp = a as SiblingString },
            "interface_bridge4.SiblingObject cannot be cast to interface_bridge4.SiblingString"
        )
    });

    suite.addTest("Bridge method expects Byte", () => {
        tmp = new Stage3();
        let d: Stage1<Object> = tmp as Stage1<Object>;
        arktest.assertEQ(d.process(2 as byte), 3);
        arktest.expectError(
            () => { d.process("A") },
            "std.core.String cannot be cast to std.core.Byte"
        )
    });

    return suite.run();
}
