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
desc: The method with the same name has the same type and number of parameters, but a method is asynchronous, redefine the fetchData method.
---*/

interface Parent1 {
  fetchDataSync(): string;
}

interface Parent2 {
  fetchDataAsync(): Promise<string>; 
}

interface Child extends Parent1, Parent2 {
  fetchDataSync(): string;
  fetchDataAsync(): Promise<string>;
}

class ChildClass implements Child {
  fetchDataSync(): string {
    return "Sync Data";
  }

  fetchDataAsync(): Promise<string> {
    return Promise.resolve("Async Data");
  }
}

function main(): void {
  const child = new ChildClass();

  const syncResult = child.fetchDataSync();

  child.fetchDataAsync().then((data: string) => console.log(data));
  assertEQ(syncResult, "Sync Data")
}