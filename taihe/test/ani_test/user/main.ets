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
import * as ani_test from "@generated/ani_test";
import * as System from "std/core/System";

loadLibrary("ani_test");

function main() {
    // struct
    const option: ani_test.Data = {
        src: "path/to/source",
        dest: 123,
        files: ["file1.txt", "file2.txt", "file3.txt"]
    };
    ani_test.showData(option)

    console.log(ani_test.makeData())

    // union
    ani_test.showUnion(undefined)
    ani_test.showUnion(1 as int)
    ani_test.showUnion(2 as float)
    ani_test.showUnion("Hello from STS!")

    console.log(ani_test.makeUnion(0))
    console.log(ani_test.makeUnion(1))
    console.log(ani_test.makeUnion(2))
    console.log(ani_test.makeUnion(3))

    // optional
    ani_test.showOptionalInt(100 as int)
    ani_test.showOptionalInt(undefined)

    console.log(ani_test.makeOptionalInt(true))
    console.log(ani_test.makeOptionalInt(false))

    // array
    ani_test.showArrayInt([1, 2, 3])

    console.log(ani_test.makeArrayInt(10, 123))

    // interface
    let impl_foo_list = ani_test.makeFoo(["A", "B", "C"]);
    for (let i = 0; i < impl_foo_list.length; i++) {
        impl_foo_list[i].bar();
    }

    class UserFoo implements ani_test.Foo {
        name: string;
        constructor(name: string) {
            this.name = name;
        }
        bar(): void {
            console.log("UserFoo(" + this.name + ") is calling bar()")
        }
    }
    ani_test.callBar([new UserFoo("X"), new UserFoo("Y"), new UserFoo("Z")]);
}
