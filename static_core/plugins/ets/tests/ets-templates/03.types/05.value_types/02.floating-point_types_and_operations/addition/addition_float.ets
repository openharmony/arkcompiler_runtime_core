{% for v in values %}
/*---
desc: check addition of two floats
---*/

function main(): void {
    const a: float = {{v.left}} as float
    const b: float = {{v.right}} as float
    const v: float = {{v.result}} as float
    const r: float = a + b
    if (Float.isNaN(v)) {
        assert(Float.isNaN(r))
    } else {
        assert r == v
    }
}

{% endfor %}
