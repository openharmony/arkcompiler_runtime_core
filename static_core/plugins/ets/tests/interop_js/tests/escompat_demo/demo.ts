import { FooClass, FooFunction, BarFunction } from "./foo"

function test() {
    let arr = new Array<FooClass>();
    arr.push(new FooClass("zero"));
    arr.push(new FooClass("one"));
    arr.push(new FooClass("two"));

    console.log("test: " + FooFunction(arr));

    let arr2 = BarFunction();
    console.log("test: check instanceof Array: " + (arr2 instanceof Array));

    console.log("test: " + arr2.at(0).name);
    console.log("test: " + arr2.at(1).name);
    console.log("test: " + arr2.at(2).name);
}
test();
