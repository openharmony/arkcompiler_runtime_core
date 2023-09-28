/*---
desc: Function with null type parameter
tags: [compile-only]
---*/

function f(a: Int|null) { }

function main(): void {
    f(null);
}
