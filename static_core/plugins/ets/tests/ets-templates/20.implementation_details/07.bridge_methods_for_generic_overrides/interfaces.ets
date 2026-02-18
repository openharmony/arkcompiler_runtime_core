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
desc: Bridge methods for generic overrides - Generic interface implementations
---*/

interface Processor<T> {
  process(v: T): T;
}

class StringProcessor implements Processor<string> {
  process(v: string): string { return v.toUpperCase(); }
}

class NumberProcessor implements Processor<number> {
  process(v: number): number { return v * 3; }
}

interface Transformer<T, U> {
  transform(input: T): U;
}

class StringToNumber implements Transformer<string, number> {
  transform(input: string): number { return input.length; }
}

interface Container<T> {
  get(): T;
  set(v: T): void;
}

class NumberContainer implements Container<number> {
  private value: number = 0;
  get(): number { return this.value; }
  set(v: number): void { this.value = v; }
}

interface Combiner<T> {
  combine(a: T, b: T): T;
}

class StringCombiner implements Combiner<string> {
  combine(a: string, b: string): string { return a + b; }
}

interface Validator<T> {
  validate(v: T): boolean;
}

class PositiveValidator implements Validator<number> {
  validate(v: number): boolean { return v > 0; }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("BridgeMethodsInterfaces");

  suite.addTest("String processor via interface reference", () => {
    let p: Processor<string> = new StringProcessor();
    arktest.assertEQ(p.process("hello"), "HELLO");
  });

  suite.addTest("Number processor via interface reference", () => {
    let p: Processor<number> = new NumberProcessor();
    arktest.assertEQ(p.process(8), 24);
  });

  suite.addTest("Two type param interface", () => {
    let t: Transformer<string, number> = new StringToNumber();
    arktest.assertEQ(t.transform("test"), 4);
  });

  suite.addTest("Container interface with get/set", () => {
    let c: Container<number> = new NumberContainer();
    c.set(99);
    arktest.assertEQ(c.get(), 99);
  });

  suite.addTest("Combiner interface", () => {
    let c: Combiner<string> = new StringCombiner();
    arktest.assertEQ(c.combine("foo", "bar"), "foobar");
  });

  suite.addTest("Validator interface", () => {
    let v: Validator<number> = new PositiveValidator();
    arktest.assertEQ(v.validate(5), true);
    arktest.assertEQ(v.validate(-3), false);
  });

  return suite.run();
}
