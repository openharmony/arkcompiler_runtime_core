VENV = .venv
VENV_SYMLINK = venv # to ensure correct work of the IDEs
VENV_PYTHON = $(VENV)/bin/python
SYSTEM_PYTHON = $(or $(shell which python3.11), $(shell which python3))
PYTHON = $(or $(wildcard $(VENV_PYTHON)), $(SYSTEM_PYTHON))

test:
	$(PYTHON) -m unittest discover -v -s test -p "*_test.py"
.PHONY: test

mypy:
	MYPYPATH=. $(PYTHON) -m mypy . --explicit-package-bases
.PHONY: mypy

autopep8:
	find test src run.py lib.py record.py -type f -name "*.py" | xargs autopep8 -i
.PHONY: autopep8

tests_full:
	make autopep8
	make test
	make mypy
.PHONY: tests_full

venv:
	rm -rf $(VENV) $(VENV_SYMLINK)
	$(SYSTEM_PYTHON) -m venv $(VENV)
	$(VENV_PYTHON) -m pip install -r requirements.txt
	ln -s $(VENV) $(VENV_SYMLINK)
.PHONY: venv
