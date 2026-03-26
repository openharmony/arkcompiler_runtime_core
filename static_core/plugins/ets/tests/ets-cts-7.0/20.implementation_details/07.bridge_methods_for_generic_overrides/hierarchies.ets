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
desc: Bridge methods for generic overrides - Complex inheritance hierarchies
---*/

class Level1<T> {
  method(v: T): T { return v; }
}
class Level2 extends Level1<number> {
  override method(v: number): number { return v + 1; }
}
class Level3 extends Level2 {
  override method(v: number): number { return super.method(v) * 2; }
}
class Level4 extends Level3 {
  override method(v: number): number { return super.method(v) + 5; }
}

class SiblingBase<T> {
  process(v: T): T { return v; }
}
class SiblingChild1 extends SiblingBase<string> {
  override process(v: string): string { return v.toUpperCase(); }
}
class SiblingChild2 extends SiblingBase<number> {
  override process(v: number): number { return v * 3; }
}

class MultiParamBase<T, U> {
  combine(a: T, b: U): string { return ""; }
}
class MultiParamChild extends MultiParamBase<string, number> {
  override combine(a: string, b: number): string { return a + b.toString(); }
}

class Pair<T, U> {
  first: T;
  second: U;
  constructor(f: T, s: U) { this.first = f; this.second = s; }
}
class PairParent<T, U> {
  process(a: T, b: U): Pair<T, U> { return new Pair<T, U>(a, b); }
}
class PairChild extends PairParent<string, number> {
  override process(a: string, b: number): Pair<string, number> {
    return new Pair<string, number>(a.toUpperCase(), b * 2);
  }
}

class ConstrainedBase<T> {
  value: T;
  constructor(value: T) { this.value = value; }
  process(): T { return this.value; }
}
class ConstrainedMiddle<T extends string> extends ConstrainedBase<T> {
  constructor(value: T) { super(value); }
}
class ConstrainedBottom extends ConstrainedMiddle<string> {
  constructor(value: string) { super(value); }
  override process(): string { return super.process().repeat(2); }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("BridgeMethodsHierarchies");

  suite.addTest("Four-level hierarchy with super calls", () => {
    let obj: Level1<number> = new Level4();
    // Level2.method(10): 10 + 1 = 11
    // Level3.method(10): super.method(10) * 2 = 11 * 2 = 22
    // Level4.method(10): super.method(10) + 5 = 22 + 5 = 27
    arktest.assertEQ(obj.method(10), 27);
  });

  suite.addTest("Sibling classes with different specializations", () => {
    let c1: SiblingBase<string> = new SiblingChild1();
    let c2: SiblingBase<number> = new SiblingChild2();
    arktest.assertEQ(c1.process("test"), "TEST");
    arktest.assertEQ(c2.process(7), 21);
  });

  suite.addTest("Multiple type parameters", () => {
    let b: MultiParamBase<string, number> = new MultiParamChild();
    arktest.assertEQ(b.combine("value", 36), "value36");
  });

  suite.addTest("Generic pair return type with two type parameters", () => {
    let b: PairParent<string, number> = new PairChild();
    let result = b.process("hello", 15);
    arktest.assertEQ(result.first, "HELLO");
    arktest.assertEQ(result.second, 30);
  });

  suite.addTest("Type constraint in hierarchy", () => {
    let b: ConstrainedBase<string> = new ConstrainedBottom("hi");
    arktest.assertEQ(b.process(), "hihi");
  });

  return suite.run();
}
