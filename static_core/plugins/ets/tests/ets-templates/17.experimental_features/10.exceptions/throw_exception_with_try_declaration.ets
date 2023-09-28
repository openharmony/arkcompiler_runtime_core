/*---
desc: Use throw with try statements.
assert: There are two ways to process exception. By handling the exception using a try statement, see “Try Statements”.
---*/

class TestException extends Exception {}

function main(): void {
  try {
    throw new TestException();
  }
  catch (e) {
    let y: int = 5;
  }
}