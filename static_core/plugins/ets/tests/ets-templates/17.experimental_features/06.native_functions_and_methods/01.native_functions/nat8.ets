/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only]
---*/

native function bar(): Object throws;

native function foo(func: () => Object throws): Object rethrows; // native with rethrows

function main(): int throws {
    let v: Object = foo(bar);
    return 0;
}
