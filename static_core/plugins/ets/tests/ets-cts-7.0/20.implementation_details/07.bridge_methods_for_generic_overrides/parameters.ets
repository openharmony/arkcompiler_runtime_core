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
desc: Parameter type variations
---*/

class MultiParamBase<T> {
  combine(a: T, b: T): T { return a; }
}
class MultiParamChild extends MultiParamBase<number> {
  override combine(a: number, b: number): number { return a + b; }
}

class MixedBase<T> {
  append(value: T, suffix: string): string { return ""; }
}
class MixedChild extends MixedBase<number> {
  override append(value: number, suffix: string): string {
    return value.toString() + suffix;
  }
}

class OptionalBase<T> {
  format(value: T, prefix?: string): string { return ""; }
}
class OptionalChild extends OptionalBase<number> {
  override format(value: number, prefix?: string): string {
    return (prefix || "") + value.toString();
  }
}

class ArrayParamBase<T> {
  processArray(arr: T[]): number { return 0; }
}
class ArrayParamChild extends ArrayParamBase<string> {
  override processArray(arr: string[]): number { return arr.length; }
}

class SuperCallBase<T> {
  process(v: T): T { return v; }
}
class SuperCallChild extends SuperCallBase<number> {
  override process(v: number): number {
    let result = super.process(v);
    return result * 2;
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("BridgeMethodsParameters");

  suite.addTest("Multiple parameters of same type", () => {
    let b: MultiParamBase<number> = new MultiParamChild();
    arktest.assertEQ(b.combine(10, 25), 35);
  });

  suite.addTest("Mixed generic and non-generic parameters", () => {
    let b: MixedBase<number> = new MixedChild();
    arktest.assertEQ(b.append(36, "!"), "36!");
  });

  suite.addTest("Optional parameters in override", () => {
    let b: OptionalBase<number> = new OptionalChild();
    arktest.assertEQ(b.format(100), "100");
    arktest.assertEQ(b.format(100, "#"), "#100");
  });

  suite.addTest("Array parameters", () => {
    let b: ArrayParamBase<string> = new ArrayParamChild();
    arktest.assertEQ(b.processArray(["a", "b", "c"]), 3);
  });

  suite.addTest("Super call through bridge method", () => {
    let b: SuperCallBase<number> = new SuperCallChild();
    arktest.assertEQ(b.process(8), 16);
  });

  return suite.run();
}
