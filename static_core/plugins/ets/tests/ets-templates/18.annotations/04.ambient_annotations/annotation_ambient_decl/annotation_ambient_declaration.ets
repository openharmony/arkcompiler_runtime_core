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

/*---
desc: If an annotation is applied to an ambient declaration in the .d.ets file
      then the annotation is to be applied to the implementation declaration manually
tags: [not-a-test, compile-only]
---*/

enum MyEnum { EnumField1, EnumField2 }

{% for case in cases %}

export @interface SomeAnnotation{{ loop.index }} {
   field1: {{ case.type1 }} = {{ case.value1 }}
   field2: {{ case.type2 }} = {{ case.value2 }}
}

@SomeAnnotation{{ loop.index }}
export class SomeClass{{ loop.index }} { }

{% endfor %}