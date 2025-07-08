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

{% for case in cases %}

/*---
desc: Annotations can be applied to the following
        Top-level declarations;
        Class members or interface members;
        Type usage;
        Parameters;
        A lambda expression;
        Local declarations.
tags: [compile-only]
---*/

{{ case.decl }}

@Retention("SOURCE")
@interface SomeAnnotation {
   field1: {{ case.type1 }} = {{ case.value1 }}
   field2: {{ case.type2 }} = {{ case.value2 }}
}

@Retention("SOURCE")
@interface Optional {
}

@SomeAnnotation({ field2: {{ case.value2 }} })
const c: string = "some_string"

@SomeAnnotation({field1: {{ case.value1 }} })
let l: number = 1

type SomeType<T> = FixedArray<T>

@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
const t: SomeType<number> = [100, 200]

let x = @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} }) () => { return 1 }

class Receiver {}
let lrc = @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} }) (this: Receiver) => { return 1 }

@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
function foo(@SomeAnnotation a: int, @Optional b?: string) {
    return a;
}

function goo(@Optional b: string = "Optional value") {
}

@SomeAnnotation
class MyClass {

    @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
    constructor (p: number) {
    }

    @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
    field: int = 1

    @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
    bar(@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} }) a: int, @Optional b?: string) {
        return a
    }
}

@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
interface MyInterface {}

{% endfor %}
