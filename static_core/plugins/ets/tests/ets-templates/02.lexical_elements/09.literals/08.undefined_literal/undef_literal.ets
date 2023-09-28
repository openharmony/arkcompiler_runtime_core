/*---
desc: Undefined literal
---*/

let x = undefined; // x type is expected to be inferred to Object|undefined type
function main() {
    assert x === undefined
}
