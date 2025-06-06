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

{%- for c1 in cases %}
    {%- for c2 in cases %}
        {%- if c1.to != c2.to and c1.to != 'Object' %}
            {%- for from in c2.from  %}
/*---
desc: >-
    This conversion causes a runtime error (ClassCastError)
    if the runtime type of an expression is not the target type.
---*/

function cast<T>(p: Object|null|undefined) {
    return p as T // attempt to convert of any type to type parameter
}

{{from.decl}}

function main() {
    try {
        cast<{{c1.to}}>({{from.arg}})
    }
    catch (error: Error) {
        if (error instanceof ClassCastError) // OK
            return

        throw error // Unknown error
    }
    throw new AssertionError("Expected a ClassCastError")
}

// Auxiliary stuff

class A {}
class B {}
class C {}

interface I {}
interface J {}

class CI implements I {}
class CJ implements J {}

class D extends C {}

enum EN { a, b, c }
enum ES { a = "a", b = "b", c = "c" }

            {%- endfor %}
        {%- endif %}
    {%- endfor %}
{%- endfor %}
