/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: >-
    The value of a type T expression is always suitable for
    assignment to a type T variable; it means that unless heap
    pollution occurs
---*/

class Box<T> {
    private v: T;

    public set(v: T): void {
      this.v = v;
    }

    public get(): T {
      return this.v;
    }
}

function main(): int {
    let intBox: Box<Int> = new Box<Int>();
    intBox.set(42);

    let obj: Object = intBox;
    let stringBox: Box<String> = obj as Box<String>;

    try {
        let s: String = stringBox.get();
    } catch (e: ClassCastError) {
        return 0;
    }
    return 1;
}
