# coverage.py 脚本使用说明：

## 前置条件

使用此脚本同级目录需要有同名的语法树文件(.json)，反汇编文件(.pa)，以及ets源码。

语法树文件(.json)放在ast目录，反汇编文件(.pa)放在pa目录，ets源码放在src目录，运行时信息(.csv)放在runtime_info目录。

目录结构如下：

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

## 指令说明

指令1：生成全量覆盖率html文件

```
python coverage.py gen_report --all
或
python coverage.py gen_report -a
```

指令2：生成增量覆盖率html文件

```
python coverage.py gen_report --diff
或
python coverage.py gen_report -d
```

注：指令2 需要传入diff.txt文件；
执行如下指令，生成diff.txt，将此文件放到脚本当前目录。

```
git diff HEAD^ HEAD > diff.txt
```