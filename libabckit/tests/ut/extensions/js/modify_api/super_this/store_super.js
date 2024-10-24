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
class Animal {
    age = 1;
    tmp = 0;
}

class Dog extends Animal {
    setAge() {
        super.tmp = 1;
        // super.age = 2;
    }
}

let doggy = new Dog();
doggy.setAge();
print(doggy.age); // 1 -> 2

function dummyFunc() {}
