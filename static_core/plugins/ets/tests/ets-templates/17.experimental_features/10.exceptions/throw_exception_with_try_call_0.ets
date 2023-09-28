/*---
desc: Use throw with try statements with exception type in catch statement.
assert: There are two ways to process exception. By handling the exception using a try statement, see “Try Statements”.
---*/

class TestException extends Exception {}

function bar(): int {
  try {
    throw new TestException();
  }
  catch(e: TestException) {
    return 5;
  }
  catch (e) {
    assert false : "Unexpected catch clause";
  }

  return 1;
}

function main(): void {
  let y: int = 1;
  y = bar();
  assert y == 5;
}