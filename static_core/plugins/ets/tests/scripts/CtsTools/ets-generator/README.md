# eTS CTS Test Generator

Tests generated using templates and parameters.


## Requirements

1. python 3.8
2. Jinja2 3.1.2
3. PyYAML 6.0



## Getting Started

1. Install python3: `sudo apt install python3`

2. Install python3-venv: `sudo apt install -y python3-venv`

3. Instal all requirements: `python3 -m pip install -r requirements.txt`

4. Generate tests: `python3 generator.py -t {test directory} -o {output directory}`



## Supported command-line arguments

| Option full name | Optional short name | Description                                                                        |
| ---------------- | ------------------- | ---------------------------------------------------------------------------------- |
|--test-dir        | -t                  | Path to eTS templates directory.                                                   |
|--output-dir      | -o                  | Generation results location. Tool will store all generated test in this directory. |



## Test naming convention

Every file name consists of a user-defined descriptive name and optionally several predefined suffixes that redlect how the file should be treated by the test generator.

Main part of name is used to distinguish between tests. It should be describe the contents of the test.
May contain the following characters: [a-zA-Z0-9_].

Standard extension for template is ".ets"

###### Sample:
`expression_statements.ets`.



## Test body

### Meta information

Template have meta-information in YAML format [name: value] and placed in `/*---  ---*/` symbols.

Meta-information can contains:
1. Description
2. Assertion
3. Parameters
4. Tags for right test running.

#### Description

Description of the test.
Marked as 'desc:'

#### Assertion

Assertion from spec
Marked as 'assert:'

#### Parameters

Parameters show what parameters are used in the test. Тeeded to define a test when using jinja2 templates.
Marked as 'params:'

#### Tags

Tags define how to properly run test.
Marked as 'tags:' and is a string array.

Current active tags:
1. negative - for negative tests, that produce an error while running.
2. compile-only - for tests without main function.

###### Sample:
```
/*---
desc: Class initializer with return statement.
assert: It is a compile-time error if a return statement appears anywhere within a class initializer.
params: {{item.expr}}
tags: [compile-only, negative]
---*/
```

#### Ark Options

You can provide test-specific options for use in ark command line. 'ark_options' field can be specified either as a string or as an array.

###### Samples:
Multiple Ark options specified in an array (array is the recommended way):
```
/*---
desc: Class initializer with return statement.
assert: It is a compile-time error if a return statement appears anywhere within a class initializer.
params: {{item.expr}}
ark_options: ['--heap-size-limit=67108864', '--init-heap-size-limit=67108864']
---*/
```

A single Ark option specified as a string value:
```
/*---
desc: Class initializer with return statement.
assert: It is a compile-time error if a return statement appears anywhere within a class initializer.
params: {{item.expr}}
ark_options: '--heap-size-limit=67108864'
---*/
```

#### Test Timeout

You can provide test-specific timeout in seconds using 'timeout' field. If not specified, then the default timeout, set by the test runner is used.

###### Samples:
Set test timeout to 45 seconds:

```
/*---
desc: Class initializer with return statement.
params: {{item.expr}}
timeout: 45
---*/
```

### Code block

Code placed after meta information block.


### Jinja2 template system

Tests generator can use 'Jinja2' library for template structure. This allows to generate multiple scenarios from one test.
Jinja using special parameters files, that must be located in same directory as test.

Jinja have three kinds of delimiters:

1. {% ... %} for Statements

2. {{ ... }} for Expressions to print to the template output

3. {# ... #} for Comments not included in the template output


Some useful formatting data filters:

1. {{var | safe}} - prevents html-escaping of string values, keeps all characters as-is, for example, without 'safe', angle brackets are escaped into \&lt; and \&gt;

2. {{var | indent}} - preserves 4 space indentation for multi-line strings, helps to make the generated code look tidy

3. {{var | capitalize}} - make the first letter capital, useful for getting boxed class names: {{"boolean" | capitalize}} = "Boolean"

More information on filters could be found in Jinja2 library documentation.

###### Sample:
```
{% for item in expression_statements %}

/*---
desc: Expression statements
params: {{item.expr}}
---*/

function main(): void {
  {{item.expr}};
}

{% endfor %}
```


### Copyright

Also test can be copyrighted, but if not, copyright text will be added automatically.

###### Copyright text:
```
Copyright (c) 2021-2023 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```



## Parameter file naming convention

Parameter file contains list of variable for jinja2 template. File can only be referenced by the tests that are located in the same directory and have the same name as the list itself.

Name for that files consists: {name of test}.{suffix 'params'}.{extension 'yaml'}.

###### Sample:
`expression_statements.params.yaml`.

## Parameter file body

File contains a list of parameters in YAML format.

If a string value is multi-line, to keep generated code tidy, specific indicators should be used. See online tool https://yaml-multiline.info/ for help in choosing right indicators.

###### Samples:
```
--- # List of valid expression statements
expression_statements:
  - {expr: "5"}
  - {expr: "1 + 2"}
  - {expr: "[\"hello\", \"world\"]"}
  - {expr: "[]"}
```

```
--- # List of string
strings:
  - "s"
  - "bigger string"
  - "{expr: "[\"hello\", \"world\"]"}"
  - ""
```
