/*---
desc: lambda evaluation
---*/

class TestException extends Exception {}

type func = () => void;

function main(): void {
  let bar: func = (): void throws => { throw new TestException(); }

  try {
    bar();
  } catch (e: TestException) {
    assert true;
  } catch {
    assert false;
  }
}