/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

class Arr<T> {
	public constructor(a: T[]) {}

	public mp<U>(a: int): U[] {
        return new U[a]
	}
	public mg<U>(): Arr<U> {
		let c = this.mp<U>(10);
		return new Arr<U>(c)
	}
}

function main(): void {
    let a : Arr<Int> = new Arr<Int>(new Int[1])
    assert (a != null) : "Generics allocating error #1!"
    let b : Arr<Int> = a.mg<Int>()
    assert (b != null) : "Generics allocating error #2!"
}
