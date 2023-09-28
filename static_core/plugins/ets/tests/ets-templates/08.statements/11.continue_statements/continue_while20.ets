/*---
desc: Continue statement (inside while statement)
---*/

function main(): void {
    let i: int = 0;
    let j: int = 0;
    while (i < 20) {
        i++;
        if (i > 10) continue;
        j++;
    }
    assert i == 20 && j == 10;
}
