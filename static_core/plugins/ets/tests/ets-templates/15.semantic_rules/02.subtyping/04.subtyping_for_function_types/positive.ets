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

{%- for c in cases %}

/*---
desc: >-
    Function type F with parameters FP1, ... , FPm and return type
    FR is a subtype of function type S with parameters SP1 , ... , SPn
    and return type SR if all of the following conditions are met:
    • m <= n
    • for each i <= m
        - Parameter type of SPi is a subtype of parameter type of FPi (contravariance), and
        – FPi is a rest parameter if SPi is a rest parameter.
        – FPi is an optional parameter if SPi is an optional parameter.
    • FR can be any type if SR is type void. Otherwise, the resultant type FR is a subtype of SR (covariance).
tags: [compile-only]
---*/

class Base {}
class Derived extends Base {}

let bb: (p: Base) => Base = (p: Base) => { return new Base }
let bd: (p: Base) => Derived = (p: Base) => { return new Derived }
let db: (p: Derived) => Base = (p: Derived) => { return new Base }
let dd: (p: Derived) => Derived = (p: Derived) => { return new Derived }

{{c}}

{%- endfor %}
