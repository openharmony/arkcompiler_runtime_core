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
function sum(a0: byte, a1: byte): byte {
    return a0 + a1 as byte;
}
}

function moduleFunction(value1: byte, value2: byte): byte {
    return value1 + value2 as byte;
}

function moduleFunction(value1: byte, value2: byte, value3: byte): byte {
    return value1 + value2 + value3 as byte;
}

namespace A {
    function byteFunctionA(value1: byte, value2: byte): byte {
        return value1 + value2 as byte;
    }

    function byteFunctionA(value1: byte, value2: byte, value3: byte): byte {
        return value1 + value2 + value3 as byte;
    }

    function nestedFunction(value1: byte, value2: byte): byte {
        let lambdaFunction: (p1: byte, p2: byte) => byte =
            (param1: byte, param2: byte): byte => {
                return param1 + param2 as byte;
        };
        return lambdaFunction(value1, value2);
    }
    
    export function recursiveFunction(value: int): byte {
        if (value <= 1) {
            return 1;
        } else {
            return value + recursiveFunction(value - 1) as byte;
        }
    }

    function calculateSum(value1: byte, value2: byte, value3: char, value4: int): byte {
        let aChar: char = 'A';
        if (value3 == aChar) {
            return value2 - value1 as byte;
        }
        if (value4 == 1) {
            return value1;
        }
        return value2 + value1 as byte;
    }

    namespace B {
        function byteFunctionB(value1: byte, value2: byte): byte {
            return value1 + value2 as byte;
        }
    }
}
