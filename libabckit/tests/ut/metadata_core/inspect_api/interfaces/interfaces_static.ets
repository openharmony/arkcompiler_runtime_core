/**
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
@interface AI1 {}

interface InterfaceA {
    fieldA: string,
    readonly fieldB: number,
    bar(){}
}

interface InterfaceB extends InterfaceA {
    foo()
}

@AI1()
interface InterfaceC {
    fieldC: string
}

class ClassA implements InterfaceA {
    fieldA = "Hello";
    fieldB = 1.0;
}

class ClassB implements InterfaceB, InterfaceC {
    fieldA = "Hello";
    fieldB = 1.0;
    fieldC = "World";
    foo(){}
}