/*---
desc: Declaring non-throwing lambda without default catch.
assert: To indicate that a function (in general sense, meaning a function, method, constructor or lambda) can throw an error, use the throws keyword at the end of the signature. A function marked with throws is called a throwing function. A function type can also be marked as ‘throws’.
---*/

class TestException extends Exception {}
type func = (x: int) => int throws;

function main(): void {
  let bar: func = (x: int): int throws =>
    {
      try {
        throw new Exception();
      }
      catch (e: TestException) {
        return 5;
      }
    }

  let i: int = bar(5);
}
