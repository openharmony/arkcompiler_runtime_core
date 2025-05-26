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
function sum(a0: float, a1: float): float {
    return a0 + a1 as float;
}
}

function moduleFunction(value1: float, value2: float): float {
    return value1 + value2 as float;
}

function moduleFunction(value1: float, value2: float, value3: float): float {
    return value1 + value2 + value3 as float;
}

namespace A {
    function floatFunctionA(value1: float, value2: float): float {
        return value1 + value2 as float;
    }

    function floatFunctionA(value1: float, value2: float, value3: float): float {
        return value1 + value2 + value3 as float;
    }

    function nestedFunction(value1: float, value2: float): float {
        let lambdaFunction: (p1: float, p2: float) => float =
            (param1: float, param2: float): float => {
                return param1 + param2 as float;
        };
        return lambdaFunction(value1, value2);
    }
    
    export function recursiveFunction(value: int): float {
        if (value <= 1) {
            return 1;
        } else {
            return value + recursiveFunction(value - 1) as float;
        }
    }

    function calculateSum(value1: float, value2: float, value3: char, value4: int): float {
        let aChar: char = 'A';
        if (value3 == aChar) {
            return value2 - value1 as float;
        }
        if (value4 == 1) {
            return value1;
        }
        return value2 + value1 as float;
    }

    namespace B {
        function floatFunctionB(value1: float, value2: float): float {
            return value1 + value2 as float;
        }
    }
}
