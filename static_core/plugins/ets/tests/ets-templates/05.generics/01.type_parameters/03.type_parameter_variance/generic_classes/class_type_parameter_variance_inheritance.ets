/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: |-
    in and out is a variance modifiers that specifies the variance of the type parameter. 
    Test of usage modifiers in and out, as well as type without parameter. 
    5.1.3 Type Parameter Variance
name: 05.generics/01.type_parameters/03.type_parameter_variance/wrong_type_parameter_variance_inheritance
tags: []
---*/

class B {}
class D extends B {}

class InG<in T> {}
class OutG<out T> {}

let ib: InG<D> = new InG<B>
let ob: OutG<B> = new OutG<D>
