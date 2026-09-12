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
desc: Mutually recursive generic convertible bounds is accepted by static ArkTS.
---*/

'use static'

interface ConvertibleTo<T> {
  convert(): T
}

interface TwoWayConvertible<T extends ConvertibleTo<S>, S extends ConvertibleTo<T>> {
  t2s(s: S): T
  s2t(t: T): S
}

class Incremented implements ConvertibleTo<Decremented> {
  public i: number

  constructor(i: number) {
    this.i = i + 1
  }

  convert(): Decremented {
    return new Decremented(this.i)
  }
}

class Decremented implements ConvertibleTo<Incremented> {
  public i: number

  constructor(i: number) {
    this.i = i - 1
  }

  convert(): Incremented {
    return new Incremented(this.i)
  }
}

class Intf5901 implements TwoWayConvertible<Incremented, Decremented> {
  public t2s(d: Decremented): Incremented {
    return d.convert()
  }

  public s2t(i: Incremented): Decremented {
    return i.convert()
  }
}

function main(): void {
  let converter = new Intf5901()
  arktest.assertEQ(converter.t2s(new Decremented(3)).i, 3)
  arktest.assertEQ(converter.s2t(new Incremented(3)).i, 3)
}
