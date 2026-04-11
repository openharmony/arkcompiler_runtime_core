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
desc: |-
    Value array type is the built-in type written as ValueArray<T> and characterized by the fact:
    Any instance of array type contains elements of type T. T must be a value type
    Test assertions:
    - Any instance of array type contains elements of type T. T must be a value type
    - The number of elements is known as array length, and can be accessed by using the property length.
    - Array element is accessed by its index. Index is an integer number starting from 0 to array length minus 1.
    - Type of each array element is equal to the element(s) type specified in the array declaration.
    - Value array can be created by using Array Literal or constructors defined for type ValueArray<T>
    - Value assigning
---*/

//type creation check
{% for c in cases %}
type A{{loop.index}} = ValueArray<{{c.type}}>
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Value_array_init");

    {% for c in cases %}

        {%- if "number" == c.type or "Number" == c.type -%}
            {% set tp = "Int.toDouble" -%}
        {%- elif "bigint" == c.type or "BigInt" == c.type -%}
            {% set tp = "new BigInt" %}
        {%- else -%}
            {% set tp = "Int.to" + c.type | capitalize -%}
        {%- endif %}

        testSuite.addTest("type_{{c.type}}", () => {
            //array init
            let arr: ValueArray<{{c.type}}> = {{c.arr}}
            {%- if "double" == c.type or "Double" == c.type %}
            arktest.assertEQ(typeof arr[0], 'number')
            arktest.assertEQ(typeof arr[arr.length-1], 'number')
            {%- else %}
            arktest.assertEQ(typeof arr[0], '{{c.type | lower}}')
            arktest.assertEQ(typeof arr[arr.length-1], '{{c.type | lower}}')
            {%- endif %}
            arr[0] = arr[arr.length-1]
            arktest.assertEQ(arr[0], arr[arr.length-1])
            arktest.assertEQ(typeof arr.length , 'int')
            arktest.assertTrue(arr.length > 0)
            
            //constructor(len: int, elem: T)
            {%- if "bigint" == c.type or "BigInt" == c.type %}
            let arr2: ValueArray<{{c.type}}> = new ValueArray<{{c.type}}>(3, new BigInt(333n))
            {%- elif "boolean" == c.type or "Boolean" == c.type %}
            let arr2: ValueArray<{{c.type}}> = new ValueArray<{{c.type}}>(2, true)
            {%- elif "char" == c.type or "Char" == c.type %}
            let arr2: ValueArray<{{c.type}}> = new ValueArray<{{c.type}}>(3, c'\u0000')
            {%- else %}
            let arr2: ValueArray<{{c.type}}> = new ValueArray<{{c.type}}>(3, 0)
            {%- endif %}
            {%- if "double" == c.type or "Double" == c.type %}
            arktest.assertEQ(typeof arr2[0], 'number')
            arktest.assertEQ(typeof arr2[arr2.length-1], 'number')
            {%- else %}
            arktest.assertEQ(typeof arr2[0], '{{c.type | lower}}')
            arktest.assertEQ(typeof arr2[arr2.length-1], '{{c.type | lower}}')
            {%- endif %}
            arr2[0] = arr2[arr2.length-1]
            arktest.assertEQ(arr2[0], arr2[arr2.length-1])
            arktest.assertEQ(typeof arr2.length , 'int')
            arktest.assertTrue(arr2.length > 0)

        });
    {%- endfor %}

    return testSuite.run();
}
