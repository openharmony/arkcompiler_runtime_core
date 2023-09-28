/*---
desc: While statement (not executed)
tags: [negative, compile-only]
---*/

function main(): void {
    while (false) { // CTE: unreachable statement
        assert false;
    }
}
