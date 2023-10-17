# DECLGEN TOOL FOR `.d.sts` FILES GENERATION

## PREREQUISITES

in order to use tool, you need to have `third_party` initialized by the cmake.

## INSTALL

```
cd declgen_ts2ets/
npm i
```

## COMPILE TOOL

```
npm run compile
```

## RUN TOOL

```
FNAME=/path/to/the/ts/file OUT_PATH=/path/to/the/directory/for/the/generated/decl/files npm run declgen
```

this command reads the provided `${FNAME}` `.ts` file and generates corresponding declaration file for the static VM in the `${OUT_PATH}` folder

## TESTS

tests are run with two scripts:

- `./scripts/ohos_sdk_tests.sh` - runs es2panda on the main file that imports generated declarations from ohos sdk. usage: `./scripts/ohos_sdk_tests.sh /path/to/ohos/sdk/ /path/to/panda/build`
- `.scripts/templated_tests.sh` - generates set of simple tests using templates in the `templates` folder and runs them. assures the format of tool's output
