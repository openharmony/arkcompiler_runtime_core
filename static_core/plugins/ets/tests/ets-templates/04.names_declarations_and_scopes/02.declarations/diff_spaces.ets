/*---
desc: A name must be unique in its block space
---*/

function foo(): int {
    let xyz: int;
    return xyz;
}

class C {
    public xyz: int = 1;
}

class D {
    public xyz(): int {
        return 2;
    }
}

enum Enum { abc, def, ghk, xyz }

let xyz: int = 4;

interface I {
    static xyz(): int {
        return 5;
    }
}

function main(): int {
    if (0 == foo() &&
        1 == new C().xyz &&
        2 == new D().xyz() &&
        3 == Enum.xyz.getValue() &&
        4 == xyz &&
        5 == I.xyz()
    ) {
        let xyz = 0;
        return xyz;
    }
    return 1;
}
