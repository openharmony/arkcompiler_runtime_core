{% for c in cases %}
/*---
desc: A value of the null type (the null reference is the only such value) may be assigned to any reference type.
---*/

type Intfoo = (i: int) => int;

class A {
  private val: int;

  public constructor(
    {%- for t in c['types'] %}
    p{{loop.index}}: {{t|safe}}{{ ", " if not loop.last }}
    {%- endfor %}
  ) {
    if (
      {%- for t in c['types'] %}
      p{{loop.index}} === null {{ ("&&" if not loop.last) | safe}}
      {%- endfor %}
    ) {
      this.val = 42;
    }
  }

  public foo(
    {%- for t in c['types'] %}
    p{{loop.index}}: {{t|safe}}{{ ", " if not loop.last }}
    {%- endfor %}
  ): int {
    if (
      {%- for t in c['types'] %}
      p{{loop.index}} === null {{ ("&&" if not loop.last) | safe}}
      {%- endfor %}
    ) {
      return this.val - 42;
    } else {
      return 1;
    }
  }
}

function main(): int {
  let c: A = new A({% for t in c['types'] %}null{{ ", " if not loop.last }}{% endfor %});
  return c.foo({% for t in c['types'] %}null{{ ", " if not loop.last }}{% endfor %});
}
{% endfor %}