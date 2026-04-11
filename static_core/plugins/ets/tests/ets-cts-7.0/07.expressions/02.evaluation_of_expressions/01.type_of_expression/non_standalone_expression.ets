/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
  There are two ways to facilitate the compatibility of a non-standalone
  expression with its surrounding context.
---*/

let x = ' ';

type someUnion = number|string;

function check(a: Any, type: string) {
  arktest.assertEQ(typeof a, type)
}

const suite = new arktest.ArkTestsuite("Non-standalone expression type compatibility")

suite.addTest("Assignment from '48' to 'number'", () => {
  let a: number = 48;
  check(a, "number")
})

suite.addTest("Assignment from '\"String\"' to 'string'", () => {
  let a: string = "String";
  check(a, "string")
})

suite.addTest("Assignment from 'true' to 'boolean'", () => {
  let a: boolean = true;
  check(a, "boolean")
})

suite.addTest("Assignment from 'null' to 'null'", () => {
  let a: null = null;
  check(a, "object")
})

suite.addTest("Assignment from 'undefined' to 'undefined'", () => {
  let a: undefined = undefined;
  check(a, "undefined")
})

suite.addTest("Assignment from Record to 'Record<string,number>'", () => {
  let a: Record<string,number> = { "x":1, "y":2 };
  check(a, "object")
})

suite.addTest("Assignment from '[1, 2, 3]' to 'number[]'", () => {
  let a: number[] = [1, 2, 3];
  check(a, "object")
})

suite.addTest("Assignment from '[1, \"two\"]' to '[number, string]'", () => {
  let a: [number, string] = [1, "two"];
  check(a, "object")
})

suite.addTest("Assignment from arrow function to '(x:number) => number'", () => {
  let a: (x:number) => number = (x:number) => x + 1;
  check(a, "function")
})

suite.addTest("Assignment from void arrow to '() => void'", () => {
  let a: () => void = ()=> {console.log("void")};
  check(a, "function")
})

suite.addTest("Assignment from '1' to 'someUnion'", () => {
  let a: someUnion = 1;
  check(a, "number")
})

suite.addTest("Assignment from '(48).toString()' to 'string'", () => {
  let a: string = (48).toString();
  check(a, "string")
})

suite.addTest("Assignment from '100n' to 'bigint'", () => {
  let a: bigint = 100n;
  check(a, "bigint")
})

suite.addTest("Assignment from 'NaN' to 'number'", () => {
  let a: number = NaN;
  check(a, "number")
})

suite.addTest("Assignment from 'Array.create<number>(3, 0)' to 'number[]'", () => {
  let a: number[] = Array.create<number>(3, 0);
  check(a, "object")
})

suite.addTest("Assignment from '[1] instanceof Array' to 'boolean'", () => {
  let a: boolean = [1] instanceof Array;
  check(a, "boolean")
})

suite.addTest("Assignment from ternary string to 'string'", () => {
  let a: string = (typeof x == "string") ? x:"";
  check(a, "string")
})

suite.addTest("Assignment from 'true && false' to 'boolean'", () => {
  let a: boolean = true && false;
  check(a, "boolean")
})

suite.addTest("Assignment from '10 + 5' to 'number'", () => {
  let a: number = 10 + 5;
  check(a, "number")
})

suite.addTest("Assignment from '(1 > 2)' to 'boolean'", () => {
  let a: boolean = (1 > 2);
  check(a, "boolean")
})

suite.addTest("Assignment from 'x ? 1:0' to 'number'", () => {
  let a: number = x ? 1:0;
  check(a, "number")
})

function main() {
  suite.run()
}
