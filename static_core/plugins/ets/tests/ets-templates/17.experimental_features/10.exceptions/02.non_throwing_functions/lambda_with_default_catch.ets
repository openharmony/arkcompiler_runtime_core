/*---
desc: Declaring non-throwing lambda without default catch.
assert: To indicate that a function (in general sense, meaning a function, method, constructor or lambda) can throw an error, use the throws keyword at the end of the signature. A function marked with throws is called a throwing function. A function type can also be marked as ‘throws’.
---*/

class TestException extends Exception {}
type func = (x: int) => int;

function main(): void {
  let bar: func = (x: int): int => 
    { 
      try {
        throw new TestException();
      }
      catch (e: TestException) {
        return 5;
      }
      catch {
        return 10;
      }
    }

  let i: int = bar(5);
}