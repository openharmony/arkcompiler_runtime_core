{% for v in values %}
/*---
desc: check unsigned right shift operation for short integer
---*/

function main(): void {
    const a: short = {{v.left}}
    const b: int = {{v.right}}
    assert (a >>> b) == {{v.result}}
}

{% endfor %}
