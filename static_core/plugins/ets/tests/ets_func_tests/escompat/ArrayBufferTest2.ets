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

function main(): int {
  let failures: int = 0;

  failures += test(createArrayBufferOfVariousLengths, "Create ArrayBuffer of Various lengths");
  failures += test(sliceArrayBufferInt(), "Slice ArrayBuffer with various int parameters");
  failures += test(sliceArrayBufferNumberNegativeEnd, "Slice ArrayBuffer with negative end number parameters"); 
  failures += test(sliceArrayBufferNumberNegativeStart, "Slice ArrayBuffer with negative start number parameters");
  failures += test(sliceArrayBufferNumberStartExceedsLength(), "Slice ArrayBuffer with start number parameter exceeds length");
  failures += test(sliceArrayBufferNumberEndExceedsLength(), "Slice ArrayBuffer with end number parameter exceeds length");
  failures += test(isViewArrayBuffer, "isView ArrayBuffer with various parameters");
  return failures == 0 ? 0 : 1;
}
/**
 *  Check that created empty ArrayBuffer has zero byteLength
 */
function createEmptyArrayBuffer(): int {
  let a = new ArrayBuffer(0);
  return a.byteLength ? 1 : 0;
}

class Assertions {
  static assertEquals(expected: int, actual: int): void {
    if (expected != actual) throw new AssertionError(`${expected} != ${actual}`)
  }

  static assertEquals(expected: number, actual: number): void {
    if (expected != actual) throw new AssertionError(`${expected} != ${actual}`)
  }

  static assertTrue(condition: boolean): void {
    if (!condition) throw new AssertionError("expected true but was false") 
  }

  static assertFalse(condition: boolean): void {
    if (condition) throw new AssertionError("expected false but was true")
  }
}

/**
 * Check created ArrayBuffer with different lengths
 */
function createArrayBufferOfVariousLengths(): int {
  let buf = new ArrayBuffer(10);
  Assertions.assertEquals(10, buf.byteLength)

  try {
    new ArrayBuffer(-1)
    return 1
  } catch (e) {
    if (!(e instanceof RangeError)) return 1
    // expected
  }

  return 0
}

/**
* Check slice() method of ArrayBuffer with different int parameters
*/
function sliceArrayBufferInt(): int {
  let a = new ArrayBuffer(8);
  
  let start = 6;
  if (a.slice(start).byteLength != 2)
    return 1;
  
  return 0;
}

/**
* Check slice() method of ArrayBuffer with negative end number parameters
*/
function sliceArrayBufferNumberNegativeEnd(): int {
  let buffer = new ArrayBuffer(8);
  
  Assertions.assertEquals(2, buffer.slice(2 as number, -4 as number).byteLength)
  Assertions.assertEquals(0, buffer.slice(2 as number, -10 as number).byteLength)
  Assertions.assertEquals(0, buffer.slice(2 as number, Number.NEGATIVE_INFINITY).byteLength)
  Assertions.assertEquals(0, buffer.slice(0 as number, Number.NaN).byteLength)
  
  return 0;
}

/**
* Check slice() method of ArrayBuffer with negative start number parameters
*/
function sliceArrayBufferNumberNegativeStart(): int {
  let buffer = new ArrayBuffer(8);
  
  Assertions.assertEquals(3, buffer.slice(-5 as number, 6 as number).byteLength)
  Assertions.assertEquals(6, buffer.slice(-12 as number, 6 as number).byteLength)
  Assertions.assertEquals(6, buffer.slice(Number.NEGATIVE_INFINITY, 6 as number).byteLength)
  Assertions.assertEquals(0, buffer.slice(Number.NaN, 0).byteLength)
  Assertions.assertEquals(0, buffer.slice(Number.POSITIVE_INFINITY).byteLength)
  
  return 0;
}

/**
* Check slice() method of ArrayBuffer with start number parameter exceeds length
*/
function sliceArrayBufferNumberStartExceedsLength(): int {
  let a = new ArrayBuffer(8);
  
  let start = 0x100000000 as number;
  let end = 7 as number;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
  start = Number.POSITIVE_INFINITY;
  end = 6 as number;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
  return 0;
}

/**
* Check slice() method of ArrayBuffer with end number parameter exceeds length
*/
function sliceArrayBufferNumberEndExceedsLength(): int {
  let a = new ArrayBuffer(8);
  
  let start = 1 as number;
  let end = 12 as number;
  if (a.slice(start, end).byteLength != 7)
    return 1;
  
  start = 2 as number;
  end = 0x100000000 as number;
  if (a.slice(start, end).byteLength != 6)
    return 1;
    
  start = 3 as number;
  end = Number.POSITIVE_INFINITY;
  if (a.slice(start, end).byteLength != 5)
    return 1;
  
  return 0;
}

/**
 * Check isView() static method of ArrayBuffer
 */
function isViewArrayBuffer(): int {
  Assertions.assertFalse(ArrayBuffer.isView({}))
  Assertions.assertFalse(ArrayBuffer.isView(new ArrayBuffer(0)))
  Assertions.assertFalse(ArrayBuffer.isView(""))

  Assertions.assertTrue(ArrayBuffer.isView(new Int8Array))
  Assertions.assertTrue(ArrayBuffer.isView(new Int16Array))
  Assertions.assertTrue(ArrayBuffer.isView(new Int32Array))
  Assertions.assertTrue(ArrayBuffer.isView(new BigInt64Array))
  Assertions.assertTrue(ArrayBuffer.isView(new Float32Array))
  Assertions.assertTrue(ArrayBuffer.isView(new Float64Array))

  Assertions.assertTrue(ArrayBuffer.isView(new Uint8Array))
  Assertions.assertTrue(ArrayBuffer.isView(new Uint8ClampedArray))
  Assertions.assertTrue(ArrayBuffer.isView(new Uint16Array))
  Assertions.assertTrue(ArrayBuffer.isView(new Uint32Array))
  Assertions.assertTrue(ArrayBuffer.isView(new BigUint64Array))

  return 0;
}

function test(result: int, message: String): int {
  if(result == 0) {
    console.println("PASSED: " + message);
    return 0;
  }
  console.println("FAILED: " + message);
  return 1;
}

function test(action: () => int, message: String) {
  try {
    const result = action()

    if (result == 0) console.println("PASSED: " + message)
    else console.println("FAILED: " + message);

    return result
  } catch (e) {
    if (e instanceof AssertionError) {
      let error = e as AssertionError
      console.println(`FAILED: '${message}' with exception: ${error.name}(${error.message})`)
    } else {
      console.println(`FAILED '${message}' with exception ` + e)
    }
    return 1
  }
}
