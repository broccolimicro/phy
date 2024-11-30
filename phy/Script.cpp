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

static PyObject* tupleFromLevel(Level level) {
	return Py_BuildValue("(ii)", level.type, level.idx);
}

static Level levelFromTuple(PyObject *obj) {
	if (obj == nullptr) {
		return Level();
	} else if (PyTuple_Size(obj) != 2) {
		PyErr_SetString(PyExc_TypeError, "Level must have 2 elements (type and idx)");
		return Level();
	}

	PyObject *p0 = PyTuple_GetItem(obj, 0);
	PyObject *p1 = PyTuple_GetItem(obj, 1);
	if (!PyLong_Check(p0)) {
			PyErr_SetString(PyExc_TypeError, "Level type must be an integer.");
			return Level();
	}
	if (!PyLong_Check(p1)) {
			PyErr_SetString(PyExc_TypeError, "Level idx must be an integer.");
			return Level();
	}

	return Level(PyLong_AsLong(p0), PyLong_AsLong(p1));
}

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

	tech->setWidth(layer, width);
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

static PyObject* py_nmos(PyObject *self, PyObject *args, PyObject *kwargs) {
	const char *variant = 0;
	const char *name = 0;

	PyObject *diff = nullptr;
	
	PyObject *l2 = nullptr;
	PyObject *pItem;
	PyObject *pSub;
	Py_ssize_t n;

	static const char *kwlist[] = {"variant", "name", "diff", "bins", NULL};

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "ssO!|O!:nmos", const_cast<char **>(kwlist), &variant, &name, &PyTuple_Type, &diff, &PyList_Type, &l2)) {
		return NULL;
	}

	vector<pair<int, int> > bins;
	if (l2 != nullptr) {
		n = PyList_Size(l2);
		for (int i = 0; i < n; i++) {
			pItem = PyList_GetItem(l2, i);
			if(!PyTuple_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			if (PyTuple_Size(pItem) != 2) {
				PyErr_SetString(PyExc_TypeError, "bins must have 2 elements (min and max)");
				return NULL;
			}

			int lo = 0;
			int hi = std::numeric_limits<int>::max();
			pSub = PyTuple_GetItem(pItem, 0);
			if(!PyLong_Check(pSub)) {
					PyErr_SetString(PyExc_TypeError, "min value must be an integer.");
					return NULL;
			}
			lo = PyLong_AsLong(pSub);

			pSub = PyTuple_GetItem(pItem, 1);
			if(!PyLong_Check(pSub)) {
					PyErr_SetString(PyExc_TypeError, "max value must be an integer.");
					return NULL;
			}
			hi = PyLong_AsLong(pSub);
			
			bins.push_back(pair<int, int>(lo, hi));
		}
	}

	tech->models.push_back(Model(Model::NMOS, variant, name, levelFromTuple(diff), bins));
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject* py_pmos(PyObject *self, PyObject *args, PyObject *kwargs) {
	const char *variant = 0;
	const char *name = 0;
	
	PyObject *diff = nullptr;

	PyObject *l2 = nullptr;
	PyObject *pItem;
	PyObject *pSub;
	Py_ssize_t n;

	static const char *kwlist[] = {"variant", "name", "diff", "bins", NULL};

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "ssO!|O!:pmos", const_cast<char **>(kwlist), &variant, &name, &PyTuple_Type, &diff, &PyList_Type, &l2)) {
		return NULL;
	}

	vector<pair<int, int> > bins;
	if (l2 != nullptr) {
		n = PyList_Size(l2);
		for (int i = 0; i < n; i++) {
			pItem = PyList_GetItem(l2, i);
			if(!PyTuple_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			if (PyTuple_Size(pItem) != 2) {
				PyErr_SetString(PyExc_TypeError, "bins must have 2 elements (min and max)");
				return NULL;
			}

			int lo = 0;
			int hi = std::numeric_limits<int>::max();
			pSub = PyTuple_GetItem(pItem, 0);
			if(!PyLong_Check(pSub)) {
					PyErr_SetString(PyExc_TypeError, "min value must be an integer.");
					return NULL;
			}
			lo = PyLong_AsLong(pSub);

			pSub = PyTuple_GetItem(pItem, 1);
			if(!PyLong_Check(pSub)) {
					PyErr_SetString(PyExc_TypeError, "max value must be an integer.");
					return NULL;
			}
			hi = PyLong_AsLong(pSub);
			
			bins.push_back(pair<int, int>(lo, hi));
		}
	}

	tech->models.push_back(Model(Model::PMOS, variant, name, levelFromTuple(diff), bins));
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject* py_dielec(PyObject *self, PyObject *args, PyObject *kwargs) {
	PyObject *down = nullptr;
	PyObject *up = nullptr;
	float thickness = 0.0f;
	float permitivity = 0.0f;

	static const char *kwlist[] = {"down", "up", "thick", "permit", NULL};

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!f|f:dielec", const_cast<char **>(kwlist), &PyTuple_Type, &down, &PyTuple_Type, &up, &thickness, &permitivity)) {
		return NULL;
	}

	tech->dielec.push_back(Dielectric(levelFromTuple(down), levelFromTuple(up), thickness, permitivity));
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject* py_subst(PyObject *self, PyObject *args, PyObject *kwargs) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	float thickness = 0.0f;
	float resistivity = 0.0f;

	PyObject *well = nullptr;

	PyObject *l0 = nullptr;
	PyObject *l1 = nullptr;
	PyObject *pItem;
	Py_ssize_t n;

	static const char *kwlist[] = {"draw", "label", "pin", "mask", "excl", "well", "thick", "resist", NULL};

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "i|iiO!O!O!ff:subst", const_cast<char **>(kwlist), &draw, &label, &pin, &PyList_Type, &l0, &PyList_Type, &l1, &PyTuple_Type, &well, &thickness, &resistivity)) {
		return NULL;
	}

	vector<int> mask;
	if (l0 != nullptr) {
		n = PyList_Size(l0);
		for (int i = 0; i < n; i++) {
			pItem = PyList_GetItem(l0, i);
			if(!PyLong_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			mask.push_back(PyLong_AsLong(pItem));
		}
	}

	vector<int> excl;
	if (l1 != nullptr) {
		n = PyList_Size(l1);
		for (int i = 0; i < n; i++) {
			pItem = PyList_GetItem(l1, i);
			if(!PyLong_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			excl.push_back(PyLong_AsLong(pItem));
		}
	}

	int result = (int)tech->subst.size();
	tech->subst.push_back(Substrate(draw, label, pin, levelFromTuple(well), thickness, resistivity));
	tech->subst.back().mask = mask;
	tech->subst.back().excl = excl;
	return tupleFromLevel(Level(Level::SUBST, result));
}

static PyObject* py_well(PyObject *self, PyObject *args, PyObject *kwargs) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	float thickness = 0.0f;
	float resistivity = 0.0f;

	PyObject *l0 = nullptr;
	PyObject *l1 = nullptr;
	PyObject *pItem;
	Py_ssize_t n;

	static const char *kwlist[] = {"draw", "label", "pin", "mask", "excl", "thick", "resist", NULL};

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "i|iiO!O!ff:well", const_cast<char **>(kwlist), &draw, &label, &pin, &PyList_Type, &l0, &PyList_Type, &l1, &thickness, &resistivity)) {
		return NULL;
	}

	vector<int> mask;
	if (l0 != nullptr) {
		n = PyList_Size(l0);
		for (int i = 0; i < n; i++) {
			pItem = PyList_GetItem(l0, i);
			if(!PyLong_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			mask.push_back(PyLong_AsLong(pItem));
		}
	}

	vector<int> excl;
	if (l1 != nullptr) {
		n = PyList_Size(l1);
		for (int i = 0; i < n; i++) {
			pItem = PyList_GetItem(l1, i);
			if(!PyLong_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			excl.push_back(PyLong_AsLong(pItem));
		}
	}

	int result = (int)tech->subst.size();
	tech->subst.push_back(Substrate(draw, label, pin, Level(), thickness, resistivity));
	tech->subst.back().mask = mask;
	tech->subst.back().excl = excl;
	return tupleFromLevel(Level(Level::SUBST, result));
}

static PyObject* py_via(PyObject *self, PyObject *args, PyObject *kwargs) {
	PyObject *dn = nullptr;
	PyObject *up = nullptr;
	int draw = -1;
	int label = -1;
	int pin = -1;
	float thickness = 0.0f;
	float resistivity = 0.0f;

	static const char *kwlist[] = {"down", "up", "draw", "label", "pin", "thick", "resist", NULL};

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!i|iiff:via", const_cast<char **>(kwlist), &PyTuple_Type, &dn, &PyTuple_Type, &up, &draw, &label, &pin, &thickness, &resistivity)) {
		return NULL;
	}

	int result = (int)tech->vias.size();
	tech->vias.push_back(Via(levelFromTuple(dn), levelFromTuple(up), draw, label, pin, thickness, resistivity));
	return tupleFromLevel(Level(Level::VIA, result));
}

static PyObject* py_route(PyObject *self, PyObject *args, PyObject *kwargs) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	float thickness = 0.0f;
	float resistivity = 0.0f;

	static const char *kwlist[] = {"draw", "label", "pin", "thick", "resist", NULL};

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "i|iiff:route", const_cast<char **>(kwlist), &draw, &label, &pin, &thickness, &resistivity)) {
		return NULL;
	}

	int result = (int)tech->wires.size();
	tech->wires.push_back(Routing(draw, label, pin, thickness, resistivity));
	return tupleFromLevel(Level(Level::ROUTE, result));
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

	if (l0 < 0) {
		PyErr_SetString(PyExc_TypeError, "enclosing layer must be a paint layer.");
		return NULL;
	}

	int result = tech->setEnclosing(l0, l1, lo, hi);

	return PyLong_FromLong(result);
}

static PyObject* py_b_and(PyObject *self, PyObject *args) {
	vector<int> l;
	Py_ssize_t n;
	PyObject *pItem;
	if (args != nullptr) {
		n = PyTuple_Size(args);
		for (int i = 0; i < n; i++) {
			pItem = PyTuple_GetItem(args, i);
			if (not PyLong_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			l.push_back(PyLong_AsLong(pItem));
		}
	}
	if ((int)l.size() < 2) {
		PyErr_SetString(PyExc_TypeError, "b_and requires at least two layers.");
		return NULL;
	}

	int result = tech->setAnd(l);
	return PyLong_FromLong(result);
}

static PyObject* py_b_or(PyObject *self, PyObject *args) {
	vector<int> l;
	Py_ssize_t n;
	PyObject *pItem;
	if (args != nullptr) {
		n = PyTuple_Size(args);
		for (int i = 0; i < n; i++) {
			pItem = PyTuple_GetItem(args, i);
			if (not PyLong_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "list items must be integers.");
					return NULL;
			}
			l.push_back(PyLong_AsLong(pItem));
		}
	}
	if ((int)l.size() < 2) {
		PyErr_SetString(PyExc_TypeError, "b_or requires at least two layers.");
		return NULL;
	}

	int result = tech->setOr(l);
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
	{"nmos", (PyCFunction)py_nmos, METH_VARARGS | METH_KEYWORDS, "Create an nmos transistor model."},
	{"pmos", (PyCFunction)py_pmos, METH_VARARGS | METH_KEYWORDS, "Create a pmos transistor model."},
	{"dielec", (PyCFunction)py_dielec, METH_VARARGS | METH_KEYWORDS, "Define a dielectric layer."},
	{"subst", (PyCFunction)py_subst, METH_VARARGS | METH_KEYWORDS, "Define a diffusion layer."},
	{"well", (PyCFunction)py_well, METH_VARARGS | METH_KEYWORDS, "Define a well layer."},
	{"via", (PyCFunction)py_via, METH_VARARGS | METH_KEYWORDS, "Add a via model."},
	{"route", (PyCFunction)py_route, METH_VARARGS | METH_KEYWORDS, "Add a route model."},
	{"spacing", py_spacing, METH_VARARGS, "Add a spacing rule."},
	{"enclosing", py_enclosing, METH_VARARGS, "Add an enclosing rule."},
	{"b_and", py_b_and, METH_VARARGS, "Intersect two layers."},
	{"b_or", py_b_or, METH_VARARGS, "Union between two layers."},
	{"b_not", py_b_not, METH_VARARGS, "Complement of a layer."},
	{"bound", py_bound, METH_VARARGS, "Set the cell boundary layer."},
	{NULL, NULL, 0, NULL}
};

static PyModuleDef EmbModule = {
	PyModuleDef_HEAD_INIT, "loom", NULL, -1, EmbMethods,
	NULL, NULL, NULL, NULL
};

static PyObject* PyInit_loom()
{
	return PyModule_Create(&EmbModule);
}

bool loadTech(Tech &dst, string path, string cells) {
	tech = &dst;

	vector<string> args = splitArguments(path);
	if (not filesystem::exists(args[0])) {
		return false;
	}
	dst.path = args[0];
	dst.lib = cells;

	PyConfig config;
	PyConfig_InitPythonConfig(&config);

	vector<char*> argv;
	argv.push_back(const_cast<char*>(args[0].c_str()));
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

	PyImport_AppendInittab("loom", &PyInit_loom);
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

