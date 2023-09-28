{% for v in values %}
/*---
desc: check bitwise and operation for int operands
---*/

function main(): void {
    const a: int = {{v.left}}
    const b: int = {{v.right}}
    assert (a & b) == {{v.result}}
}

{% endfor %}
