/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
@interface __$$ETS_ANNOTATION$$__Anno {
    a: number = 3
    b: number[] = [13, 9]
    d: boolean = false
    str: string = 'Hello'
}

@__$$ETS_ANNOTATION$$__Anno({
    a: 20,
    b: [13, 10],
    d: true,
    str: 'ABC'
})
class A {
    @__$$ETS_ANNOTATION$$__Anno({
        a: 10,
        b: [1, 2, 3],
        d: true,
        str: 'DEF'
    })
    foo() {}
}
