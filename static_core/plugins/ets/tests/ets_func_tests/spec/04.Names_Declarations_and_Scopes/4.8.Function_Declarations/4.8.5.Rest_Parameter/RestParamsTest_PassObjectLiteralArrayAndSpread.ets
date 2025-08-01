/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
desc: A rest parameter allows to make a function or method that take an unbounded
  number of arguments. A rest parameter is marked with ... symbol before parameter
  name.
name: spec/rest-params/RestParamsTest_PassObjectLiteralArrayAndSpread.ets

---*/

class Base {
    public spread(): int {
        return 1;
    }
}

class TupleT {
    index: int;
    name: String = "";
}


class Testee extends Base {

    public spread(prefix: String, ...values: TupleT[]): int {
        return 42;
    }

}

function main(): int {

    let test = new Testee();
    let result: int = 0;

    let tuples: TupleT[] = [{index: 11, name: "Jane"}, {index: 0, name: "John"}];
    result = test.spread("Double Tuple", ...tuples);
    if(result != 42) return 1;

    return 0;

}
