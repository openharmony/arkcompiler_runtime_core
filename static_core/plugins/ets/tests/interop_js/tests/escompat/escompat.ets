/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

package escompat_test

// import * from "./compat_array"

function GCJSRuntimeCleanup(): void {
    try {
        // trigger FinalizationQueue cleanup
        let full_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }
}

class FooClass {
    constructor(name: String) {
        this.name = name;
    }
    public override toString(): String {
        return "{Foo named " + this.name + "}"
    }
    public name: String;
};

/********** Array **********/

function Array_TestJSSample(arr: Array): void {
    assert(arr as Object) instanceof Array;
    assert(arr.at(0) as FooClass).name == "zero";
    assert arr.at(1).toString() == "{Foo named one}";

    arr.push(new FooClass("two"));
    assert arr.at(2).toString() == "{Foo named two}";
    assert arr.toString() == "{Foo named zero},{Foo named one},{Foo named two}";
}

function Array_CreateEtsSample(): Array {
    let arr = new Array();
    arr.push(123);
    arr.push("foo");
    return arr;
}

/********** Boolean **********/

function Boolean_TestJSSample(v: Boolean): void {
    assert(v as Object) instanceof Boolean;
    assert v.toString() == "false";
}

function Boolean_CreateEtsSample(): Boolean {
    return true;
}

/********** Error **********/

function Error_TestJSSample(v: Error): void {
    assert(v as Object) instanceof Error;
    assert v.message == "foo";
}

function Error_CreateEtsSample(): Error {
    return new Error("bar");
}
