/*---
desc: The null type is used to create nullable types (§3.6.9).
---*/

type NULL = null

function main(): void {
    let a: Object|NULL = null;
    assert a === null;
}
