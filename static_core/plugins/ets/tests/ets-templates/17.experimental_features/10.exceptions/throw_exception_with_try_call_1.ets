/*---
desc: Use throw with try statements without exception type in catch sttaement.
assert: There are two ways to process exception. By handling the exception using a try statement, see “Try Statements”.
---*/

class TestException extends Exception {}

function bar(): int {
  try {
    throw new TestException();
  }
  catch (e) {
    return 5;
  }
}

function main(): void {
  let y: int = 1;
  y = bar();
  assert y == 5;
}