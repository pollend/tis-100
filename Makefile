PYTHON    = python
TSCRIPT   = test/test_tis.py

install: build
	$(PYTHON) setup.py install --user

test: install
	$(PYTHON) $(TSCRIPT)