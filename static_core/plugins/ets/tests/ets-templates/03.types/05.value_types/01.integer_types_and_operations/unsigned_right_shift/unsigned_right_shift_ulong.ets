{% for v in values %}
/*---
desc: check unsigned right shift operation for unsigned long integer
---*/

function main(): void {
    const a: ulong = {{v.left}}
    const b: int = {{v.right}}
    assert (a >>> b) == {{v.result}}
}

{% endfor %}
