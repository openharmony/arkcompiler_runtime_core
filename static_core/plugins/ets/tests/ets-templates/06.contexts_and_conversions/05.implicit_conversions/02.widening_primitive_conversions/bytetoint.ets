/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Casting conversions byte to int.
---*/

function main(): void {
  let byteArray: Uint8Array = new Uint8Array([0x12, 0x34, 0x56, 0x78]);
  let dataView = new DataView(byteArray.buffer);
  let intValue = dataView.getInt32(0, false);
  if (intValue === 0x12345678) {
    console.log("Test Passed: Byte array successfully converted to int.");
  } else {
    console.error("Test Failed: Byte array conversion to int failed.");
  }
}