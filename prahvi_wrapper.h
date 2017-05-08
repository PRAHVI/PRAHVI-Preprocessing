#include <Python.h>
#include "prahvi.hpp"

extern "C" {
	prahvi* Prahvi_new();
	ProcessResult Prahvi_getNewText(prahvi* pv, PyObject* img);
	const char* Prahvi_getText(prahvi* pv);
}
