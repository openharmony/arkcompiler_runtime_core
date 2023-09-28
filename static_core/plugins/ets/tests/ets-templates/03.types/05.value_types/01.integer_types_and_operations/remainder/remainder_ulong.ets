{% for v in values %}
/*---
desc: check remainder of division operation for two unsigned long integer operands
---*/

function main(): void {
    const a: ulong = {{v.left}}
    const b: ulong = {{v.right}}
    assert (a % b) == {{v.result}}
}

{% endfor %}
