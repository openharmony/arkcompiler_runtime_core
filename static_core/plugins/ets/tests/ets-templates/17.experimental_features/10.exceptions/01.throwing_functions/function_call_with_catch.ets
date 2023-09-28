/*---
desc: Throwing function call if a call of a throwing function is not enclosed in a try statement;.
assert: Exception propagation occurs in two cases. If a call of a throwing function is not enclosed in a try statement; If the enclosed try statement does not contain a clause that catches the raised exception.
---*/

class TestException extends Exception {}

function bar(): void throws {
  throw new TestException();
}

function main(): void {
  let i: int = 1;

  try {
    bar();
  }
  catch(e: TestException) {
    i = 5
  }
  catch (e) {
    i = 10;
  }

  assert i == 5;
}