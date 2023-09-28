/*---
desc: Combined comments
params: multi-line over one-liner
tags: [compile-only, negative]
---*/

function main(): int {
    // CTE - extra opening multi-line bracket
    let i: int =
      /*
          // /* comment
      */
      5;
    return i - 5;
}
