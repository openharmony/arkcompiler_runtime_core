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
name: spec/rest-params/RestParamsTest_CheckShadowing.ets

---*/

interface Supplier {
    get(): int;
}

class SupplierImpl implements Supplier {
    override get(): int {
        return 3;
    }
}

type SupplierN = Supplier|null;

class SupplierImplForShadow implements Supplier {
    override get(): int {
        return 2;
    }
}

class Base {
}


class Testee extends Base {


    public callShadow(...shadow: SupplierN[]): int {
        let sum: int = 0;
        for(let call of shadow) {
            if(call != null) sum += call.get();
        }
        return sum;
    }

}

function main(): int {

    let test = new Testee();
    let result: int = 0;

    //Check shadowing parameters
    let shadow: Supplier[] = [new SupplierImplForShadow(), new SupplierImplForShadow()];
    result = test.callShadow(new SupplierImpl(), new SupplierImpl());
    if(result != 6) return 1;


    return 0;

}
