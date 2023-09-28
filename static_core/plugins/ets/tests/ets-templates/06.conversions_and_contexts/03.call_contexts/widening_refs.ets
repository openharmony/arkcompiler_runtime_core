{% for c in cases %}
/*---
desc: Invocation contexts allow a widening reference conversion.
params: Widening reference conversion from '{{c.fromtype}}' to '{{c.totype}}'
---*/

interface I {}
interface J extends I {}
class A implements I {}
class B extends A implements J {}
class C extends B {}

class Clazz {
  public fld1: {{c.totype}};
  public fld2: {{c.totype}}?;

  public constructor(p: {{c.totype}}) {
    this.fld1 = p;
  }

  public foo(p: {{c.totype}}): void {
    this.fld2 = p;
  }
}

function main(): int {
  let v: {{c.fromtype}} = {{c.fromval | safe}};
  let obj: Clazz = new Clazz(v);
  obj.foo(v);
  if (obj.fld1 !== v || obj.fld2 !== v) {
    return 1;
  }
  return 0;
}
{% endfor %}
