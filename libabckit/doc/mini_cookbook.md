### The main goals of this cookbook are to provide recipes for the main supported functions and to help you quickly get started with the LibAbcKit API.

## Get API implementations

```cpp
enum AbckitApiVersion version = ABCKIT_VERSION_RELEASE_1_0_0;
auto impl = AbckitGetApiImpl(version); // top level api with entry points
auto implI = AbckitGetInspectApiImpl(version); // language-independent inspect api
auto implM = AbckitGetModifyApiImpl(version); // language-independent modify api
auto implArkI = AbckitGetArkTSInspectApiImpl(version); // language-dependent inspect api
auto implArkM = AbckitGetArkTSModifyApiImpl(version); // language-dependent modify api

auto implG = AbckitGetGraphApiImpl(version); // language-independent graph api
auto dynG = AbckitGetIsaApiDynamicImpl(version); // language-dependent graph api
auto statG = AbckitGetIsaApiStaticImpl(version); // language-dependent graph api
```
One static instance of each at the beginning of the .cpp file is enough

## Errors
Every API in libabckit sets error value after execution. You can get it with `impl->getLastError()`. If there are no errors, the return value will be ABCKIT_STATUSNO_ERROR.
Сheck the execution status with:
```cpp
assert(impl->getLastError() == ABCKIT_STATUSNO_ERROR);
```

## Strings in LibAbcKit
Here is an useful example of a custom function for working with strings in libabckit:

```cpp
AbckitString *str = implI->createString(file, "new_string");

std::string AbckitStringToString(AbckitFile *file, AbckitString *str)
{
    std::size_t len = 0;
    implI->abckitStringToString(file, str, nullptr, &len);
    auto name = malloc(len + 1);
    implI->abckitStringToString(file, str, name, &len);
    std::string res {name};
    free(name);

    return res;
}
```

# Metadata

## Open/Close/Write

### *Open file*

Mandatory and only entry point

```cpp
AbckitFile *file = implI->openAbc(path_to_abc);
```

Make sure that you have used the WriteAbc() or CloseFile() function after working with the file. This way you will avoid memory leaks

### *Write or Close*

```cpp
impl->writeAbc(file, path_to_output_file); // save changes and close
// or
impl->closeFile(file); // close without saving changes
```

## Traversing

libabckit objects hierarchy:
1. file contains modules ("file" is ".abc file" and "module" is the abstract of "source code file")
2. module contains: namespaces, top level classes, top level functions
3. namespace contains: nested namespaces, classes, functions
4. class contains: functions (methods)
5. functions contains: nested functions

<!-- file module  class  function -> introduce hierarchy -->

All the enumerators in libabckit accepts libabckit object, void* data with user data and callback lambda:
```cpp
void XXXEnumerateYYY(AbckitCoreXXX* xxx, void *data, bool(*cb)(AbckitCoreYYY* yyy, void *data))
```

### *Enumerate methods from class*
```cpp
// AbckitCoreClass *klass
implI->classEnumerateMethods(klass, data, [](AbckitCoreMethod *method, void *data) {
    return true;
});
```

### *Collect all top level classes from module*
```cpp
// AbckitCoreModule *module
std::vector<AbckitCoreClass *> classes;
implI->moduleEnumerateClasses(module, (void*)&classes, [](AbckitCoreClass *klass, void *data) {
    ((std::vector<AbckitCoreClass*>*)data)->emplace.back(klass);
    return true;
});
```

## Inspecting

### *Get method's parent class*
```cpp
// AbckitCoreMethod *method
AbckitCoreClass *klass = implI->functionGetParentClass(method);
```

### *Get value*
<!-- Test: LiteralGetU32_2 -->

```cpp
// AbckitLiteral *res
void GetValues(AbckitValue *u32_res, AbckitValue *double_res, AbckitValue *string_res) {
    uint32_t val = implI->literalGetU32(u32_res);
    double d = implI->valueGetDouble(double_res);
    AbckitString *s = implI->valueGetString(string_res);
}
```

### *Get module name*

```cpp
// AbckitCoreModule *mod
AbckitString *GetModuleName(AbckitCoreModule *mod) {
    return implI->moduleGetName(mod);
}
```


## Metadata Lang API


### *Add annotation for arkts method*
```cpp
void AddAnno(AbckitModifyContext *file, AbckitCoreMethod *method) {
    auto mod = implI->functionGetModule(method);

    // Find annoation interface with name "Anno"
    AbckitCoreAnnotationInterface *ai;
    implI->moduleEnumerateAnnotationInterfaces(mod, &ai, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        auto ai1 = (AbckitCoreAnnotationInterface **)data;
        auto file = implI->annotationInterfaceGetInspectContext(annoI);
        auto str = implI->annotationInterfaceGetName(annoI);
        auto name = AbckitStringToString(file, str);
        if (name == "Anno") {
            (*ai1) = annoI;
        }
        return true;
    });

    // Add "@Anno" annotation for method
    struct AbckitArktsAnnotationCreateParams annoCreateParams;
    annoCreateParams.ai = implArkI->coreAnnotationInterfaceToArkTsAnnotationInterface(ai);
    AbckitArktsAnnotation *anno = implArkM->functionAddAnnotation(file,
        implArkI->coreFunctionToArkTsFunction(method), &annoCreateParams);
}
```

### *Cast AbckitArtksXXX -> AbckitXXX / AbckitXXX -> AbckitArktsXXX*
```cpp
AbckitCoreModule *CastToAbcKit(AbckitArktsInspectApi implArkI, AbckitArktsModule *mod) {
    return implArkI->arkTsModuleToCoreModule(mod); }

AbckitArktsAnnotation *CastToArkTS(AbckitArktsInspectApi implArkI, AbckitCoreAnnotation *anno) {
    return implArkI->coreAnnotationToArkTsAnnotation(anno); }
```

## Modification

### *Create values*

```cpp
AbckitLiteral *res1 = implM->createLiteralString(file, "asdf");
AbckitLiteral *res2 = implM->createLiteralDouble(file, 1.0);
AbckitLiteral *res = implM->createLiteralU32(file, 1);
```

```cpp
AbckitValue *res_u = implM->createValueU1(file, true);
AbckitValue *res_d = implM->createValueDouble(file, 1.2);
```

### *Get value type*
<!-- Test: ValueGetType_1 -->

```cpp
AbckitType *s_type = implI->valueGetType(res_s); // val_s->id == ABCKIT_TYPE_ID_STRING
AbckitType *u_type = implI->valueGetType(res_u); // val_u->id == ABCKIT_TYPE_ID_U1
AbckitType *d_type = implI->valueGetType(res_d); // val_d->id == ABCKIT_TYPE_ID_F32,
```

# Graph


## *Create and destroy graph*
```cpp
AbckitGraph *graph = implI->createGraphFromFunction(method);
// ...
implM->functionSetGraph(method, graph); // save with changes
// or
impl->destroyGraph(graph); // without changes
```

## *Graph traversal*
```cpp
implG->gVisitBlocksRpo(ctxG, &bbs, [](AbckitBasicBlock *bb, void *data) {
    // user lambda
});
```
```cpp
implG->bbVisitSuccBlocks(bb, (void *)&succBBs, []([[maybe_unused]] AbckitBasicBlock *curBasicBlock,
    AbckitBasicBlock *succBasicBlock, void *d) {
    // user lambda
});
```
```cpp
implG->bbVisitPredBlocks(bb, (void *)&predBBs,
    []([[maybe_unused]] AbckitBasicBlock *curBasicBlock, AbckitBasicBlock *succBasicBlock, void *d) {
        // user lambda
    });

// ...
```


### *Collect all basic blocks from graph*
```cpp
std::vector<AbckitBasicBlock *> bbs;
implG->gVisitBlocksRPO(ctxG, &bbs, [](AbckitBasicBlock *bb, void *data) {
    ((std::vector<AbckitBasicBlock *> *)data)->emplace_back(bb);
});
```

### *Collect all inst in basic block*
```cpp
std::vector<AbckitInst *> insts;
for (auto *inst = implG->bbGetFirstInst(bb); inst != nullptr; inst = implG->iGetNext(inst)) {
    insts.emplace_back(inst);
}
```

### *Collect block's succs*
```cpp
std::vector<AbckitBasicBlock *> succBBs;
implG->bbVisitSuccBlocks(bb, (void *)&succBBs,
    []([[maybe_unused]] AbckitBasicBlock *curBasicBlock, AbckitBasicBlock *succBasicBlock, void *d) {
        auto *succs = (std::vector<AbckitBasicBlock *> *)d;
        succs->emplace_back(succBasicBlock);
    });
```

## Graph inspecting

### *Get method*
```cpp
AbckitCoreMethod *method = implG->iGetFunction(curInst);
```

## Graph modification

### *Create basic block*
```cpp
AbckitBasicBlock *empty = implG->bbCreateEmpty(ctxG);
```
### *Create insts with const*
```cpp
AbckitInst *new_inst = implG->gCreateConstantI64(ctxG, 1U);
```

### *Connect and disconnect blocks*
<!-- Test: BBcreateEmptyBlock_1 -->
```cpp
auto *start = implG->gGetStartBasicBlock(ctxG);
auto *bb = implG->bbCreateEmpty(ctxG);

implG->bbInsertSuccBlock(start, bb, 0);
implG->bbEraseSuccBlock(start, 0);
```

### *Insert instructions*
```cpp
implG->bbAddInstBack(bb, some_inst_1);
implG->iInsertAfter(some_inst_1, some_inst_2);
```


## Graph-lang

### *Create instructions*
```cpp
// for static
AbckitInst *neg_inst = statG->iCreateNeg(ctxG, new_inst);
AbckitInst *add_inst = statG->iCreateAdd(ctxG, neg_inst, new_inst);
AbckitInst *ret = statG->iCreateReturnVoid(ctxG);

// for dynamic
AbckitInst *neg_inst = dynG->iCreateNeg(ctxG, new_inst);
AbckitInst *add_inst = dynG->iCreateAdd2(ctxG, neg_inst, new_inst);
AbckitInst *ret = dynG->iCreateReturnundefined(ctxG);
```

### *Create 'print("Hello")' for ArkTS1.0*

```cpp
AbckitInst *str = dynG->iCreateLoadString(ctxG, implM->createString(file, "Hello"));
AbckitInst *print = dynG->iCreateTryldglobalbyname(ctxG, implM->createString(file, "print"));
AbckitInst *callArg = dynG->iCreateCallarg1(ctxG, print, str);
```


# User scenarios

### *Enumerate the names of methods from the module*

```cpp
std::vector<std::string> functionNames;

std::function<void(AbckitCoreFunction *)> cbFunc = [&](AbckitCoreFunction *f) {
    auto funcName = helpers::AbckitStringToString(implI->functionGetName(f));
    functionNames.emplace_back(funcName);
};

std::function<void(AbckitCoreClass *)> cbClass = [&](AbckitCoreClass *c) {
    implI->classEnumerateMethods(c, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
        return true;
    });
};

std::function<void(AbckitCoreNamespace *)> cbNamespce;
cbNamespce = [&](AbckitCoreNamespace *n) {
    implI->namespaceEnumerateNamespaces(n, &cbNamespce, [](AbckitCoreNamespace *n, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
        return true;
    });
    implI->namespaceEnumerateClasses(n, &cbClass, [](AbckitCoreClass *c, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
        return true;
    });
    implI->namespaceEnumerateTopLevelFunctions(n, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(f);
        return true;
    });
};

std::function<void(AbckitCoreModule *)> cbModule = [&](AbckitCoreModule *m) {
    implI->moduleEnumerateNamespaces(m, &cbNamespce, [](AbckitCoreNamespace *n, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
        return true;
    });
    implI->moduleEnumerateClasses(m, &cbClass, [](AbckitCoreClass *c, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
        return true;
    });
    implI->moduleEnumerateTopLevelFunctions(m, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
        return true;
    });
};

implI->fileEnumerateModules(file, &cbModule, [](AbckitCoreModule *m, void *cb) {
    (*reinterpret_cast<std::function<void(AbckitCoreModule *)> *>(cb))(m);
    return true;
});
```


### *Collect predecessor basic blocks*
```cpp
std::vector<AbckitBasicBlock *> predBBs;
implG->bbVisitPredBlocks(bb, (void *)&predBBs,
    []([[maybe_unused]] AbckitBasicBlock *curBasicBlock, AbckitBasicBlock *succBasicBlock, void *d) {
    auto *preds = (std::vector<AbckitBasicBlock *> *)d;
    preds->emplace_back(succBasicBlock);
    });
```