/*---
desc: Function type return lambda.
---*/

function f(): () => int {
    return (): int => { return 5; }
}

function main(): void {
    let ff: () => int = f();
    assert ff() == 5;
}
