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

class TestClass {}
enum ColorSet { Red, Green, Blue }
class MyClass {
  public score: string = 'good';
  protected testObj: TestClass = new TestClass();
  private levelArray: string[] = ['A', 'B', 'C'];
  public static myEnum: ColorSet = ColorSet.Red;
  private static myMap: Map<string, number> = new Map<string, number>();
  public mySet: Set<number> = new Set<number>();
}

class ParentTest {
  public id: string = '001';
}

class PersonTest extends ParentTest {
  public name: string = 'Alice';
  public age: number = 30;

  showName() {
    console.log(this.name);
  }
}

function main() {
  let c = new PersonTest();
  c.showName();
}

