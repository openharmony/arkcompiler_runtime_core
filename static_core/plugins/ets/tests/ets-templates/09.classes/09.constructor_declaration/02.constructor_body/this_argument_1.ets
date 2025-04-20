/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: this can be passed as an argument no matter if fields are inited. If an uninited fields is used. NullPointerError should happen
---*/

class A {
  public n: number
  public s: string

  foo(a: A) {
    return a.s.length
  }

  public constructor() {
    this.n = this.foo(this)
    this.s = "abcd"
  }
}

function main(){
  try{
    let a = new A
  } catch (e) {
    arktest.assertTrue( e instanceof NullPointerError )
    return
  }
  arktest.assertTrue(false, "NPE expected")
}