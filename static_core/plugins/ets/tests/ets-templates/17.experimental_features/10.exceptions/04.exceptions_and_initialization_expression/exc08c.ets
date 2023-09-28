/*---
desc: >-
  A variable declaration (§4.7.1) or a constant declaration (§4.7.2) expression
  used to initialize a variable or constant must not have calls to functions
  that can throw or rethrow exceptions.
tags: [compile-only, negative]
---*/

let i = 2;

function main(): int {
    const v: int = (
        (i: int):int throws => {
            if (i != 2) {
                throw new Exception()
            }
            return --i;
        }
    )(i);   // CTE expected
    return v;
}
