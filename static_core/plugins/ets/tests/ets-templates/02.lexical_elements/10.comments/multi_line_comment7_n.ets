/*---
desc: Multi-line comments
tags: [compile-only, negative]
---*/

function main(): int {
    let i: int =
    /*
      /*
        /*
            nested comment, closing is missing
      */
    */
    5;

    return i;
}
