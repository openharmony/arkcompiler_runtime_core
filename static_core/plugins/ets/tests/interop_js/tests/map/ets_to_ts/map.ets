/**
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

package map.test;

export let myMap: Map<string, number>;
static {
  myMap = new Map<string, number>();
  myMap.set('apple', 1);
}

export function GetMapElement(key: string): number|undefined {
  return myMap.get(key);
}

export function HasMapElement(key: string): boolean {
  return myMap.has(key);
}

export function GetMapSize(): number {
  return myMap.size;
}