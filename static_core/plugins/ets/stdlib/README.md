# Style guide

## Spacing
Remove all trailing whitespaces

File must end with a **single** empty line

In type annotations don't place a space before colon and place single after, like `let x: number | null = null`

Always use block statement within braces (`{}`) after control structures `if`, `while`, `else`. The only exception is `if` after `else`, where braces must be omitted

Always place space between open parenthesis and control structures such as `if`, `while`. Place space between closing parenthesis and open brace in them as well

Always place spaces between operators and operands, example: `a * b + c`

Always be consistent with semicolons (`;`) after statements before line breaks in a single file: either use them always or never. Each statement must be on a separate line

Minimize redundant parenthesis around non bitwise operators. `&&`, `||` and `as` are not exceptions

Always indent statements with 4 spaces within block statement (`{}`) that takes more than one line including braces. Place open brace `{` on the last line of declaring construct, without newline beforehand, but with one after. Place closing brace `}` on a new line. If block statement is empty or consists of a single short line, newlines can be omitted. Single line statement must be separated from braces with a single space from each side. Examples:
```ts
if (status == 200) { return true; }
if (status == 404) {
	this.getEnvironment().getLogger().debug("...");
}
```

Place `else` on the line of `if`'s closing brace `}`, if then clause wasn't a one-liner. And don't embrace this if into a block statement. Examples:
```ts
if (ok) { console.log('123') }
else { console.log('345') }
if (ok) {
	// ...
} else if (ok1) {
	// ...
} else {
	// ...
}
```

Empty braces for single-block constructs must be placed on the same line. Example:
```ts
function nop(): void {}
```

Braces must be written in same style for multi-block statements. Examples:
```ts
if (ok) { /* ... */ }
else if (ok) { /* ... */ }
else { /* ... */ }
// ^ valid formatting

try {
    // ...
} catch (e) {} // <- invalid

if (ok) { fn() }
else { // <- invalid
    // ...
}
```

### Vertical spaces

Don't use vertical spaces longer than two lines in declarations and longer than one line in code. Declare fields without getters and setters without vertical space between them if they belong to same logical group, or with one to emphasize that they belong to different. Use single vertical space between all methods, and optionally double between their groups. Examples:

```ts
class Foo {
    private a1: int;
    private a2: int;

    private b1: number;

    constructor() {}

    constructor(a1: int) {
        this.a1 = a1
    }


    function foo(): void {}

    function fooDeprecated(): void {}
}
```


## Naming
Name types in camel case, starting with and upper case later, like `VeryLongTypeName`

Name packages, properties and methods in camel case, like `myMethod`. Write entire abbreviation in single case according to rules, like `xmlParser()` or `getXMLParser()`

Name global constants in upper snake case, like `NOT_USED`. This rule also applies to `enum` members

Don't use prefixes or suffixes, including `_`, "Hungarian notation" and others. Also don't use `_` itself as an identifier

## Declarations

### Local variables
Prefer to omit type annotations

Always start with `const` and promote to `let` only if needed

Avoid variable shadowing

### Class members
Don't omit `public` and `override`

Prefer most secure applicable visibility: `public` < `protected` < `internal` < `private`

## Exception handling

If "exceptional" control path is "expected", as if file is absent on opening, use `Exception`s. Otherwise, if it is a consequence of a bug, such as invalid argument, use `Error`s

Minimize amount of statements within all blocks of `try`-`catch`-`finally`

Don't `return` from `finally`

Don't annotate `main` with `throws`

## Comments
For documentation use doc comments, that start with `/**` instead of `/*` and don't use this notation for regular comments

For commenting out parts of code, don't use spaces after `//` and do for text comments. Example:
```ts
// TODO: commented out until #... is fixed
//if (...) {
//    someMethod1();
//    someMethod2();
//}
```

Prefer using types and identifiers over comments

## `switch` statement
Always add `default` case if `switch` doesn't cover all alternatives. Place `assert false` if necessary

Mark fallthrough cases with comment `// fallthrough`

## TODO!!!
Column limit, line wrapping
