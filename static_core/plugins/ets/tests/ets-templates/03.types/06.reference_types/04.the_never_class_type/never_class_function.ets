/*---
desc: If a function has the return type of never, it means that it never returns.
tags: [negative, compile-only]
---*/

let x: String;

function f(): never {}

function main(): void {
    f();
}
