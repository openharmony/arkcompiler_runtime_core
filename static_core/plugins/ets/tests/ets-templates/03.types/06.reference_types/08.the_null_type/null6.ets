/*---
desc: The null type is used to create nullable types (§3.6.9).
---*/

function foo<T extends Object|null>(a: T): T {
    return a;
}

function main(): void {
    let a: Object|null = foo<null>(null);
    assert a === null;
}
