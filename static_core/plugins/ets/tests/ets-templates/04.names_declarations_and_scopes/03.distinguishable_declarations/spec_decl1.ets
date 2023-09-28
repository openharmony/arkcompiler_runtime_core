/*---
desc: Declarations must be distinguishable in a declaration space.
---*/

// declarations distinguishable by name
const PI = 3.14
const pi = 3
function Pi(): double {
    return PI;
}
type IP = byte[]

function main(): int {
    let x: IP = new byte[4]
    Pi()
    if (Pi() == PI && pi == 3) return 0;
    return 1;
}
