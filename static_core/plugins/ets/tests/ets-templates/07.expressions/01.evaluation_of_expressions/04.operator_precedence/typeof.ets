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
desc: typeof operator with other operators
---*/

function main(): int {
    let t255: int = 255
    let num: number = 1.0
    let str: string = ""
    let bool: boolean = true
    let obj: Object = new Object()

    // postfix
    str = typeof num++
    if (str != "number" || num != 2.0) return 1;
    str = ""

    str = typeof num--
    if (str != "number" || num != 1.0) return 1;
    str = ""

    str = typeof typeof num++
    if (str != "string" || num != 2.0) return 1;
    str = ""

    str = typeof typeof num--
    if (str != "string" || num != 1.0) return 1;
    str = ""

    // prefix
    str = typeof ++num
    if (str != "number" || num != 2.0) return 1;
    str = ""

    str = typeof --num
    if (str != "number" || num != 1.0) return 1;
    str = ""

    str = typeof typeof ++num
    if (str != "string" || num != 2.0) return 1;
    str = ""

    str = typeof typeof --num
    if (str != "string" || num != 1.0) return 1;
    str = ""

    // unary plus
    str = typeof +num
    if (str != "number" || num != 1.0) return 1;
    str = ""

    str = typeof typeof +num
    if (str != "string" || num != 1.0) return 1;
    str = ""

    // unary minus
    str = typeof -num
    if (str != "number" || num != 1.0) return 1;
    str = ""

    str = typeof typeof -num
    if (str != "string" || num != 1.0) return 1;
    str = ""

    // not
    str = typeof !bool
    if (str != "boolean" || bool != true) return 1;
    str = ""

    str = typeof typeof !bool
    if (str != "string" || bool != true) return 1;
    str = ""

    // xor
    str = typeof ~t255
    if (str != "number") return 1;
    str = ""

    str = typeof typeof ~t255
    if (str != "string") return 1;
    str = ""

    // addition
    num = 42.0
    str = typeof num + num
    if (str != "number42") return 1;
    str = ""

    num = 42.0
    str = typeof typeof num + num
    if (str != "string42") return 1;
    str = ""

    num = 42.0
    str = typeof num + typeof num
    if (str != "numbernumber") return 1;
    str = ""

    num = 42.0
    str = typeof num + typeof typeof num
    if (str != "numberstring") return 1;
    str = ""

    // cast
    obj = typeof num as Object
    if (obj as string != "number") return 1;

    obj = typeof typeof str as String
    if (obj as string != "string") return 1;

    obj = typeof typeof str as String as Object
    if (obj as string != "string") return 1;

    // relational
    if (typeof num > typeof str) return 0;
    if (typeof num >= typeof str) return 0;
    if (typeof num == typeof str) return 0;
    if (typeof typeof num != typeof str) return 0;
    if (typeof typeof num > typeof str) return 0;
    if (typeof typeof num >= typeof str) return 0;
    if (typeof typeof num < typeof str) return 0;
    if (typeof typeof num <= typeof str) return 0;
    if (typeof typeof num != typeof typeof str) return 0;
    if (typeof typeof num > typeof typeof str) return 0;
    if (typeof typeof num >= typeof typeof str) return 0;
    if (typeof typeof num < typeof typeof str) return 0;
    if (typeof typeof num <= typeof typeof str) return 0;

    // ternary
    str = typeof num < typeof str ? typeof num : typeof str
    if (str != "number") return 1;
    str = ""

    str = typeof num > typeof str ? typeof num : typeof str
    if (str != "string") return 1;
    str = ""

    str = typeof str < typeof typeof str ? typeof typeof num : typeof num + 1
    if (str != "number1") return 1;
    str = ""

    // assignments
    let s = "1"
    str = s += typeof num
    if (str != "1number" || s != "1number") return 1;
    s = "1"
    str = ""

    str = s += typeof typeof num
    if (str != "1string" || s != "1string") return 1;
    s = "1"
    str = ""

    str = s += typeof typeof num + typeof num
    if (str != "1stringnumber" || s != "1stringnumber") return 1;

    return 0;
}
