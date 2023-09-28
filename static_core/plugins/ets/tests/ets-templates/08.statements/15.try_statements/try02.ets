/*---
desc: >-
  Try statements run the block of code to handle exceptions and errors.
  Try statements can contain several catch clauses to handle exceptions and
  errors.
---*/

function main(): int {
    try {
        if (1 != 2) throw new Exception();
        return 1;
    } catch (e) {
        return 0;
    }
}
