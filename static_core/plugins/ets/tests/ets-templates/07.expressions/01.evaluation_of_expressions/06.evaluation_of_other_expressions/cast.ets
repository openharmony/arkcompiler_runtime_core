{% for t in cases %}
/*---
desc: >-
    The cast operator may specify a type that is incompatible with
    the class of the object referenced by the run-time value of the
    operand expression.
---*/

interface I {}
interface J {}
interface K {}
class A implements I {}
class B extends A implements J {}

function main(): int {
    {{t.init}};
    try {
      {{t.expr}};
      return {{t.fail}};
    } catch (e: ClassCastException) {
      return 1 - {{t.fail}};
    } catch (e) {
      return 1;
    }
}
{% endfor %}
