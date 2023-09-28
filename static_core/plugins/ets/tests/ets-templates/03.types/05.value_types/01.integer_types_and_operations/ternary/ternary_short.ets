{% for v in values %}
/*---
desc: check ternary if-else operation with condition of short integer operand
---*/

function main(): void {
    const a: short = {{v.condition}}
    const b: short = {{v.ifTrue}}
    const c: short = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
