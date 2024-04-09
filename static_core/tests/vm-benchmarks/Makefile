.PHONY: clean

VMB = ./run-vmb.sh

clean:
	rm -rf dist generated src/generated examples/benchmarks/generated
	rm -rf .pytest_cache
	rm -rf src/vmb/.pytest_cache
	rm -rf src/vmb.egg-info
	rm -rf .mypy_cache
	rm -rf .tox
	find . -type d -name __pycache__ -exec rm -rf {} +
	python3 -m pip uninstall -y vmb || true

vmb: clean
	python3 -m build
	python3 -m pip install dist/vmb-*-py3-none-any.whl

help:
	$(VMB) help || true
	$(VMB) version
	$(VMB) list

tox:
	python3 -m tox

