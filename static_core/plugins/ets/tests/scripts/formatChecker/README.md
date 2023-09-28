# STS CTS Format Checker

> Stabilized format for writing tests and a proof of concept test generator uses that format


Tests in STS CTS should be generated to increase developer productivity.
Tests can be generated using the idea of templates and parameters: test developers define programs with placeholder spot (a.k.a. templates) and values which can be substituted in those spots (parameters).

The test generator must define a certain format for writing templates and parameters.
In `Format Checker` we try to stabilize this format by implementing a Proof of Concept tool that eats test files in the specified format and generates ready-to-execute tests.

## Getting Started

1. Install all dependencies: `source formatChecker/setup.sh`; or activate venv if you have already executed the setup script: `source formatChecker/.venv/bin/activate` 

2. Generate CTS: `python3 formatChecker/generate.py CTS ./formatChecker/output`

3. Build migration tool: at project root run `ant build`

4. Run tests: `python3 formatChecker/run_tests.py ./formatChecker/output ../../out/migrator-0.1.jar`

5. Get coverage: `python3 formatChecker/coverage.py ./formatChecker/output [--specpath=./example.spec.yaml]`

## File Structure

TBD

### File Naming Convention 

TBD 

## Format

TBD

### Test meta information 

TBD

### Test file (template) format 

TBD

### Parameter list file format

TBD 