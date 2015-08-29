PYTHON    = python
TSCRIPT   = test/test_tis.py

install: build
	$(PYTHON) setup.py install --user

test: install
	 $(PYTHON) $(TSCRIPT)

test_valgrind: install
	valgrind  --suppressions=valgrind-python.supp $(PYTHON) $(TSCRIPT) 