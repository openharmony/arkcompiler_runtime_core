/*---
desc: ClassCastException is thrown by a cast expression
  if at run time a cast cannot be performed.
---*/

function main(): int {
  let a = 1;
  let b = 1;
  let o = new Object();

  try {
    let v: Long = (a = 2) + new Long((o as Long).unboxed()) + (b = 2);
    return 1;
  } catch (e: ClassCastException) {
    if (a != 2 || b != 1) return 1;
    return 0;
  } catch (e) {
    return 1;
  }
}
