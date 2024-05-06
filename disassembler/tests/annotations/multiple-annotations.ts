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

@interface Anno1 {
    a: number = 1;
}

@interface Anno2 {
    b: string = "string"
}

@interface Anno3 {
    c: boolean[] = [true, false];
}

@#Anno1({a: 42})
@#Anno2({b: "abc"})
@#Anno3({c: [false, true]})
class A {
    @#Anno1({a: 42})
    @#Anno2({b: "abc"})
    @#Anno3({c: [false, true]})
    foo() {}
}