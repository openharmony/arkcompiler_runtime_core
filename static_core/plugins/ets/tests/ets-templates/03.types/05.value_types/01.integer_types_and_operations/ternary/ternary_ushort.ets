{% for v in values %}
/*---
desc: check ternary if-else operation with condition of short integer operand
---*/

function main(): void {
    const a: ushort = {{v.condition}}
    const b: ushort = {{v.ifTrue}}
    const c: ushort = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
