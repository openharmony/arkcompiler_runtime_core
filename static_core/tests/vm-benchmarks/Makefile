.PHONY: clean

VMB = ./run-vmb.sh
OUTDIR ?= $(HOME)/vmb-build

clean:
	rm -rf dist generated src/generated examples/benchmarks/generated
	rm -rf .pytest_cache
	rm -rf src/vmb/.pytest_cache
	rm -rf src/vmb.egg-info
	rm -rf .mypy_cache
	rm -rf .tox
	find . -type d -name __pycache__ -exec rm -rf {} +

uninstall:
	python3 -m pip uninstall -y vmb || true

dir:
	mkdir -p $(OUTDIR)

vmb: dir uninstall
	python3 -c 'import build' || python3 -m pip install build
	python3 -m build --outdir=$(OUTDIR)
	python3 -m pip install $(OUTDIR)/vmb-*-py3-none-any.whl

help:
	$(VMB) help || true
	$(VMB) version
	$(VMB) list

tox: dir
	python3 -m tox --workdir=$(OUTDIR)

