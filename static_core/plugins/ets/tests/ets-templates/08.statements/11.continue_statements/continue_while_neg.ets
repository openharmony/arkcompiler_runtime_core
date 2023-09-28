/*---
desc: Continue statement (inside while statement)
tags: [negative, compile-only]
---*/

function main(): void {
    let i: int = 0;
    while (i <= 2) {
        i++;
        continue;
        i--; // compilation fails due to this unreachable statement
    }
    assert i == 3;
}
