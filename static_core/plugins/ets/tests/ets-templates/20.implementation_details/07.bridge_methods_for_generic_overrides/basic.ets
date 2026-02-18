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
desc: Bridge methods for generic overrides - Basic scenarios
---*/

class StringBase<T> {
  process(v: T): T { return v; }
}
class StringChild extends StringBase<string> {
  override process(v: string): string { return v.toUpperCase(); }
}

class NumberBase<T> {
  process(v: T): T { return v; }
}
class NumberChild extends NumberBase<number> {
  override process(v: number): number { return v * 2; }
}

class BooleanBase<T> {
  process(v: T): T { return v; }
}
class BooleanChild extends BooleanBase<boolean> {
  override process(v: boolean): boolean { return !v; }
}

class Data {
  value: number;
  constructor(v: number) { this.value = v; }
}
class DataBase<T> {
  process(v: T): T { return v; }
}
class DataChild extends DataBase<Data> {
  override process(v: Data): Data {
    return new Data(v.value + 10);
  }
}

class VoidBase<T> {
  stored: T | null = null;
  store(v: T): void { this.stored = v; }
  get(): T | null { return this.stored; }
}
class VoidChild extends VoidBase<string> {
  override store(v: string): void { this.stored = v.toUpperCase(); }
}

class NoParamBase<T> {
  value: T;
  constructor(v: T) { this.value = v; }
  getValue(): T { return this.value; }
}
class NoParamChild extends NoParamBase<number> {
  constructor(v: number) { super(v); }
  override getValue(): number { return this.value * 3; }
}

class ArrayBase<T> {
  wrap(v: T): T[] { return [v]; }
}
class ArrayChild extends ArrayBase<string> {
  override wrap(v: string): string[] { return [v, v.toUpperCase()]; }
}

class DirectBase<T> {
  transform(v: T): T { return v; }
}
class DirectChild extends DirectBase<number> {
  override transform(v: number): number { return v + 100; }
}

class ByteBase<T> {
  process(v: T): T { return v; }
}
class ByteChild extends ByteBase<byte> {
  override process(v: byte): byte { return (v + 5).toByte(); }
}

class LongBase<T> {
  compute(v: T): T { return v; }
}
class LongChild extends LongBase<long> {
  override compute(v: long): long { return v * 2; }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("BridgeMethodsBasic");

  suite.addTest("String with parent reference", () => {
    let b: StringBase<string> = new StringChild();
    arktest.assertEQ(b.process("test"), "TEST");
  });

  suite.addTest("Number with computation", () => {
    let b: NumberBase<number> = new NumberChild();
    arktest.assertEQ(b.process(15), 30);
  });

  suite.addTest("Bridge with Boolean", () => {
    let b: BooleanBase<boolean> = new BooleanChild();
    arktest.assertEQ(b.process(true), false);
    arktest.assertEQ(b.process(false), true);
  });

  suite.addTest("Custom class", () => {
    let b: DataBase<Data> = new DataChild();
    let d = new Data(25);
    arktest.assertEQ(b.process(d).value, 35);
  });

  suite.addTest("Void return type", () => {
    let b: VoidBase<string> = new VoidChild();
    b.store("hello");
    arktest.assertEQ(b.get(), "HELLO");
  });

  suite.addTest("Method with no parameters", () => {
    let b: NoParamBase<number> = new NoParamChild(7);
    arktest.assertEQ(b.getValue(), 21);
  });

  suite.addTest("Array return type", () => {
    let b: ArrayBase<string> = new ArrayChild();
    let result = b.wrap("test");
    arktest.assertEQ(result.length, 2);
    arktest.assertEQ(result[0], "test");
    arktest.assertEQ(result[1], "TEST");
  });

  suite.addTest("Direct child without parent reference", () => {
    let c = new DirectChild();
    arktest.assertEQ(c.transform(50), 150);
  });

  suite.addTest("Bridge dispatch with parent reference", () => {
    let c: DirectBase<number> = new DirectChild();
    arktest.assertEQ(c.transform(50), 150);
  });

  suite.addTest("Bridge with byte", () => {
    let b: ByteBase<byte> = new ByteChild();
    arktest.assertEQ(b.process((10).toByte()), (15).toByte());
  });

  suite.addTest("Bridge with long", () => {
    let b: LongBase<long> = new LongChild();
    arktest.assertEQ(b.compute(1000000000000), 2000000000000);
  });

  return suite.run();
}
