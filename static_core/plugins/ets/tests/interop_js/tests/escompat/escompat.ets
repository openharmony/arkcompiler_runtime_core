/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assertTrue( false, "Unexpected exception during GC");
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

function Array_TestJSSample(arr: Array<FooClass>): void {
    assertTrue( (arr as Object) instanceof Array);
    assertEQ( (arr.at(0)! as FooClass).name, "zero");
    assertEQ( arr.at(1)!.toString(), "{Foo named one}");

    let idx: number = 0;
    assertEQ( (arr.at(idx)! as FooClass).name, "zero");

    arr.push(new FooClass("two"));
    assertEQ( arr.at(2)!.toString(), "{Foo named two}");
    assertEQ( arr.toString(), "{Foo named zero},{Foo named one},{Foo named two}");
}

function Array_TestJSLength(arr: Array<FooClass>): void {
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // let len: int = arr.length();
    // assertTrue len == 2;
    // arr.push(new FooClass("two"));
    // assertTrue arr.length() == len + 1;
}

function Array_TestJSPop(arr: Array<FooClass>): void {
    arr.push(new FooClass("two"));
    let popped: FooClass = arr.pop() as FooClass;
    assertEQ( popped.toString(), "{Foo named two}");
}

function Array_TestJSSort(arr: Array<FooClass>): void {
    let comparator: (a: FooClass, b: FooClass) => number = (a: FooClass, b: FooClass): number => {
        return Int.toDouble(a.toString().compareTo(b.toString()));
    }
    arr.sort(comparator);
    assertEQ( arr.at(1)!.toString(), "{Foo named zero}");
}

function Array_TestJSToSorted(arr: Array<FooClass>): void {
    let comparator: (a: FooClass, b: FooClass) => number = (a: FooClass, b: FooClass): number => {
        return Int.toDouble(a.toString().compareTo(b.toString()));
    }
    let toSorted = arr.toSorted(comparator);
    assertEQ( toSorted.at(1)!.toString(), "{Foo named zero}");
}

function Array_TestJSFill(arr: Array<FooClass>): void {
    arr.fill(arr.at(0)! as FooClass, undefined, undefined);
    assertEQ( arr.at(1)!.toString(), "{Foo named zero}");
}

function Array_TestJSJoin(arr: Array<FooClass>): void {
    let joined: String = arr.join(",");
    assertEQ( arr.toString(), "{Foo named zero},{Foo named one}");
}

function Array_TestJSSome(arr: Array<FooClass>): void {
    let fnTrue: (v: FooClass) => boolean = (v: FooClass): boolean => { return true; }
    let fn1True: (v: FooClass, k: number) => boolean = (v: FooClass, k: number): boolean => { return true; }
    assertEQ( arr.some(fnTrue), true);
    assertEQ( arr.some(fn1True), true);
    let fnFalse: (v: FooClass) => boolean = (v: FooClass): boolean => { return false; }
    let fn1False: (v: FooClass, k: number) => boolean = (v: FooClass, k: number): boolean => { return k < 0; }
    assertEQ( !arr.some(fnFalse), true);
    assertEQ( !arr.some(fn1False), true);
}

function Array_TestJSEvery(arr: Array<FooClass>): void {
    let fnTrue: (v: FooClass) => boolean = (v: FooClass): boolean => { return true; }
    let fn1True: (v: FooClass, k: number) => boolean = (v: FooClass, k: number): boolean => { return true; }
    assertEQ( arr.every(fnTrue), true);
    assertEQ( arr.every(fn1True), true);
    let fnFalse: (v: FooClass) => boolean = (v: FooClass): boolean => { return false; }
    let fn1False: (v: FooClass, k: number) => boolean = (v: FooClass, k: number): boolean => { return k < 0; }
    assertEQ( !arr.every(fnFalse), true);
    assertEQ( !arr.every(fn1False), true);
}

function Array_TestJSFilter(arr: Array<FooClass>): void {
    let fnTrue: (v: FooClass) => boolean = (v: FooClass): boolean => { return true; }
    let fn1True: (v: FooClass, k: number) => boolean = (v: FooClass, k: number): boolean => { return true; }
    let filter = arr.filter(fnTrue);
    let filter1 = arr.filter(fn1True);
    assertEQ( arr.at(0), filter.at(0));
    assertEQ( arr.at(0), filter1.at(0));
    let fnFalse: (v: FooClass) => boolean = (v: FooClass): boolean => { return false; }
    let fn1False: (v: FooClass, k: number) => boolean = (v: FooClass, k: number): boolean => { return k < 0; }
    let filterFalse = arr.filter(fnFalse);
    let filter1False = arr.filter(fn1False);
    assertNE( arr.at(0), filterFalse.at(0));
    assertNE( arr.at(0), filter1False.at(0));
}

function Array_TestJSFilterArray(arr: Array<FooClass>): void {
    let fnTrue: (v: FooClass) => boolean = (v: FooClass): boolean => { return true; }
    let filter: Array<FooClass> = arr.filter(fnTrue);
    assertEQ( arr.at(0), filter.at(0));
    let fnFalse: (v: FooClass) => boolean = (v: FooClass): boolean => { return false; }
    let filterFalse: Array<FooClass> = arr.filter(fnFalse);
    assertNE( arr.at(0), filterFalse.at(0));
}

function Array_TestJSShift(arr: Array<FooClass>): void {
    let shifted: FooClass = arr.shift() as FooClass;
    assertEQ( shifted.toString(), "{Foo named zero}");
    assertEQ( arr.at(0)!.toString(), "{Foo named one}");
}

function Array_TestJSWith(arr: Array<FooClass>): void {
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // let arr1: Array<FooClass> = arr.with(1, new FooClass("three"));
    // assertTrue arr1.at(1).toString() == "{Foo named three}";
}

function Array_TestJSSlice(arr: Array<FooClass>): void {
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // assertTrue (arr.slice(1) as Array<FooClass>).toString() == "{Foo named two}";
    // assertTrue (arr.slice(1, 2) as Array<FooClass>).toString() == "{Foo named two}";
}

function Array_TestJSSplice(arr: Array<FooClass>): void {
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // arr.push(new FooClass("two"));
    // arr.splice(2, 1);
    // assertTrue arr.toString() == "{Foo named zero},{Foo named two}";
    // arr.splice(1);
    // assertTrue arr.toString() == "{Foo named zero}";
}

function Array_TestJSToSpliced(arr: Array<FooClass>): void {
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // arr.push(new FooClass("two"));
    // let spliced = arr.toSpliced(1, 1);
    // assertTrue spliced.toString() == "{Foo named zero},{Foo named two}";
    // let spliced1 = arr.toSpliced(1);
    // assertTrue spliced1.toString() == "{Foo named zero}";
}

function Array_TestJSCopyWithin(arr: Array<FooClass>): void {
    arr.push(new FooClass("two"));
    assertEQ( arr.toString(), "{Foo named zero},{Foo named one},{Foo named two}");
    arr.copyWithin(2, 0);
    assertEQ( arr.toString(), "{Foo named zero},{Foo named one},{Foo named zero}");

    // NOTE (oignatenko) uncomment below after interop will be supported for this method signature
    // arr.copyWithin(1);
    // assertTrue arr.toString() == "{Foo named zero},{Foo named zero},{Foo named one}";
}

function Array_TestJSLastIndexOf(arr: Array<FooClass>): void {
    arr.push(arr.at(1) as FooClass);

    assertEQ( arr.lastIndexOf(arr.at(1) as FooClass, 2.0), 2);
    assertEQ( arr.lastIndexOf(new FooClass("not in array"), 2.0), -1);

    // NOTE (oignatenko) uncomment below after interop will be supported for this method signature
    // assertTrue arr.lastIndexOf(arr.at(1) as FooClass) == 2;
    // assertTrue arr.lastIndexOf(new FooClass("not in array")) == -1;
}

function Array_TestJSToReversed(arr: Array<FooClass>): void {
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // let reversed_copy: Array<FooClass> = arr.toReversed() as Array<FooClass>;
    // assertTrue reversed_copy.toString() == "{Foo named one},{Foo named zero}";
}

function Array_TestJSMap(arr: Array<FooClass>): void {
    let fn_map: (v: FooClass, k: int) => String = (v: FooClass, k: int): String => { return "String"; }
    let fn_map1: (v: FooClass) => String = (v: FooClass): String => { return "String1"; }

    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // let mapped = arr.map(fn_map);
    // assertTrue mapped.toString() == "String,String";
    //
    // let mapped1 = arr.map(fn_map1);
    // assertTrue mapped1.toString() == "String1,String1";
}

function Array_TestJSFlatMap(arr: Array<FooClass>): void {
    let fn_map: (v: FooClass, k: int) => String = (v: FooClass, k: int): String => { return "String"; }
    let fn_map1: (v: FooClass) => String = (v: FooClass): String => { return "String1"; }

    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // let mapped = arr.flatMap(fn_map);
    // assertTrue mapped.toString() == "String,String";
    //
    // let mapped1 = arr.flatMap(fn_map1);
    // assertTrue mapped1.toString() == "String1,String1";
}

function Array_TestJSReduce(arr: Array<FooClass>): void {
    let fn_reduce: (a: FooClass, b: FooClass) => FooClass = (a: FooClass, b: FooClass): FooClass => { return a; }
    let reduced: FooClass = arr.reduce(fn_reduce) as FooClass;
    assertEQ( reduced.toString(), "{Foo named zero}");

    let fn_reduce2: (a: FooClass, b: FooClass) => FooClass = (a: FooClass, b: FooClass): FooClass => { return b; }
    let reduced2: FooClass = arr.reduce(fn_reduce2, new FooClass("initVal")) as FooClass;
    assertEQ( reduced2.toString(), "{Foo named initVal}");
}

function Array_TestJSReduceRight(arr: Array<FooClass>): void {
    let fn_reduce: (a: FooClass, b: FooClass) => FooClass = (a: FooClass, b: FooClass): FooClass => { return a; }
    let reduced: FooClass = arr.reduceRight(fn_reduce) as FooClass;
    assertEQ( reduced.toString(), "{Foo named zero}");

    let fn_reduce2: (a: FooClass, b: FooClass) => FooClass = (a: FooClass, b: FooClass): FooClass => { return b; }
    let reduced2: FooClass = arr.reduceRight(fn_reduce2, new FooClass("initVal")) as FooClass;
    assertEQ( reduced2.toString(), "{Foo named initVal}");
}

function Array_TestJSFindLast(arr: Array<FooClass>): void {
    let fnTrue: (v: FooClass) => boolean = (v: FooClass): boolean => { return true; }
    let fnFalse: (v: FooClass) => boolean = (v: FooClass): boolean => { return false; }

    let last: FooClass = arr.findLast(fnTrue) as FooClass;
    assertEQ( last.toString(), "{Foo named one}" );

    let last_null = arr.findLast(fnFalse);
    assertTrue( last_null === undefined );
}

function Array_TestJSFindIndex(arr: Array<FooClass>): void {
    let fnTrue: (v: FooClass) => boolean = (v: FooClass): boolean => { return true; }
    let fnFalse: (v: FooClass) => boolean = (v: FooClass): boolean => { return false; }

    let found: int = Double.toInt(arr.findIndex(fnTrue));
    assertEQ( found, 0) ;

    let not_found: int = Double.toInt(arr.findIndex(fnFalse));
    assertEQ( not_found, -1 );
}

function Array_TestJSFind(arr: Array<FooClass>): void {
    let fnTrue: (v: FooClass) => boolean = (v: FooClass): boolean => { return true; }
    let fnFalse: (v: FooClass) => boolean = (v: FooClass): boolean => { return false; }

    let found: FooClass = arr.find(fnTrue) as FooClass;
    assertEQ( found.toString(), "{Foo named zero}" );

    let found_null = arr.find(fnFalse);
    assertTrue( found_null === undefined );
}

function Array_TestJSIsArray(arr: Array<FooClass>): void {
    assertTrue( Array.isArray(arr));
}

function Array_TestJSFromAsync(arr: Array<FooClass>): void {
    let fn_map: (v: FooClass, k: int) => String = (v: FooClass, k: int): String => { return "String"; }
    let fn_map1: (v: FooClass) => String = (v: FooClass): String => { return "String1"; }

    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // Array.fromAsync(arr).then((array) => { assertTrue array.at(0).toString() == "{Foo named zero}" });
    // Array.fromAsync(arr, fn_map).then((array) => { assertTrue array.at(0).toString() == "String" });
    // Array.fromAsync(arr, fn_map1).then((array) => { assertTrue array.at(0).toString() == "String1" });
}

function Array_CreateEtsSample(): Array<Object> {
    let arr = new Array<Object>();
    arr.push(123, "foo");
    return arr;
}

/********** Boolean **********/

function Boolean_TestJSSample(v: Boolean): void {
    assertTrue( (v as Object) instanceof Boolean );
    assertEQ( v.toString(), "false" );
}

function Boolean_CreateEtsSample(): Boolean {
    return true;
}

/********** Error **********/

function Error_TestJSSample(v: Error): void {
    assertTrue((v as Object) instanceof Error);
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // assertTrue v.message.toString() == "foo";
    // assertTrue v.cause == v;
    // assertTrue v.stack.toString().includes("Error");
}

function Error_TestJSWithCause(v: Error): void {
    assertTrue((v as Object) instanceof Error);
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // assertTrue v.message.toString() == "message";
    // assertTrue v.cause.toString() == "cause";
    // assertTrue v.stack.toString().includes("Error");
}

function Error_TestJSToString(v: Error): void {
    // NOTE (oignatenko) uncomment this after interop will be supported in this direction
    // assertTrue v.toString().includes(v.message.toString()));
}

function Error_CreateEtsSample(): Error {
    return new Error("bar");
}

function Error_CreateEtsSampleWithCause(): Error {
    return new Error("message", new ErrorOptions("cause"));
}

function ArrayBuffer_TestJSLength(dynArr: Any): void {
    let arr = InteropTransferHelper.transferArrayBufferToStatic(dynArr) as ArrayBuffer;
    assertTrue( (arr as Object) instanceof ArrayBuffer);
    assertEQ( arr.byteLength, 32 );
}

function ArrayBuffer_TestJSIsView(dynArr: Any): void {
    let arr = InteropTransferHelper.transferArrayBufferToStatic(dynArr) as ArrayBuffer;
    assertTrue( !ArrayBuffer.isView(arr) );
    assertTrue( ArrayBuffer.isView(new Int8Array(arr)) );
}

function ArrayBuffer_TestJSSlice(dynArr: Any): void {
    let arr = InteropTransferHelper.transferArrayBufferToStatic(dynArr) as ArrayBuffer;
    let sliced = arr.slice(8 as int, 24 as int); // workaround for #26000
    assertEQ( sliced.byteLength, 16 );
}

function ArrayBuffer_TestJSResize(dynArr: Any): void {
    let arr = InteropTransferHelper.transferArrayBufferToStatic(dynArr) as ArrayBuffer;
    // NOTE(dslynko, #23919): finalize semantics of resizable ArrayBuffers
    assertEQ(arr.resizable, false);
}

function ArrayBuffer_CreateEtsSample(): Any {
    return InteropTransferHelper.transferArrayBufferToDynamic(new ArrayBuffer(32, 32));
}

function JSON_stringify_jsv(x: JSValue) {
    return JSON.stringify(x)
}

function JSON_stringify_obj(x: JSValue) {
    let y = x as Object
    return JSON.stringify(y)
}

class TestClass {
    constructor(x: JSValue) {
        this.x = x
    }
    x: JSValue
}

function JSON_stringify_field(x: JSValue) {
    return JSON.stringify(new TestClass(x))
}
