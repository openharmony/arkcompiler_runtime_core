/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: If dimension expression value is less than zero at runtime, NegativeArraySizeError is thrown.
---*/

function dim(p: int): int {
  return p;
}

function main(): int {
  let suite = new arktest.ArkTestsuite("Runtime negative dimension throws NegativeArraySizeError");

  suite.addTest("number array with negative dim throws", () => {
    arktest.expectThrow(() => {
      let v = new number[dim(-1)](0);
    }, (e: Error): boolean => { return e instanceof NegativeArraySizeError; });
  });

  suite.addTest("int array with negative dim throws", () => {
    arktest.expectThrow(() => {
      let v = new int[dim(-5)](0);
    }, (e: Error): boolean => { return e instanceof NegativeArraySizeError; });
  });

  suite.addTest("string array with negative dim throws", () => {
    arktest.expectThrow(() => {
      let v = new string[dim(-3)]("a");
    }, (e: Error): boolean => { return e instanceof NegativeArraySizeError; });
  });

  suite.addTest("class array with negative dim throws", () => {
    arktest.expectThrow(() => {
      let v = new Object[dim(-7)](new Object());
    }, (e: Error): boolean => { return e instanceof NegativeArraySizeError; });
  });

  suite.addTest("zero dim is valid (not negative)", () => {
    let v = new number[dim(0)](0);
    arktest.assertEQ(v.length, 0);
  });

  return suite.run();
}
