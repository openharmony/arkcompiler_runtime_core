/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only]
---*/

class MyEx extends Exception {}

native function foo<T extends Exception>(a: T): T; // parameterized native

function main(): int {
    let ex: Exception = foo<MyEx>(new MyEx());
    return 0;
}
