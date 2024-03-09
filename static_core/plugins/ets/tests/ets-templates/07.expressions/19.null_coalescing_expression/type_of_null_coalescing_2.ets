/*---
Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: null safety -> Null-Coalescing Operator.
---*/

class A {}
class BA extends A {}
class CBA extends BA {}
class DA extends A {}

function main(): void {
  let a: CBA = new CBA();
  let b: DA = new DA();
  let c = a ?? b;
  assert c instanceof CBA;

  let d: A = new A();
  let e = a ?? d;
  assert e instanceof CBA;
  
  let f: BA = new BA();
  let g = a ?? f;
  assert e instanceof BA;
}
