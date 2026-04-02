/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: A class that implements an interface with properties can also use a field or an accessor notation
---*/

interface I {
  set text(s: string)
}

class C implements I {
  private _text: string = "str"
  get text(): string {
    return this._text
  }
  set text(i: string) {
    this._text = i
  }
}

function main() {
  let i: I = new C()
  arktest.assertEQ(i.text, "str")
  i.text = "str2"
  arktest.assertEQ(i.text, "str2")
}
