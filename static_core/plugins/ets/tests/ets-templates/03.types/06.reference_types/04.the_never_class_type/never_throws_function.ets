/*---
desc: If a function has the return type of never, it means that it never returns.
tags: [compile-only]
---*/

function f(): never {
    throw new Exception("error");
}