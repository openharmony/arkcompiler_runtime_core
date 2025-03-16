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

const RESULT_SUCCESS = 0
const RESULT_FAILURE = 1

class Assertions {
  static assertEquals(expected: number, actual: number, message: string): void {
    if (expected != actual) throw new AssertionError(`${message} - expected: ${expected} but was ${actual}`)
  }

  static assertSame(expected: Object, actual: Object, message: string): void {
    if (expected !== actual) throw new AssertionError(message)

  } 

  static assertTrue(condition: boolean, message: string): void {
    if (!condition) throw new AssertionError(message) 
  }

  static assertFalse(condition: boolean, message: string): void {
    if (condition) throw new AssertionError(message)
  }
}

function main(): int {
    let testsFailed = 0

    testsFailed += runTest(testArrayBufferDataView, "ArrayBufferView methods")

    return testsFailed == 0 ? RESULT_SUCCESS : RESULT_FAILURE
}

function testArrayBufferDataView(): int {
    let buffer = new ArrayBuffer(16)
    
    let bufferView: ArrayBufferView = new DataView(buffer, 4, 12)

    Assertions.assertSame(buffer, bufferView.buffer, "ArrayBufferView.buffer returned completely different buffer instance")
    Assertions.assertEquals(12, bufferView.byteLength, "ArrayBufferView.byteLength")
    Assertions.assertEquals(4, bufferView.byteOffset, "ArrayBufferView.byteOffset")

    return RESULT_SUCCESS
}

function runTest(test: () => int, message: String) {
  try {
    const testResult = test()

    if (testResult == RESULT_SUCCESS) console.println("PASSED: " + message)
    else console.println("FAILED: " + message);

    return testResult
  } catch (e) {
    if (e instanceof AssertionError) {
      const error = e as AssertionError
      console.println(`FAILED: '${message}' with exception: ${error.name}(${error.message})`)
    } else {
      console.println(`FAILED '${message}' with exception ` + e)
    }
    return RESULT_FAILURE
  }
}
