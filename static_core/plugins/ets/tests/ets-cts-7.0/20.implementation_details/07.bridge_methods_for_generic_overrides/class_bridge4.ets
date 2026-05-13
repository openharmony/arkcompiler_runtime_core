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
    ClassCastError in bridge methods for generic overrides with deep hierarchy of classes
---*/

class Level1<T> {
    process(p: T): T { return p }
}
class Level2 extends Level1<number> {
    process(p: number): number { return p * 2 }
}
class Level3 extends Level2 {
}
class Level4 extends Level3 {
}

class Prime1<T> {
    process(p: T): T { return p }
}
abstract class Prime2<T> extends Prime1<T> {
    abstract process(p: T): T
}
class Prime3 extends Prime2<int> {
    process(p: int): int { return p }
}
class Prime4 extends Prime3 {
    override process(p: int): int { return p * 4 }
}

class Area1<T> {
    process(p: T): T { return p }
}
abstract class Area2<T> extends Area1<T> {
}
abstract class Area3 extends Area2<byte|short|int> {
    abstract process(p: byte|short|int): byte|short|int
}
class Area4 extends Area3 {
    override process(p: byte|short|int): byte { return p.toByte() }
}

class Base<T> {
    process(p: T): T { return p }
}
class Middle<T extends string> extends Base<T> {
    process(p: T): T { return p }
}
class Bottom extends Middle<string> {
    process(p: string): string { return p + p }
}

class SiblingBase<T> {
    process(p: T): T { return p }
}
abstract class SiblingMiddle<T extends Object> extends SiblingBase<T> {
}
class SiblingString extends SiblingMiddle<string> {
    process(p: string): string { return p + p }
}
class SiblingObject extends SiblingMiddle<Object> {
    process(p: Object): Object { return p }
}

abstract class Stage1<T> {
    abstract process(p: T): T
}
abstract class Stage2 extends Stage1<byte> {
}
class Stage3 extends Stage2 {
    override process(p: byte): byte { return ++p }
}

let tmp: Any

function main(): int {
    let suite = new arktest.ArkTestsuite("ClassCastError in Bridge Methods");

    suite.addTest("Bridge method expects Number", () => {
        tmp = new Level4();
        let d: Level1<Object> = tmp as Level1<Object>;
        arktest.assertEQ(d.process(1.0), 2.0);
        arktest.expectError(
            () => { d.process("A") },
            "std.core.String cannot be cast to std.core.Double"
        )
    });

    suite.addTest("Bridge method expects Int", () => {
        tmp = new Prime4();
        let d: Prime1<Object> = tmp as Prime1<Object>;
        arktest.assertEQ(d.process(1), 4);
        arktest.expectError(
            () => { d.process("A") },
            "std.core.String cannot be cast to std.core.Int"
        )
    });

    suite.addTest("Bridge method expects Byte|Short|Int", () => {
        tmp = new Area4();
        let d: Area1<Object> = tmp as Area1<Object>;
        arktest.assertEQ(d.process(256), 0);
        arktest.expectError(
            () => { d.process("A") },
            "std.core.String cannot be cast to {Ustd.core.Byte,std.core.Int,std.core.Short}"
        )
    });

    suite.addTest("Bridge method expects String", () => {
        tmp = new Bottom();
        let d: Base<Object> = tmp as Base<Object>;
        arktest.assertEQ(d.process("A"), "AA");
        arktest.expectError(
            () => { d.process(1) },
            "std.core.Int cannot be cast to std.core.String"
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
            "class_bridge4.SiblingString cannot be cast to class_bridge4.SiblingObject"
        )
        arktest.expectError(
            () => { tmp = a as SiblingString },
            "class_bridge4.SiblingObject cannot be cast to class_bridge4.SiblingString"
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
