/*---
desc: Null literal.
---*/

let x = null; // x type is expected to be inferred to Object|null type
function main() {
    assert x === null
}
