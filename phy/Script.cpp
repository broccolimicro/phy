#include "Script.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <filesystem>

std::vector<std::string> splitArguments(const std::string& input) {
	std::vector<std::string> args;
	std::string current;
	bool inSingleQuotes = false;
	bool inDoubleQuotes = false;
	bool inEscape = false;

	for (size_t i = 0; i < input.size(); i++) {
		char c = input[i];

		if (inEscape) {
			current += c;
			inEscape = false;
		} else if (c == '\\') {
			inEscape = true;
		} else if (c == '\'' && !inDoubleQuotes) {
			inSingleQuotes = !inSingleQuotes;
		} else if (c == '"' && !inSingleQuotes) {
			inDoubleQuotes = !inDoubleQuotes;
		} else if (isspace(c) && !inSingleQuotes && !inDoubleQuotes) {
			if (!current.empty()) {
				args.push_back(current);
				current.clear();
			}
		} else {
			current += c;
		}
	}

	if (!current.empty()) {
		args.push_back(current);
	}

	return args;
}

namespace phy {

Tech *tech;

static PyObject* py_dbunit(PyObject *self, PyObject *args) {
	double dbunit = -1;
	if(!PyArg_ParseTuple(args, "d:dbunit", &dbunit)) {
		return NULL;
	}

	tech->dbunit = dbunit;
	return PyFloat_FromDouble(dbunit);
}

static PyObject* py_scale(PyObject *self, PyObject *args) {
	double scale = -1;
	if(!PyArg_ParseTuple(args, "d:scale", &scale)) {
		return NULL;
	}

	tech->scale = scale;
	return PyFloat_FromDouble(scale);
}


static PyObject* py_paint(PyObject *self, PyObject *args) {
	const char *name = 0;
	int major = -1;
	int minor = -1;
	if(!PyArg_ParseTuple(args, "sii:paint", &name, &major, &minor)) {
		return NULL;
	}

	int result = (int)tech->paint.size();
	tech->paint.push_back(Paint(name, major, minor));
	return PyLong_FromLong(result);
}

static PyObject* py_width(PyObject *self, PyObject *args) {
	int layer = -1;
	int width = -1;
	if(!PyArg_ParseTuple(args, "ii:width", &layer, &width)) {
		return NULL;
	}

	tech->paint[layer].minWidth = width;
	return PyLong_FromLong(layer);
}

static PyObject* py_fill(PyObject *self, PyObject *args) {
	int layer = -1;
	if(!PyArg_ParseTuple(args, "i:fill", &layer)) {
		return NULL;
	}

	tech->paint[layer].fill = true;
	return PyLong_FromLong(layer);
}

static PyObject* py_nmos(PyObject *self, PyObject *args) {
	const char *variant = 0;
	const char *name = 0;
	
	PyObject *pList;
	PyObject *pItem;
	Py_ssize_t n;

	if(!PyArg_ParseTuple(args, "ssO!:nmos", &variant, &name, &PyList_Type, &pList)) {
		return NULL;
	}

	vector<int> stack;
	n = PyList_Size(pList);
	for (int i = 0; i < n; i++) {
		pItem = PyList_GetItem(pList, i);
		if(!PyLong_Check(pItem)) {
				PyErr_SetString(PyExc_TypeError, "list items must be integers.");
				return NULL;
		}
		stack.push_back(PyLong_AsLong(pItem));
	}
	
	printf("loaded model %s %s {", variant, name);
	for (int i = 0; i < (int)stack.size(); i++) {
		printf("%d ", stack[i]);
	}
	printf("}\n");

	int result = flip(tech->models.size());
	tech->models.push_back(Model(Model::NMOS, variant, name, stack));
	return PyLong_FromLong(result);
}


static PyObject* py_pmos(PyObject *self, PyObject *args) {
	const char *variant = 0;
	const char *name = 0;
	
	PyObject *pList;
	PyObject *pItem;
	Py_ssize_t n;

	if(!PyArg_ParseTuple(args, "ssO!:pmos", &variant, &name, &PyList_Type, &pList)) {
		return NULL;
	}

	vector<int> stack;
	n = PyList_Size(pList);
	for (int i = 0; i < n; i++) {
		pItem = PyList_GetItem(pList, i);
		if(!PyLong_Check(pItem)) {
				PyErr_SetString(PyExc_TypeError, "list items must be integers.");
				return NULL;
		}
		stack.push_back(PyLong_AsLong(pItem));
	}

	printf("loaded model %s %s {", variant, name);
	for (int i = 0; i < (int)stack.size(); i++) {
		printf("%d ", stack[i]);
	}
	printf("}\n");

	int result = flip(tech->models.size());
	tech->models.push_back(Model(Model::PMOS, variant, name, stack));
	return PyLong_FromLong(result);
}

static PyObject* py_subst(PyObject *self, PyObject *args) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	if(!PyArg_ParseTuple(args, "i|ii:subst", &draw, &label, &pin)) {
		return NULL;
	}

	int result = flip((int)tech->subst.size());
	tech->subst.push_back(Diffusion(draw, label, pin, false));
	printf("subst:%d (%d %d %d)\n", result, draw, label, pin);
	return PyLong_FromLong(result);
}

static PyObject* py_well(PyObject *self, PyObject *args) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	if(!PyArg_ParseTuple(args, "i|ii:well", &draw, &label, &pin)) {
		return NULL;
	}

	int result = flip((int)tech->subst.size());
	tech->subst.push_back(Diffusion(draw, label, pin, true));
	printf("well:%d (%d %d %d)\n", result, draw, label, pin);
	return PyLong_FromLong(result);
}

static PyObject* py_via(PyObject *self, PyObject *args) {
	int dn = -1;
	int up = -1;
	int draw = -1;
	int label = -1;
	int pin = -1;
	if(!PyArg_ParseTuple(args, "iii|ii:via", &dn, &up, &draw, &label, &pin)) {
		return NULL;
	}

	int result = (int)tech->vias.size();
	tech->vias.push_back(Via(dn, up, draw, label, pin));
	return PyLong_FromLong(result);
}

static PyObject* py_route(PyObject *self, PyObject *args) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	if(!PyArg_ParseTuple(args, "i|ii:route", &draw, &label, &pin)) {
		return NULL;
	}

	int result = (int)tech->wires.size();
	tech->wires.push_back(Routing(draw, label, pin));
	return PyLong_FromLong(result);
}

static PyObject* py_spacing(PyObject *self, PyObject *args) {
	int l0 = -1;
	int l1 = -1;
	int value = -1;
	if(!PyArg_ParseTuple(args, "iii:spacing", &l0, &l1, &value)) {
		return NULL;
	}

	int result = tech->setSpacing(l0, l1, value);
	return PyLong_FromLong(result);
}

static PyObject* py_enclosing(PyObject *self, PyObject *args) {
	int l0 = -1;
	int l1 = -1;
	int lo = -1;
	int hi = -1;
	if(!PyArg_ParseTuple(args, "iii|i:enclosing", &l0, &l1, &lo, &hi)) {
		return NULL;
	}

	int result = tech->setEnclosing(l0, l1, lo, hi);

	printf("enclosing %d around %d: (%d,%d)\n", l0, l1, lo, hi);
	tech->print(result);

	return PyLong_FromLong(result);
}

static PyObject* py_b_and(PyObject *self, PyObject *args) {
	int l0 = -1;
	int l1 = -1;
	if(!PyArg_ParseTuple(args, "ii:b_and", &l0, &l1)) {
		return NULL;
	}

	int result = tech->setAnd(l0, l1);
	return PyLong_FromLong(result);
}

static PyObject* py_b_or(PyObject *self, PyObject *args) {
	int l0 = -1;
	int l1 = -1;
	if(!PyArg_ParseTuple(args, "ii:b_or", &l0, &l1)) {
		return NULL;
	}

	int result = tech->setOr(l0, l1);
	return PyLong_FromLong(result);
}

static PyObject* py_b_not(PyObject *self, PyObject *args) {
	int l0 = -1;
	if(!PyArg_ParseTuple(args, "i:b_not", &l0)) {
		return NULL;
	}

	int result = tech->setNot(l0);
	return PyLong_FromLong(result);
}

static PyObject* py_bound(PyObject *self, PyObject *args) {
	int l0 = -1;
	if(!PyArg_ParseTuple(args, "i:bound", &l0)) {
		return NULL;
	}

	tech->boundary = l0;
	return PyLong_FromLong(l0);
}

static PyMethodDef EmbMethods[] = {
	{"dbunit", py_dbunit, METH_VARARGS, "Set the dbunit."},
	{"scale", py_scale, METH_VARARGS, "Set the scale."},
	{"paint", py_paint, METH_VARARGS, "Create a paint layer."},
	{"width", py_width, METH_VARARGS, "Set the minimum width for a paint layer."},
	{"fill", py_fill, METH_VARARGS, "Indicate a fill layer."},
	{"nmos", py_nmos, METH_VARARGS, "Create an nmos transistor model."},
	{"pmos", py_pmos, METH_VARARGS, "Create a pmos transistor model."},
	{"subst", py_subst, METH_VARARGS, "Define a diffusion layer."},
	{"well", py_well, METH_VARARGS, "Define a well layer."},
	{"via", py_via, METH_VARARGS, "Add a via model."},
	{"route", py_route, METH_VARARGS, "Add a route model."},
	{"spacing", py_spacing, METH_VARARGS, "Add a spacing rule."},
	{"enclosing", py_enclosing, METH_VARARGS, "Add an enclosing rule."},
	{"b_and", py_b_and, METH_VARARGS, "Intersect two layers."},
	{"b_or", py_b_or, METH_VARARGS, "Union between two layers."},
	{"b_not", py_b_not, METH_VARARGS, "Complement of a layer."},
	{"bound", py_bound, METH_VARARGS, "Set the cell boundary layer."},
	{NULL, NULL, 0, NULL}
};

static PyModuleDef EmbModule = {
	PyModuleDef_HEAD_INIT, "floret", NULL, -1, EmbMethods,
	NULL, NULL, NULL, NULL
};

static PyObject* PyInit_floret()
{
	return PyModule_Create(&EmbModule);
}

bool loadTech(Tech &dst, string path) {
	tech = &dst;

	if (not filesystem::exists(path)) {
		return false;
	}

	PyConfig config;
	PyConfig_InitPythonConfig(&config);

	vector<string> args = splitArguments(path);
	vector<char*> argv;
	for (auto arg = args.begin(); arg != args.end(); arg++) {
		argv.push_back(const_cast<char*>(arg->c_str()));
	}

	PyStatus status = PyConfig_SetBytesArgv(&config, argv.size(), argv.data());
	if (PyStatus_Exception(status)) {
		tech = nullptr;
		PyConfig_Clear(&config);
		if (not PyStatus_IsExit(status)) {
			Py_ExitStatusException(status);
		}
		return false;
	}

	PyImport_AppendInittab("floret", &PyInit_floret);
	status = Py_InitializeFromConfig(&config);
	if (PyStatus_Exception(status)) {
		tech = nullptr;
		PyConfig_Clear(&config);
		if (not PyStatus_IsExit(status)) {
			Py_ExitStatusException(status);
		}
		return false;
	}
	PyConfig_Clear(&config);

	bool success = true;
	FILE *fptr = fopen(argv[0], "r");
	if (fptr != nullptr) {
		PyRun_SimpleFile(fptr, argv[0]);
		fclose(fptr);
		success = (Py_FinalizeEx() >= 0);
	}

	tech = nullptr;
	return success;
}


}

