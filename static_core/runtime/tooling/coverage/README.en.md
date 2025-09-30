# coverage.py Script instructions:

## Precondition

Using this script requires a syntax tree file (.json), a disassembly file (.pa) and ets source code with the same name.

Syntax tree file (.json) is placed in ast directory, disassembly file (.pa) is placed in pa directory, ets source code is placed in src directory, and runtime information (.csv) is placed in runtime_info directory.

The directory structure is as follows:
```
.
├── ast
│   ├── mypackage1
│   │   └── hello.json
│   └── mypackage2
│       └── world.json
├── coverage.py
├── pa
│   ├── mypackage1
│   │   └── hello.pa
│   └── mypackage2
│       └── world.pa
├── runtime_info
│   └── coverageBytecodeInfo.csv
└── src
    ├── mypackage1
    │   └── hello.ets
    └── mypackage2
        └── world.ets
```

## instruction description
Instruction 1: Generate a full coverage html file
```
python coverage.py gen_report --all
or
python coverage.py gen_report -a
```

Instruction 2: Generate an incremental coverage html file
```
python coverage.py gen_report --diff
or
python coverage.py gen_report -d
```


Note: instruction 2 needs to be passed into diff.txt file;
Execute the following instructions to generate diff.txt and put this file in the current directory of the script.
```
git diff HEAD^ HEAD > diff.txt
```