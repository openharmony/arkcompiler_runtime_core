/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Throwing function call if a call of a throwing function is not enclosed in a try statement;.
assert: Exception propagation occurs in two cases. If a call of a throwing function is not enclosed in a try statement; If the enclosed try statement does not contain a clause that catches the raised exception.
---*/

class TestException extends Exception {}

function bar(): void throws {
  throw new TestException();
}

function main(): void {
  let i: int = 1;

  try {
    bar();
  }
  catch(e: TestException) {
    i = 5
  }
  catch (e) {
    i = 10;
  }

  assertEQ( i, 5 );
}
