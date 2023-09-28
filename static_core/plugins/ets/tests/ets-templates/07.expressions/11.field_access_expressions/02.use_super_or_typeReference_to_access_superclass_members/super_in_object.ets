/*---
desc: Access to field by keyword super from constructor of Object class
assert: It is a compile-time error if the forms using the keyword super appear in the declaration of class Object, since Object has no superclass.
---*/

package std.core;

class Object {
  constructor() {
    super()
  }

  public toString(): String {
    return "";
  }

  public hashCode(): int {
    return runtime.GetHashCode(this);
  }

  public equals(to: Object): boolean {
    return runtime.equals(this, to);
  }
}
