/*---
desc: Declarations must be distinguishable in a declaration space.
tags: [compile-only]
---*/

// functions distinguishable by signatures

function foo() {}

function foo(x: int) {}

function foo(x: int[]) {}

function foo(x: string) {}
