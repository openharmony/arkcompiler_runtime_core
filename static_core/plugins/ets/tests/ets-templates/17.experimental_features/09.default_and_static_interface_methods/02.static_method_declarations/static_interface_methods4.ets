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
desc: Generic interfaces is OK
---*/

interface I<T> {
    static undef2Null<T>(p: T|undefined|null): T|null {
        return p === undefined ? null : p
    }

    undef2Null1<T>(p: T|undefined|null): T|null {
        return p === undefined ? null : p
    }
}

class C<T> implements I<T> {}

function main(): void {
  let c = new C<Int>();
  assert(c.undef2Null1(null) == null);
  assert(c.undef2Null1(undefined) == null);
  assert(c.undef2Null1(new Int(6)) == 6);

  assert(C.undef2Null(null) == null);
  assert(C.undef2Null(undefined) == null)
  assert(C.undef2Null<Int>(new Int(6)) == 6)
}
