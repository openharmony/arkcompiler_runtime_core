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
  No target type is expected in the context, then the expression
  is called a standalone expression.
---*/

let x = ' ';

function check(a: Any, type: string) {
  arktest.assertEQ(typeof a, type)
}

const suite = new arktest.ArkTestsuite("Standalone expression type")

suite.addTest("Assignment from '48'", () => {
  let a = 48;
  check(a, "int")
})

suite.addTest("Assignment from '\"String\"'", () => {
  let a = "String";
  check(a, "string")
})

suite.addTest("Assignment from 'true'", () => {
  let a = true;
  check(a, "boolean")
})

suite.addTest("Assignment from 'null'", () => {
  let a = null;
  check(a, "object")
})

suite.addTest("Assignment from 'undefined'", () => {
  let a = undefined;
  check(a, "undefined")
})

suite.addTest("Assignment from '[1, 2, 3]'", () => {
  let a = [1, 2, 3];
  check(a, "object")
})

suite.addTest("Assignment from '[1, \"two\"]'", () => {
  let a = [1, "two"];
  check(a, "object")
})

suite.addTest("Assignment from '(x:number) => x + 1'", () => {
  let a = (x:number) => x + 1;
  check(a, "function")
})

suite.addTest("Assignment from '()=> {console.log(\"void\")}'", () => {
  let a = ()=> {console.log("void")};
  check(a, "function")
})

suite.addTest("Assignment from '(48).toString()'", () => {
  let a = (48).toString();
  check(a, "string")
})

suite.addTest("Assignment from '100n'", () => {
  let a = 100n;
  check(a, "bigint")
})

suite.addTest("Assignment from 'NaN'", () => {
  let a = NaN;
  check(a, "number")
})

suite.addTest("Assignment from 'Array.create<number>(3, 0)'", () => {
  let a = Array.create<number>(3, 0);
  check(a, "object")
})

suite.addTest("Assignment from '[1] instanceof Array'", () => {
  let a = [1] instanceof Array;
  check(a, "boolean")
})

suite.addTest("Assignment from ternary string expression", () => {
  let a = (typeof x == "string") ? x:"";
  check(a, "string")
})

suite.addTest("Assignment from 'true && false'", () => {
  let a = true && false;
  check(a, "boolean")
})

suite.addTest("Assignment from '10 + 5'", () => {
  let a = 10 + 5;
  check(a, "int")
})

suite.addTest("Assignment from '(1 > 2)'", () => {
  let a = (1 > 2);
  check(a, "boolean")
})

suite.addTest("Assignment from 'x ? 1:0'", () => {
  let a = x ? 1:0;
  check(a, "int")
})

function main() {
  suite.run()
}
