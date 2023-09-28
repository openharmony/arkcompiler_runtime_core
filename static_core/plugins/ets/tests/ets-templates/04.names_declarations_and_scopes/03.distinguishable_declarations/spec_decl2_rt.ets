/*---
desc: Declarations must be distinguishable in a declaration space.
---*/

// functions distinguishable by signatures

function foo(): int {
    return 1;
}

function foo(x: int): int {
    return 2;
}

function foo(x: int[]): int {
    return 3;
}

function foo(x: string): int {
    return 4;
}

function main(): int {
    if (foo() != 1) return 1;

    if (foo(1) != 2) return 1;

    if (foo(new int[0]) != 3) return 1;

    if (foo("") != 4) return 1;

    return 0;
}
