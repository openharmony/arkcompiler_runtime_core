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
desc: Record index expression - the keys of the record are dynamically generated.
---*/

let dynamicKeyRecord: Record<string, number> = {};

function setValue(key: string, value: number) {
  dynamicKeyRecord[key] = value;
}

function main(): void {
  setValue("key1", 10);
  setValue("key2", 20);
  assertEQ(dynamicKeyRecord["key1"], 10);
}