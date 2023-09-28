{% for v in values %}
/*---
desc: check ternary if-else operation with condition of integer operand
---*/

function main(): void {
    const a: int = {{v.condition}}
    const b: int = {{v.ifTrue}}
    const c: int = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
