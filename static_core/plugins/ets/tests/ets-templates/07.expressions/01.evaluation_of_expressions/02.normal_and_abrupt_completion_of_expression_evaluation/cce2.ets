/*---
desc: ClassCastException is thrown by a cast expression
  if at run time a cast cannot be performed.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let t: Long|null = null;

  try {
    let v: long = (a = 2) + (t === null ? 0 : t.unboxed()) + (b = 2);
    if (a != 2 || b != 2) return 1;
    return 0;
  } catch (e: ClassCastException) {
    return 1;
  } catch (e) {
    return 1;
  }
}
