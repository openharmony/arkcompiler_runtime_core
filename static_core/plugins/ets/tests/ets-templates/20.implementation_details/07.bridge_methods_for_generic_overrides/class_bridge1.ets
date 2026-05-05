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
desc: ClassCastError in bridge methods for generic overrides - Basic scenarios
---*/

class StringBase<T> {
  process(v: T): T { return v; }
}
class StringChild extends StringBase<string> {
  override process(v: string): string { return v.toUpperCase(); }
}

class NumberBase<T extends Any> {
  process(v: T): T { return v; }
}
class NumberChild extends NumberBase<number> {
  override process(v: number): number { return v * 2; }
}

class BooleanBase<T extends Object> {
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
class VoidChild extends VoidBase<int> {
  override store(v: int): void { this.stored = v * 2; }
}

class ArrayBase<T extends int|string|boolean> {
  wrap(v: T): T[] { return [v]; }
}
class ArrayChild extends ArrayBase<string> {
  override wrap(v: string): string[] { return [v, v.toUpperCase()]; }
}

class DirectBase<T extends number|char|boolean|string> {
  transform(v: T): T { return v; }
}
class DirectChild extends DirectBase<number> {
  override transform(v: number): number { return v + 100; }
}

class ByteBase<T extends byte|char> {
  process(v: T): T { return v; }
}
class ByteChild extends ByteBase<byte> {
  override process(v: byte): byte { return (v + 5).toByte(); }
}

class LongBase<T extends byte|short|int|long> {
  compute(v: T): T { return v; }
}
class LongChild extends LongBase<int|long> {
  override compute(v: int|long): int|long { return v instanceof int ? v * 3 : v * 2; }
}

class NullishBase<T extends null|undefined|void|never> {
  compute(v: T): T { return v }
}
class NullishChild extends NullishBase<undefined> {
  compute(v: undefined): undefined { return undefined }
}

class NullishBase2<T extends null|undefined|void> {
  compute(v: T): T { return v }
}
class NullishChild2 extends NullishBase2<null> {
  compute(v: null): null { return null }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("ClassCastError in Bridge Method");

  suite.addTest("Bridge method expects String", () => {
    let obj = ((): Object => new StringChild())();
    let b: StringBase<Object> = obj as StringBase<Object>;
    arktest.assertEQ(b.process("test"), "TEST");
    arktest.expectThrow(
      () => { b.process(new Error()) },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects Number", () => {
    let obj = ((): Object => new NumberChild())();
    let b: NumberBase<Object> = obj as NumberBase<Object>;
    arktest.assertEQ(b.process(15.0), 30.0);
    arktest.expectThrow(
      () => { b.process("abc") },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects Boolean", () => {
    let obj = ((): Object => new BooleanChild())();
    let b: BooleanBase<Object> = obj as BooleanBase<Object>;
    arktest.assertEQ(b.process(true), false);
    arktest.assertEQ(b.process(false), true);
    arktest.expectThrow(
      () => { b.process(0) },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects Data", () => {
    let obj = ((): Object => new DataChild())();
    let b: DataBase<Object> = obj as DataBase<Object>;
    let d = new Data(25);
    arktest.assertEQ((b.process(d) as Data).value, 35);
    arktest.expectThrow(
      () => { b.process(new Error) },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects Int", () => {
    let obj = ((): Object => new VoidChild())();
    let b: VoidBase<Any> = obj as VoidBase<Any>;
    b.store(42);
    arktest.assertEQ(b.get(), 84);
    arktest.expectThrow(
      () => { b.store("42") },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects String", () => {
    let obj = ((): Object => new ArrayChild())();
    let b: ArrayBase<string|int|boolean> = obj as ArrayBase<string|int|boolean>;
    let result = b.wrap("test");
    arktest.assertEQ(result.length, 2);
    arktest.assertEQ(result[0], "test");
    arktest.assertEQ(result[1], "TEST");
    arktest.expectThrow(
      () => { b.wrap(0) },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects Number", () => {
    let obj = ((): Object => new DirectChild())();
    let c: DirectBase<number|char|string> = obj as DirectBase<number|char|string>;
    arktest.assertEQ(c.transform(50), 150);
    arktest.expectThrow(
      () => { c.transform(c'A') },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects Byte", () => {
    let obj = ((): Object => new ByteChild())();
    let b: ByteBase<byte|char> = obj as ByteBase<byte|char>;
    arktest.assertEQ(b.process(10 as byte), 15);
    arktest.expectThrow(
      () => { b.process(c'A') },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects int|long", () => {
    let obj = ((): Object => new LongChild())();
    let b: LongBase<byte|short|int|long> = obj as LongBase<byte|short|int|long>;
    arktest.assertEQ(b.compute(1000000000000), 2000000000000);
    arktest.assertEQ(b.compute(10000), 30000);
    arktest.expectThrow(
      () => { b.compute(7 as byte) },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects undefined", () => {
    let obj = ((): Object => new NullishChild())();
    let b: NullishBase<null|undefined|void> = obj as NullishBase<null|undefined|void>;
    arktest.assertEQ(b.compute(undefined), undefined);
    arktest.expectThrow(
      () => { b.compute(null) },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("No bridge method created here", () => {
    let obj = ((): Object => new NullishChild2())();
    let b: NullishBase2<null|undefined|void> = obj as NullishBase2<null|undefined|void>;
    arktest.assertEQ(b.compute(null), null);
    arktest.assertEQ(b.compute(undefined), null);
  });

  return suite.run();
}
