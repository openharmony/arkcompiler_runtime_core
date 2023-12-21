/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

  failures += test(createArrayBufferOfVariousLengths(), "Create ArrayBuffer of Various lengths");
  failures += test(sliceArrayBufferInt(), "Slice ArrayBuffer with various int parameters");
  failures += test(sliceArrayBufferNumberNegativeEnd(), "Slice ArrayBuffer with negative end number parameters"); 
  failures += test(sliceArrayBufferNumberNegativeStart(), "Slice ArrayBuffer with negative start number parameters"); 
  failures += test(sliceArrayBufferNumberStartExceedsLength(), "Slice ArrayBuffer with start number parameter exceeds length"); 
  failures += test(sliceArrayBufferNumberEndExceedsLength(), "Slice ArrayBuffer with end number parameter exceeds length"); 
  failures += test(resizeArrayBufferInt(), "Resize ArrayBuffer with various int parameters");
  failures += test(isViewArrayBuffer(), "isView ArrayBuffer with various parameters");
  return failures == 0 ? 0 : 1;
}
/**
 *  Check that created empty ArrayBuffer has zero byteLength
 */
function createEmptyArrayBuffer(): int {
  let a = new ArrayBuffer(0);
  return a.byteLength ? 1 : 0;
}

/**
 * Check created ArrayBuffer with different lengths
 */
function createArrayBufferOfVariousLengths(): int {
  let a = new ArrayBuffer(10);

  try {
    a = new ArrayBuffer(1024 * 1024 * 1024 * 2);
    return 1;
  }
  catch(e) {
    if (!(e instanceof NegativeArraySizeException))
      return 1;
  }

  try {
    a = new ArrayBuffer(1024 * 1024 * 1024 * 4);
    return 1;
  }
  catch(e) {
    if (!(e instanceof NegativeArraySizeException))
      return 1;
  }

  return 0;
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
  let a = new ArrayBuffer(8);
  
  let start = 2 as number;
  let end = -4 as number;
  if (a.slice(start, 0 as number).byteLength != 2)
    return 1;
  
  start = 2 as number;
  end = -10 as number;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
  start = 2 as number;
  end = Number.NEGATIVE_INFINITY;
  if (a.slice(start, end).byteLength != 5)
    return 1;
  
  start = 0 as number;
  end = Number.NaN;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
  return 0;
}

/**
* Check slice() method of ArrayBuffer with negative start number parameters
*/
function sliceArrayBufferNumberNegativeStart(): int {
  let a = new ArrayBuffer(8);
  
  let start = -5 as number;
  let end = 6 as number;
  if (a.slice(start, end).byteLength != 3)
    return 1;
  
  start = -12 as number;
  end = 6 as number;
  if (a.slice(start, end).byteLength != 6)
    return 1;
  
  start = Number.NEGATIVE_INFINITY;
  end = 6 as number;
  if (a.slice(start, end).byteLength != 6)
    return 1;
  
  start = Number.NaN;
  end = 0;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
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
* Check that resize() method of ArrayBuffer increase buffer and byteLength correspondingly
*/
function resizeArrayBufferInt(): int {
  let a = new ArrayBuffer(8);
  
  a.resize(12);
  let b = a.at(11);
  if (a.byteLength != 12)
    return 1;

  try {
    a.resize(0);
    if (a.byteLength != 0)
      return 1;
  }
  catch(e) {
    return 1;
  }
  
  return 0;
}

/**
 * Check isView() static method of ArrayBuffer
 */
function isViewArrayBuffer(): int {
  if (ArrayBuffer.isView({}))
    return 1;

  if (ArrayBuffer.isView(new ArrayBuffer(0)))
    return 1;

  if (ArrayBuffer.isView(""))
    return 1;

  if (!ArrayBuffer.isView(new Int8Array) ||
    !ArrayBuffer.isView(new Int16Array) ||
    !ArrayBuffer.isView(new Int32Array) ||
    !ArrayBuffer.isView(new BigInt64Array) ||
    !ArrayBuffer.isView(new Float32Array) ||
    !ArrayBuffer.isView(new Float64Array)
    // NOTE: no support for unsigned TypedArrays and DataView yet
    )
    return 1;

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
