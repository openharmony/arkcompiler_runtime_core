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
desc: Use throw with try statements with exception type in catch statement.
assert: There are two ways to process exception. By handling the exception using a try statement, see “Try Statements”.
---*/

class TestException extends Exception {}

function bar(): int {
  try {
    throw new TestException();
  }
  catch(e: TestException) {
    return 5;
  }
  catch (e) {
    assertTrue( false, "Unexpected catch clause" );
  }

  return 1;
}

function main(): void {
  let y: int = 1;
  y = bar();
  assertEQ( y, 5 );
}
