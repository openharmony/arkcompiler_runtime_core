/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Function Call Expression Subtyping
name: function_call_subtyping_0000.ets
spec: 7.11
---*/




class A<T>  {
    countA : T;

    foo() : T {
    return this.countA
    }
  
    constructor(p: T) {
    this.countA = p
  }
}

class B<T> extends A<T> {
  countB : T;

    foo() : T {
    return this.countB
    }
    
    constructor(p: T) {
    super(p)
    this.countB = p
    }
}


function f(): B<Int> {
  return new B<Int>(6);
}

function main() {
  let x : A<Int> = f()
  
  assertEQ(x.foo(),  6)
}

