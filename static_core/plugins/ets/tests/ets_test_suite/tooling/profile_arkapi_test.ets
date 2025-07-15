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

function foo1(repetitions: int) {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
}

function foo2(repetitions: int) {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
}

function foo3(size: int) {
    let num: double = 0.0;
    let signNum: int = 1;
    for (let firstIndex: int = 0; firstIndex < size; firstIndex++) {
        num += signNum / (2.0 * firstIndex + 1);
        signNum *= -1;
    }
    foo1(size);
    for (let secondIndex: int = 0; secondIndex < size; secondIndex++) {
        num += signNum / (2.0 * secondIndex + 1);
        signNum *= -1;
    }
}

function foo() {
    let repetitions: int = 2;
    let delay: int = 1;
	let checkpoint = 0;
	let id2 = -1;
	id2 = setInterval(() => {
        if (checkpoint == 10000) {
            clearInterval(id2);
        }
		foo1(repetitions);
        foo2(repetitions);
        foo3(repetitions);
        checkpoint ++ ;
    }, delay)
}

function main(): int {
    foo();
    return 0;
}
