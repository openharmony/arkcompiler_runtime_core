/*---
desc: ArrayStoreException is thrown by an assignment to an array component of
  reference type when the component type of the array is incompatible with
  the value that is being assigned.
params: Long to String[]
---*/

function main(): int {
    let a = 1;
    let b = 1;
    let v: Object[];
    let s: String = "";

    try {
      s = s + (a = 2) + ((v = ["A", "B", "C"] as Object[])[b] = new Long()) + (b = 2);
      return 1;
    } catch (e: ArrayStoreException) {
      if (a != 2 || b != 1) return 1;
      return 0;
    } catch (e) {
      return 1;
    }
    return 1;
}
