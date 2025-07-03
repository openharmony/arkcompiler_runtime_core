/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

namespace ops {
function sub(c0: char, c1: char): char {
    let c: Char = new Char(c'0');
    if (c0 > c1) {
        c = c0 - c1 as char;
    } else {
        c = c1 - c0 as char;
    }
    return c as char;
}
}

function moduleFunction(value1: char, value2: char): char {
    return value1 > value2 ? value1 : value2;
}

function moduleFunction(value1: char, value2: char, value3: char): char {
    return value1 < value2 ? value1 : value3;
}

namespace A {
    function charFunctionA(value1: char, value2: char): char {
        return value1 > value2 ? value1 : value2;
    }

    function charFunctionA(value1: char, value2: char, value3: char): char {
        return value1 > value2 ? value1 : value3;
    }

    function nestedFunction(value1: char, value2: char): char {
        let lambdaFunction: (p1: char, p2: char) => char =
            (param1: char, param2: char): char => {
                return param1 > param2 ? param1 : param2;
        };
        return lambdaFunction(value1, value2);
    }

    export function recursiveFunction(value: int): char {
        if (value <= 0) {
            return c'A';
        }
        if (value === 1) {
            return c'Z';
        }
        return recursiveFunction(value - 1);
    }

    function calculateSum(value1: char, value2: char, value3: double, value4: int): char {
        if (value3 === 1.0) {
            return value2;
        }
        if (value4 === 1) {
            return value1;
        }
        return value2;
    }

    namespace B {
        function charFunctionB(value1: char, value2: char): char {
            return value1 > value2 ? value1 : value2;
        }
    }
}
