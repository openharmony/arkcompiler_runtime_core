{% for t in cases %}
/*---
desc: >-
    A catch clause can throw an exception only if the class of the thrown
    exception object is an instanceof the type of the formal parameter of
    the respective catch clause.
---*/

interface I {}
interface J {}
interface K {}
class A extends Exception implements I {}
class B extends A implements J {}
class C extends B implements K {}

function main(): int {
    let v: Exception = {{t.init}};
    try {
      throw v;
    } catch (e: {{t.exc}}) {
      return {{t.fail}};
    } catch (e) {
      return 1 - {{t.fail}};
    }
}
{% endfor %}
