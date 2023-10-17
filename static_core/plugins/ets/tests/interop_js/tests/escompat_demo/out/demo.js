"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var foo_1 = require("./foo");
function test() {
    var arr = new Array();
    arr.push(new foo_1.FooClass("zero"));
    arr.push(new foo_1.FooClass("one"));
    arr.push(new foo_1.FooClass("two"));
    console.log("test: " + (0, foo_1.FooFunction)(arr));
    var arr2 = (0, foo_1.BarFunction)();
    console.log("test: check instanceof Array: " + (arr2 instanceof Array));
    console.log("test: " + arr2.at(0).name);
    console.log("test: " + arr2.at(1).name);
    console.log("test: " + arr2.at(2).name);
}
test();
