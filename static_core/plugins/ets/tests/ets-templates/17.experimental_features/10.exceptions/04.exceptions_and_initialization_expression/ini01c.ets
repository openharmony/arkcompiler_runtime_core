/*---
desc: >-
  A variable declaration (§4.7.1) or a constant declaration (§4.7.2) expression
  used to initialize a variable or constant must not have calls to functions
  that can throw or rethrow exceptions.
---*/

function main(): int throws {
    let i = 2;
    const v: int = (
        (i: int): int => { return --i; } // non throwing lambda
    )(i + 1) - 2;
    return v;
}
