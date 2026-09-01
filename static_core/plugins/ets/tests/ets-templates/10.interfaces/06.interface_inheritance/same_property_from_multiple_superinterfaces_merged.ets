/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Same property from multiple superinterfaces is merged
params: "TEST-UNIT-10-0008; GAP-10-0008; CI-10-06-001-01"
assertion: "Properties with same name from multiple superinterfaces are merged"
test_suffix: "01"
---*/

interface First {
  prop1: number
  set prop2(p: number)
  get prop3(): number
}

interface Second {
  prop1: number
  set prop2(p: number)
  get prop3(): number
}

interface Merged extends First, Second {}

class MergedImpl implements Merged {
  public prop1: number = 0
  private stored2: number = 0
  private stored3: number = 3

  set prop2(p: number) {
    this.stored2 = p
  }

  get prop3(): number {
    return this.stored3
  }

  probe2(): number {
    return this.stored2
  }
}

function main(): void {
  let merged: Merged = new MergedImpl()
  merged.prop1 = 5
  arktest.assertEQ(merged.prop1, 5,
    'the merged prop1 exposes both getter and setter')
  merged.prop2 = 7
  arktest.assertEQ((merged as MergedImpl).probe2(), 7,
    'the merged prop2 setter is called')
  arktest.assertEQ(merged.prop3, 3,
    'the merged prop3 getter is called')
}
