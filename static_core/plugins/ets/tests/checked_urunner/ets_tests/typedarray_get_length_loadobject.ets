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



//! CHECKER         Test check optimize LoadObjects get length of Int8Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt8ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayIterableCtor"
function testInt8ArrayIterableCtor() : int {
  let count: int = 15;
  let integerIndexes: Int8Array =
     new Int8Array([3, 14, 44, 25, 91, 38, 82, 88, 64, 81, 70, 90, 33, 63, 70]);
  let resources: Int8Array =
     new Int8Array([12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]);
  let res: Int8Array = new Int8Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 845;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Uint8Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayIterableCtor"
function testUint8ArrayIterableCtor() : int {
  let count: int = 15;
  let integerIndexes: Uint8Array =
     new Uint8Array([26, 18, 44, 33, 32, 89, 123, 97, 35, 234, 321, 21, 42, 13, 74]);
  let resources: Uint8Array =
     new Uint8Array([124, 23, 64, 34, 12, 83, 16, 33, 37, 41, 43, 31, 34, 75, 84]);
  let res: Uint8Array = new Uint8Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 809;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Uint8ClampedArray
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:
//!                   "--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ClampedArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayIterableCtor"
function testUint8ClampedArrayIterableCtor() : int {
  let count: int = 15;
  let integerIndexes: Uint8ClampedArray =
     new Uint8ClampedArray([23, 57, 43, 53, 35, 17, 28, 35, 25, 234, 165, 26, 178, 27, 78]);
  let resources: Uint8ClampedArray =
     new Uint8ClampedArray([83, 84, 78, 34, 47, 94, 95, 84, 94, 86, 95, 27, 26, 36, 18]);
  let res: Uint8ClampedArray = new Uint8ClampedArray([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1073;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Int16Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt16ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayIterableCtor"
function testInt16ArrayIterableCtor() : int {
  let count: int = 20;
  let integerIndexes: Int16Array =
     new Int16Array([36, 72, 235, 23, 52, 37, 36, 48, 35, 37, 47, 58, 37, 38, 48]);
  let resources: Int16Array =
     new Int16Array([58, 48, 68, 7, 47, 95, 53, 54, 26, 85, 43, 27, 16, 5, 67]);
  let res: Int16Array = new Int16Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1595;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Uint16Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint16ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayIterableCtor"
function testUint16ArrayIterableCtor() : int {
  let count: int = 20;
  let integerIndexes: Uint16Array =
     new Uint16Array([73, 32, 235, 63, 3, 56, 25, 16, 27, 26, 84, 26, 37, 36, 15]);
  let resources: Uint16Array =
     new Uint16Array([26, 25, 94, 74, 84, 74, 16, 26, 43, 63, 54, 63, 95, 84, 63]);
  let res: Uint16Array = new Uint16Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1615;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Int32Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt32ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayIterableCtor"
function testInt32ArrayIterableCtor() : int {
  let count: int = 20;
  let integerIndexes: Int32Array =
     new Int32Array([25, 23, 63, 15, 83, 27, 48, 37, 94, 38, 27, 58, 47, 94, 25]);
  let resources: Int32Array =
     new Int32Array([95, 36, 276, 26, 47, 48, 37, 5, 75, 36, 54, 36, 5, 26, 56]);
  let res: Int32Array = new Int32Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1001;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Uint32Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint32ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayIterableCtor"
function testUint32ArrayIterableCtor() : int {
  let count: int = 20;
  let integerIndexes: Uint32Array =
     new Uint32Array([63, 64, 26, 47, 84, 57, 64, 36, 95, 25, 64, 43, 36, 38, 96]);
  let resources: Uint32Array =
     new Uint32Array([266, 25, 34, 85, 64, 33, 36, 4, 95, 74, 25, 54, 96, 26, 54]);
  let res: Uint32Array = new Uint32Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1235;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Float32Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat32ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 1
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 0
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayIterableCtor"
function testFloat32ArrayIterableCtor() : int {
  let count: int = 20;
  let integerIndexes: Int32Array =
     new Int32Array([96, 376, 46, 85, 37, 27, 95, 65, 84, 25, 58, 83, 27, 94, 15]);
  let resources: Float32Array =
     new Float32Array([12.2, 43.5, 56.2, 76.6, 89.7, 54.9, 45.2, 32.5, 35.6, 47.2, 46.6, 44.3, 21.2, 37.6, 84.57]);
  let res: Float32Array = new Float32Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1201;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Float64Array
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat64ArrayIterableCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayIterableCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayIterableCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 3
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 0
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayIterableCtor"
function testFloat64ArrayIterableCtor() : int {
  let count: int = 20;
  let integerIndexes: Int32Array =
     new Int32Array([96, 376, 46, 85, 37, 27, 95, 65, 84, 25, 58, 83, 27, 94, 15]);
  let resources: Float64Array =
     new Float64Array([12.2, 43.5, 56.2, 76.6, 89.7, 54.9, 45.2, 32.5, 35.6, 47.2, 46.6, 44.3, 21.2, 37.6, 84.57]);
  let res: Float64Array = new Float64Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1201;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt8ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt8ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayFixedarrayCtor"
function testInt8ArrayFixedarrayCtor() : int {
  let count: int = 15;
  const nums: FixedArray<number> = [3, 14, 44, 25, 91, 38, 82, 88, 64, 81, 70, 90, 33, 63, 70];
  const ints: FixedArray<int> = [12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84];

  let integerIndexes: Int8Array = new Int8Array(nums);
  let resources: Int8Array = new Int8Array(ints);
  let res: Int8Array = new Int8Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 845;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint8ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayFixedarrayCtor"
function testUint8ArrayFixedarrayCtor() : int {
  let count: int = 15;
  const nums: FixedArray<number> = [26, 18, 44, 33, 32, 89, 123, 97, 35, 234, 321, 21, 42, 13, 74];
  const ints: FixedArray<int> = [124, 23, 64, 34, 12, 83, 16, 33, 37, 41, 43, 31, 34, 75, 84];

  let integerIndexes: Uint8Array = new Uint8Array(nums);
  let resources: Uint8Array = new Uint8Array(ints);
  let res: Uint8Array = new Uint8Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 809;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Uint8ClampedArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:
//!                   "--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ClampedArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArray"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayFixedarrayCtor"
function testUint8ClampedArrayFixedarrayCtor() : int {
  let count: int = 15;
  const nums: FixedArray<number> = [23, 57, 43, 53, 35, 17, 28, 35, 25, 234, 165, 26, 178, 27, 78];
  const ints: FixedArray<int> = [83, 84, 78, 34, 47, 94, 95, 84, 94, 86, 95, 27, 26, 36, 18];

  let integerIndexes: Uint8ClampedArray = new Uint8ClampedArray(nums);
  let resources: Uint8ClampedArray = new Uint8ClampedArray(ints);
  let res: Uint8ClampedArray = new Uint8ClampedArray([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1073;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Int16ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt16ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayFixedarrayCtor"
function testInt16ArrayFixedarrayCtor() : int {
  let count: int = 20;
  const nums: FixedArray<number>  = [36, 72, 235, 23, 52, 37, 36, 48, 35, 37, 47, 58, 37, 38, 48];
  const ints: FixedArray<int> = [58, 48, 68, 7, 47, 95, 53, 54, 26, 85, 43, 27, 16, 5, 67];

  let integerIndexes: Int16Array = new Int16Array(nums);
  let resources: Int16Array = new Int16Array(ints);
  let res: Int16Array = new Int16Array([0, 0, 0, 0, 0]);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1595;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Uint16ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint16ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayFixedarrayCtor"
function testUint16ArrayFixedarrayCtor() : int {
  let count: int = 20;
  const nums: FixedArray<number> = [73, 32, 235, 63, 3, 56, 25, 16, 27, 26, 84, 26, 37, 36, 15];
  const ints: FixedArray<int> = [26, 25, 94, 74, 84, 74, 16, 26, 43, 63, 54, 63, 95, 84, 63];
  let res: Uint16Array = new Uint16Array([0, 0, 0, 0, 0]);

  let integerIndexes: Uint16Array = new Uint16Array(nums);
  let resources: Uint16Array = new Uint16Array(ints);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1615;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Int32ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt32ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayFixedarrayCtor"
function testInt32ArrayFixedarrayCtor() : int {
  let count: int = 20;
  const nums: FixedArray<number> = [25, 23, 63, 15, 83, 27, 48, 37, 94, 38, 27, 58, 47, 94, 25];
  const ints: FixedArray<int> = [95, 36, 276, 26, 47, 48, 37, 5, 75, 36, 54, 36, 5, 26, 56];
  let res: Int32Array = new Int32Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int32Array = new Int32Array(nums);
  let resources: Int32Array = new Int32Array(ints);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1001;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}


//! CHECKER         Test check optimize LoadObjects get length of Uint32ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint32ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayFixedarrayCtor"
function testUint32ArrayFixedarrayCtor() : int {
  let count: int = 20;
  const nums: FixedArray<number> = [63, 64, 26, 47, 84, 57, 64, 36, 95, 25, 64, 43, 36, 38, 96];
  const ints: FixedArray<int> = [266, 25, 34, 85, 64, 33, 36, 4, 95, 74, 25, 54, 96, 26, 54];
  let res: Uint32Array = new Uint32Array([0, 0, 0, 0, 0]);

  let integerIndexes: Uint32Array = new Uint32Array(nums);
  let resources: Uint32Array = new Uint32Array(ints);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1235;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Float32ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat32ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 1
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 0
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayFixedarrayCtor"
function testFloat32ArrayFixedarrayCtor() : int {
  let count: int = 20;
  const ints: FixedArray<int> =
      [96, 376, 46, 85, 37, 27, 95, 65, 84, 25, 58, 83, 27, 94, 15];
  const nums: FixedArray<number> =
      [12.2, 43.5, 56.2, 76.6, 89.7, 54.9, 45.2, 32.5, 35.6, 47.2, 46.6, 44.3, 21.2, 37.6, 84.57];
  let res: Float32Array = new Float32Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int32Array = new Int32Array(ints);
  let resources: Float32Array = new Float32Array(nums);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1201;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of Float64ArrayFixedarrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat64ArrayFixedarrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayFixedarrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayFixedarrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 3
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 0
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 0
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayFixedarrayCtor"
function testFloat64ArrayFixedarrayCtor() : int {
  let count: int = 20;
  const ints: FixedArray<int> =
      [96, 376, 46, 85, 37, 27, 95, 65, 84, 25, 58, 83, 27, 94, 15];
  const nums: FixedArray<number> =
      [12.2, 43.5, 56.2, 76.6, 89.7, 54.9, 45.2, 32.5, 35.6, 47.2, 46.6, 44.3, 21.2, 37.6, 84.57];
  let res: Float64Array = new Float64Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int32Array = new Int32Array(ints);
  let resources: Float64Array = new Float64Array(nums);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1201;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt8ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt8ArrayFixedlengthCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayFixedlengthCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayFixedlengthCtor"
function testInt8ArrayFixedlengthCtor() : int {
  let count: int = 15;

  let integerIndexes: Int8Array = new Int8Array(20);
  let resources: Int8Array = new Int8Array(20);
  let res: Int8Array = new Int8Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(24);
  resources.fill(32);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 361;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint8ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ArrayFixedlengthCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayFixedlengthCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayFixedlengthCtor"
function testUint8ArrayFixedlengthCtor() : int {
  let count: int = 15;

  let integerIndexes: Uint8Array = new Uint8Array(20);
  let resources: Uint8Array = new Uint8Array(20);
  let res: Uint8Array = new Uint8Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(12);
  resources.fill(27);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 181;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt16ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt16ArrayFixedlengthCtor.*'",
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayFixedlengthCtor"
function testInt16ArrayFixedlengthCtor() : int {
  let count: int = 20;
  let integerIndexes: Int16Array = new Int16Array(20);
  let resources: Int16Array = new Int16Array(20);
  let res: Int16Array = new Int16Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(15);
  resources.fill(31);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 301;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint16ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint16ArrayFixedlengthCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayFixedlengthCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayFixedlengthCtor"
function testUint16ArrayFixedlengthCtor() : int {
  let count: int = 20;
  let integerIndexes: Uint16Array = new Uint16Array(20);
  let resources: Uint16Array = new Uint16Array(20);
  let res: Uint16Array = new Uint16Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(15);
  resources.fill(31);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 301;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt32ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt32ArrayFixedlengthCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayFixedlengthCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayFixedlengthCtor"
function testInt32ArrayFixedlengthCtor() : int {
  let count: int = 20;
  let integerIndexes: Int32Array = new Int32Array(20);
  let resources: Int32Array = new Int32Array(20);
  let res: Int32Array = new Int32Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(16);
  resources.fill(27);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 321;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint32ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint32ArrayFixedlengthCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayFixedlengthCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayFixedlengthCtor"
function testUint32ArrayFixedlengthCtor() : int {
  let count: int = 20;
  let integerIndexes: Uint32Array = new Uint32Array(20);
  let resources: Uint32Array = new Uint32Array(20);
  let res: Uint32Array = new Uint32Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(29);
  resources.fill(31);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 581;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testFloat32ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat32ArrayFixedlengthCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayFixedlengthCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 1
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 0
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayFixedlengthCtor"
function testFloat32ArrayFixedlengthCtor() : int {
  let count: int = 20;
  let integerIndexes: Int8Array = new Int8Array(20);
  let resources: Float32Array = new Float32Array(20);
  let res: Float32Array = new Float32Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(11);
  resources.fill(22.2);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 221;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testFloat64ArrayFixedlengthCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat64ArrayFixedlengthCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayFixedlengthCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayFixedlengthCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 3
//! PASS_AFTER_NEXT "ChecksElimination"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 0
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 0
//! INST_COUNT      "BoundsCheck D", 0
//! INST_COUNT      "BoundsCheck", 0
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayFixedlengthCtor"
function testFloat64ArrayFixedlengthCtor() : int {
  let count: int = 20;
  let integerIndexes: Int8Array = new Int8Array(20);
  let resources: Float64Array = new Float64Array(20);
  let res: Float64Array = new Float64Array([0, 0, 0, 0, 0]);
  integerIndexes.fill(26);
  resources.fill(15.4);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 521;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt8ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt8ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayTypedArrayCtor"
function testInt8ArrayTypedArrayCtor() : int {
  let count: int = 15;
  let integerIndexesOriginal: Int8Array = new Int8Array([3, 14, 44, 25, 91, 38, 82, 88, 64, 81, 70, 90, 33, 63, 70]);
  let resourcesOriginal: Int8Array = new Int8Array([12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]);
  let res: Int8Array = new Int8Array([0, 0, 0, 0, 0]);
  let integerIndexes: Int8Array = new Int8Array(integerIndexesOriginal);
  let resources: Int8Array = new Int8Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 845;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint8ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayTypedArrayCtor"
function testUint8ArrayTypedArrayCtor() : int {
  let count: int = 15;
  let integerIndexesOriginal: Uint8Array =
     new Uint8Array([26, 18, 44, 33, 32, 89, 123, 97, 35, 234, 321, 21, 42, 13, 74]);
  let resourcesOriginal: Uint8Array =
     new Uint8Array([124, 23, 64, 34, 12, 83, 16, 33, 37, 41, 43, 31, 34, 75, 84]);
  let res: Uint8Array = new Uint8Array([0, 0, 0, 0, 0]);
  let integerIndexes: Uint8Array = new Uint8Array(integerIndexesOriginal);
  let resources: Uint8Array = new Uint8Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 809;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint8ClampedArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:
                      "--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ClampedArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"",entry:"typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayTypedArrayCtor"
function testUint8ClampedArrayTypedArrayCtor() : int {
  let count: int = 15;
  let integerIndexesOriginal: Uint8ClampedArray =
     new Uint8ClampedArray([23, 57, 43, 53, 35, 17, 28, 35, 25, 234, 165, 26, 178, 27, 78]);
  let resourcesOriginal: Uint8ClampedArray =
     new Uint8ClampedArray([83, 84, 78, 34, 47, 94, 95, 84, 94, 86, 95, 27, 26, 36, 18]);
  let res: Uint8ClampedArray = new Uint8ClampedArray([0, 0, 0, 0, 0]);
  let integerIndexes: Uint8ClampedArray = new Uint8ClampedArray(integerIndexesOriginal);
  let resources: Uint8ClampedArray = new Uint8ClampedArray(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1073;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt16ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt16ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayTypedArrayCtor"
function testInt16ArrayTypedArrayCtor() : int {
  let count: int = 20;
  let integerIndexesOriginal: Int16Array =
     new Int16Array([36, 72, 235, 23, 52, 37, 36, 48, 35, 37, 47, 58, 37, 38, 48]);
  let resourcesOriginal: Int16Array =
     new Int16Array([58, 48, 68, 7, 47, 95, 53, 54, 26, 85, 43, 27, 16, 5, 67]);
  let res: Int16Array = new Int16Array([0, 0, 0, 0, 0]);
  let integerIndexes: Int16Array = new Int16Array(integerIndexesOriginal);
  let resources: Int16Array = new Int16Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1595;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint16ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint16ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayTypedArrayCtor"
function testUint16ArrayTypedArrayCtor() : int {
  let count: int = 20;
  let integerIndexesOriginal: Uint16Array =
     new Uint16Array([73, 32, 235, 63, 3, 56, 25, 16, 27, 26, 84, 26, 37, 36, 15]);
  let resourcesOriginal: Uint16Array =
     new Uint16Array([26, 25, 94, 74, 84, 74, 16, 26, 43, 63, 54, 63, 95, 84, 63]);
  let res: Uint16Array = new Uint16Array([0, 0, 0, 0, 0]);
  let integerIndexes: Uint16Array = new Uint16Array(integerIndexesOriginal);
  let resources: Uint16Array = new Uint16Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1615;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}


//! CHECKER         Test check optimize LoadObjects get length of testInt32ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt32ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayTypedArrayCtor"
function testInt32ArrayTypedArrayCtor() : int {
  let count: int = 20;
  let integerIndexesOriginal: Int32Array =
     new Int32Array([25, 23, 63, 15, 83, 27, 48, 37, 94, 38, 27, 58, 47, 94, 25]);
  let resourcesOriginal: Int32Array =
     new Int32Array([95, 36, 276, 26, 47, 48, 37, 5, 75, 36, 54, 36, 5, 26, 56]);
  let res: Int32Array = new Int32Array([0, 0, 0, 0, 0]);
  let integerIndexes: Int32Array = new Int32Array(integerIndexesOriginal);
  let resources: Int32Array = new Int32Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1001;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint32ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint32ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32Array_typedarray_ctor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayTypedArrayCtor"
function testUint32ArrayTypedArrayCtor() : int {
  let count: int = 20;
  let integerIndexesOriginal: Uint32Array =
     new Uint32Array([63, 64, 26, 47, 84, 57, 64, 36, 95, 25, 64, 43, 36, 38, 96]);
  let resourcesOriginal: Uint32Array =
     new Uint32Array([266, 25, 34, 85, 64, 33, 36, 4, 95, 74, 25, 54, 96, 26, 54]);
  let res: Uint32Array = new Uint32Array([0, 0, 0, 0, 0]);
  let integerIndexes: Uint32Array = new Uint32Array(integerIndexesOriginal);
  let resources: Uint32Array = new Uint32Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1235;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testFloat32ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat32ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 1
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 2
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayTypedArrayCtor"
function testFloat32ArrayTypedArrayCtor() : int {
  let count: int = 20;
  let integerIndexesOriginal: Int32Array =
     new Int32Array([96, 376, 46, 85, 37, 27, 95, 65, 84, 25, 58, 83, 27, 94, 15]);
  let resourcesOriginal: Float32Array =
     new Float32Array([12.2, 43.5, 56.2, 76.6, 89.7, 54.9, 45.2, 32.5, 35.6, 47.2, 46.6, 44.3, 21.2, 37.6, 84.57]);
  let res: Float32Array = new Float32Array([0, 0, 0, 0, 0]);
  let integerIndexes: Int32Array = new Int32Array(integerIndexesOriginal);
  let resources: Float32Array = new Float32Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1201;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testFloat64ArrayTypedArrayCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat64ArrayTypedArrayCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayTypedArrayCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayTypedArrayCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 3
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 2
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayTypedArrayCtor"
function testFloat64ArrayTypedArrayCtor() : int {
  let count: int = 20;
  let integerIndexesOriginal: Int32Array =
     new Int32Array([96, 376, 46, 85, 37, 27, 95, 65, 84, 25, 58, 83, 27, 94, 15]);
  let resourcesOriginal: Float64Array =
     new Float64Array([12.2, 43.5, 56.2, 76.6, 89.7, 54.9, 45.2, 32.5, 35.6, 47.2, 46.6, 44.3, 21.2, 37.6, 84.57]);
  let res: Float64Array = new Float64Array([0, 0, 0, 0, 0]);
  let integerIndexes: Int32Array = new Int32Array(integerIndexesOriginal);
  let resources: Float64Array = new Float64Array(resourcesOriginal);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 1201;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt8ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt8ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int8Array\.lengthInt/, 5
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt8ArrayArrayBufferCtor"
function testInt8ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40);
  let res: Int8Array = new Int8Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int8Array = new Int8Array(integerIndexesBuffer);
  let resources: Int8Array = new Int8Array(resourcesBuffer, 2, 20);
  integerIndexes.fill(24);
  resources.fill(32);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 361;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint8ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ArrayArrayBufferCtor"
function testUint8ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40);
  let res: Uint8Array = new Uint8Array([0, 0, 0, 0, 0]);

  let integerIndexes: Uint8Array = new Uint8Array(integerIndexesBuffer);
  let resources: Uint8Array = new Uint8Array(resourcesBuffer, 2, 20);
  integerIndexes.fill(11);
  resources.fill(42);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 166;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint8ClampedArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:
//!                   "--compiler-regex='.*typedarray_get_length_loadobject.*testUint8ClampedArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint8ClampedArray\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"",entry:"typedarray_get_length_loadobject.ETSGLOBAL::testUint8ClampedArrayArrayBufferCtor"
function testUint8ClampedArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40);
  let res: Uint8ClampedArray = new Uint8ClampedArray([0, 0, 0, 0, 0]);

  let integerIndexes: Uint8ClampedArray = new Uint8ClampedArray(integerIndexesBuffer);
  let resources: Uint8ClampedArray = new Uint8ClampedArray(resourcesBuffer, 2, 20);
  integerIndexes.fill(21);
  resources.fill(37);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 316;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt16ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt16ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int16Array\.lengthInt/, 5
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt16ArrayArrayBufferCtor"
function testInt16ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20 * 2);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40 * 2);
  let res: Int16Array = new Int16Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int16Array = new Int16Array(integerIndexesBuffer);
  let resources: Int16Array = new Int16Array(resourcesBuffer, 2, 20);
  integerIndexes.fill(5);
  resources.fill(22);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 76;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint16ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint16ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint16Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint16ArrayArrayBufferCtor"
function testUint16ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20 * 2);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40 * 2);
  let res: Int16Array = new Int16Array([0, 0, 0, 0, 0]);

  let integerIndexes: Uint16Array = new Uint16Array(integerIndexesBuffer);
  let resources: Uint16Array = new Uint16Array(resourcesBuffer, 2, 20);
  integerIndexes.fill(17);
  resources.fill(21);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 256;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testInt32ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testInt32ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 5
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testInt32ArrayArrayBufferCtor"
function testInt32ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20 * 4);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40 * 4);
  let res: Int32Array = new Int32Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int32Array = new Int32Array(integerIndexesBuffer);
  let resources: Int32Array = new Int32Array(resourcesBuffer, 4, 20);
  integerIndexes.fill(26);
  resources.fill(14);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 391;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testUint32ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testUint32ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Uint32Array\.lengthInt/, 4
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testUint32ArrayArrayBufferCtor"
function testUint32ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20 * 4);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40 * 4);
  let res: Uint32Array = new Uint32Array([0, 0, 0, 0, 0]);

  let integerIndexes: Uint32Array = new Uint32Array(integerIndexesBuffer);
  let resources: Uint32Array = new Uint32Array(resourcesBuffer, 4, 20);
  integerIndexes.fill(21);
  resources.fill(15);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 316;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testFloat32ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat32ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 1
//! INST_COUNT      "BoundsCheck D", 2
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      /LoadObject.*escompat\.Float32Array\.lengthInt/, 3
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat32ArrayArrayBufferCtor"
function testFloat32ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20 * 4);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40 * 4);
  let res: Float32Array = new Float32Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int32Array = new Int32Array(integerIndexesBuffer);
  let resources: Float32Array = new Float32Array(resourcesBuffer, 4, 20);
  integerIndexes.fill(31);
  resources.fill(10.5);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 466;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}

//! CHECKER         Test check optimize LoadObjects get length of testFloat64ArrayArrayBufferCtor
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options:"--compiler-regex='.*typedarray_get_length_loadobject.*testFloat64ArrayArrayBufferCtor.*'",
//!                 entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayArrayBufferCtor"
//! METHOD          "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayArrayBufferCtor"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 1
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 2
//! INST_COUNT      "BoundsCheck D", 3
//! PASS_AFTER_NEXT "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /LoadObject.*escompat\.Int32Array\.lengthInt/, 2
//! INST_COUNT      /LoadObject.*escompat\.Float64Array\.lengthInt/, 3
//! PASS_AFTER      "ChecksElimination"
//! INST_COUNT      "BoundsCheck D", 2
//! INST_COUNT      "BoundsCheck", 2
//! RUN             options:"", entry: "typedarray_get_length_loadobject.ETSGLOBAL::testFloat64ArrayArrayBufferCtor"
function testFloat64ArrayArrayBufferCtor() : int {
  let count: int = 15;
  let integerIndexesBuffer: ArrayBuffer = new ArrayBuffer(20 * 8);
  let resourcesBuffer: ArrayBuffer = new ArrayBuffer(40 * 8);
  let res: Float64Array = new Float64Array([0, 0, 0, 0, 0]);

  let integerIndexes: Int32Array = new Int32Array(integerIndexesBuffer);
  let resources: Float64Array = new Float64Array(resourcesBuffer, 8, 20);
  integerIndexes.fill(21);
  resources.fill(17.5);
  let num: int = 1;
  let indexLength: int = integerIndexes.length.toInt() - 1;
  let resourcesLength: int = resources.length.toInt() - 1;
  let resLength: int = res.length.toInt() - 1;
  let finalNum: int = 316;
  for (let i = 0; i < count; i++) {
    num += integerIndexes[i % num & indexLength];
    res[i & resLength] = resources[i % num & resourcesLength];
  }
  arktest.assertEQ(num, finalNum);
  return 0;
}
