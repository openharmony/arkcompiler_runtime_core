{% for t in cases %}
/*---
desc: Assignment to an array element of reference type.
---*/

interface I {}
interface J {}
interface K {}
class A implements I {}
class B extends A implements J {}

function main(): int {
    let v: Object[] = {{t.init}};
    let w: Object = {{t.expr}};
    try {
      v[0] = w;
      return {{t.fail}};
    } catch (e: ArrayStoreException) {
      return 1 - {{t.fail}};
    } catch (e) {
      return 1;
    }
}
{% endfor %}
