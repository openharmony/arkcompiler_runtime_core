{% for v in values %}
/*---
desc: check unary plus operation for float operand
---*/

function main(): void {
  const v: float = {{v.value}} as float
  const r: float = {{v.result}} as float
    if (Float.isNaN(v)) {
        assert(Float.isNaN(r))
    } else {
      assert +(v) == r
    }
}

{% endfor %}
