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

interface StringBase<T> {
  process(v: T): T
}
class StringChild implements StringBase<string> {
  process(v: string): string { return v.toUpperCase(); }
}

interface NumberBase<T extends Any> {
  process(v: T): T
}
class NumberChild implements NumberBase<number> {
  process(v: number): number { return v * 2; }
}

interface BooleanBase<T extends Object> {
  process(v: T): T
}
class BooleanChild implements BooleanBase<boolean> {
  process(v: boolean): boolean { return !v; }
}

class Data {
  value: number;
  constructor(v: number) { this.value = v; }
}
interface DataBase<T> {
  process(v: T): T
}
class DataChild implements DataBase<Data> {
  process(v: Data): Data {
    return new Data(v.value + 10);
  }
}

interface VoidBase<T> {
  store(v: T): void
  get(): T | null
}
class VoidChild implements VoidBase<int> {
  stored: int | null = null;
  store(v: int): void { this.stored = v * 2; }
  get(): int | null { return this.stored; }
}

interface ArrayBase<T extends int|string|boolean> {
  wrap(v: T): T[]
}
class ArrayChild implements ArrayBase<string> {
  wrap(v: string): string[] { return [v, v.toUpperCase()]; }
}

interface DirectBase<T extends number|char|boolean|string> {
  transform(v: T): T
}
class DirectChild implements DirectBase<number> {
  transform(v: number): number { return v + 100; }
}

interface ByteBase<T extends byte|char> {
  process(v: T): T
}
class ByteChild implements ByteBase<byte> {
  process(v: byte): byte { return (v + 5).toByte(); }
}

interface LongBase<T extends byte|short|int|long> {
  compute(v: T): T
}
class LongChild implements LongBase<int|long> {
  compute(v: int|long): int|long { return v instanceof int ? v * 3 : v * 2; }
}

interface NullishBase<T extends null|undefined|void|never> {
  compute(v: T): T
}
class NullishChild implements NullishBase<undefined> {
  compute(v: undefined): undefined { return undefined }
}

interface NullishBase2<T extends null|undefined|void> {
  compute(v: T): T
}
class NullishChild2 implements NullishBase2<null> {
  compute(v: null): null { return null }
}

interface Processor<T> {
  process(v: T): T
}
class SP implements Processor<string> {
  process(v: string): string {
    return v.toUpperCase()
  }
}

interface Container<T> {
  get(): T
  set(v: T): void
}
class NC implements Container<number> {
  fld: number
  get(): number { return this.fld }
  set(p: number): void { this.fld = p }
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

  suite.addTest("Bridge method expects string", () => {
    let p: Processor<Object> = new SP() as Processor<Object>
    arktest.assertEQ(p.process("abc"), "ABC");
    arktest.expectThrow(
      () => { p.process(42 as Object) },
      (e: Error) => e instanceof ClassCastError
    )
  });

  suite.addTest("Bridge method expects number", () => {
    let c: Container<Object> = new NC() as Container<Object>
    arktest.assertEQ(c.get(), 0);
    arktest.expectThrow(
      () => { c.set("wrong") },
      (e: Error) => e instanceof ClassCastError
    )
    arktest.expectThrow(
      () => { c.get() as string },
      (e: Error) => e instanceof ClassCastError
    )
  });

  return suite.run();
}
