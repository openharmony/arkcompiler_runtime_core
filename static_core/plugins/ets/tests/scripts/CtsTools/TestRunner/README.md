
Please use **Universal Rest Runner** for running ArkTS CTS tests.

Detailed information on Universal Test Runner can be found in `tests/tests-u-runner/readme.md`

**Notes:**

- The Universal Test Runner incorporates the test generator, so, there's no need to generate the tests before run.
- If you run multiple instances of the Universal Test Runner at the same time, specify different WORK_DIR locations.

**Example:**
```
SRC_DIR=${HOME}/arkcompiler_runtime_core
BUILD_DIR=${HOME}/builds/panda-default-fv
WORK_DIR=${HOME}/tmp

${SRC_DIR}/tests/tests-u-runner/runner.sh \
  ${SRC_DIR} \
  --ets-cts \
  --es2panda-opt-level=0 \
  --interpreter-type=cpp \
  --timeout=30 \
  --gc-type=g1-gc \
  --heap-verifier=fail_on_verification:pre:into:before_g1_concurrent:post \
  --force-generate \
  --work-dir ${WORK_DIR}  \
  --build-dir ${BUILD_DIR}  \
  --test-root ${SRC_DIR}/plugins/ets
```
