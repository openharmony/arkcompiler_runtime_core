/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 7.19 Nullish-Coalescing Expression
 name: issue14609_02
 tags:
 ---*/

class A<T> {
  public parent: A<T>
  data: T
  constructor(data: T) {
      this.data = data
  }  
  foo() : A<T> {
    return this.parent ?? this
  }
}

function main(): void {
  let a = new A<string>("hello")
  let aa = a.foo()
  let b = new A<null>(null)
  let bb = b.foo()
  assertTrue( aa instanceof A)
  assertTrue( bb instanceof A)
}