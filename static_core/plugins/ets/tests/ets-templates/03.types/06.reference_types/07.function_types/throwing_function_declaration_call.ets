/*---
desc: It is a compile-time error if a value which is a throwing function is assigned to a variable of none-throwing function type.
tags: [negative, compile-only]
---*/

function f(): int throws {
    return 5;
}

function main(): void {
    let x = f();
}
