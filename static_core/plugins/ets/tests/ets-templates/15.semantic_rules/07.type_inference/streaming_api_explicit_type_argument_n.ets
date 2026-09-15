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
desc: Streaming api explicit type argument is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0273: Expected 0 type arguments, got 1 .
    - >-
        Semantic error ESE0127: No matching call signature for filter(((p1: A) => Boolean))
---*/

'use static'

/*
 * TC-MC-002: 流式API泛型
 * Expected 0 type arguments, got 1
 * No matching call signature for filter
 * 预期结果: CTE编译错误
 * 关联测试点: 方法链-流式
 */

class ListWrapper<T> {
  public filter(fn: (t: T) => boolean): ListWrapper<T> {
    return this
  }

  public map<U>(fn: (t: T) => U): ListWrapper<U> {
    return new ListWrapper<U>()
  }
}

class A {
  public value: string = ''
}

class B extends A {
  public name: string = ''
}

class C extends B {
  public id: int = 0
}

function process(): void {
  let list: ListWrapper<A> = new ListWrapper<A>()
  let result: ListWrapper<B> = list.filter<A>((x: A): boolean => {
    return true
  }).map<B>((x: A): B => {
    return new B()
  })
  console.log(result.toString())
}

function main(): void {
  process()
}
