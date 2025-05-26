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
 desc: 5. Generics
 tags: [compile-only]
 ---*/

abstract class A<T> {
    abstract foo<T>() : void
}

class B<T> extends A<T> {
    foo<T>() : void {}
}

function main() : int {
    let a: B<[number, int, long]> = new B<[number, int, long]>()
    return 0;
}