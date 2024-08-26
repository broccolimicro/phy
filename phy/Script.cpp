#include "Script.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <wordexp.h>

namespace ruler {

Tech *tech;

static PyObject* py_dbunit(PyObject *self, PyObject *args) {
	double dbunit = -1;
	if(!PyArg_ParseTuple(args, "d:dbunit", &dbunit)) {
		return NULL;
	}

	tech->dbunit = dbunit;
	return PyFloat_FromDouble(dbunit);
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
	const char *name = 0;
	int overhang = -1;
	if(!PyArg_ParseTuple(args, "si:nmos", &name, &overhang)) {
		return NULL;
	}

	int result = flip((int)tech->models.size());
	tech->models.push_back(Model(Model::NMOS, name, overhang));
	return PyLong_FromLong(result);
}

static PyObject* py_pmos(PyObject *self, PyObject *args) {
	const char *name = 0;
	int overhang = -1;
	if(!PyArg_ParseTuple(args, "si:pmos", &name, &overhang)) {
		return NULL;
	}

	int result = flip((int)tech->models.size());
	tech->models.push_back(Model(Model::PMOS, name, overhang));
	return PyLong_FromLong(result);
}

static PyObject* py_subst(PyObject *self, PyObject *args) {
	int model = -1;
	int draw = -1;
	int label = -1;
	int pin = -1;
	int overhangX = 0;
	int overhangY = 0;
	if(!PyArg_ParseTuple(args, "iiiiii:subst", &model, &draw, &label, &pin, &overhangX, &overhangY)) {
		return NULL;
	}

	model = flip(model);
	int result = (int)tech->models[model].paint.size();
	tech->models[model].paint.push_back(Diffusion(draw, label, pin, vec2i(overhangX, overhangY)));
	return PyLong_FromLong(result);
}

static PyObject* py_via(PyObject *self, PyObject *args) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	int downLevel = -1;
	int upLevel = -1;
	int downLo = 0;
	int downHi = 0;
	int upLo = 0;
	int upHi = 0;
	if(!PyArg_ParseTuple(args, "iiiiiiiii:via", &draw, &label, &pin, &downLevel, &upLevel, &downLo, &downHi, &upLo, &upHi)) {
		return NULL;
	}

	int result = (int)tech->vias.size();
	tech->vias.push_back(Via(draw, label, pin, downLevel, upLevel, downLo, downHi, upLo, upHi));
	return PyLong_FromLong(result);
}

static PyObject* py_route(PyObject *self, PyObject *args) {
	int draw = -1;
	int label = -1;
	int pin = -1;
	if(!PyArg_ParseTuple(args, "iii:route", &draw, &label, &pin)) {
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
	{"paint", py_paint, METH_VARARGS, "Create a paint layer."},
	{"width", py_width, METH_VARARGS, "Set the minimum width for a paint layer."},
	{"fill", py_fill, METH_VARARGS, "Indicate a fill layer."},
	{"nmos", py_nmos, METH_VARARGS, "Create an nmos transistor model."},
	{"pmos", py_pmos, METH_VARARGS, "Create a pmos transistor model."},
	{"subst", py_subst, METH_VARARGS, "Add a diffusion layer to the bottom of a model."},
	{"via", py_via, METH_VARARGS, "Add a via model."},
	{"route", py_route, METH_VARARGS, "Add a route model."},
	{"spacing", py_spacing, METH_VARARGS, "Add a spacing rule."},
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

	PyConfig config;
	PyConfig_InitPythonConfig(&config);

	wordexp_t args;
	if (wordexp(path.c_str(), &args, WRDE_APPEND) != 0) {
		return false;
	}

	int argc = args.we_wordc + 1;
	char **argv = new char*[argc];
	argv[0] = args.we_wordv[0];
	for (int i = 0; i < (int)args.we_wordc; i++) {
		argv[i+1] = args.we_wordv[i];
	}

	PyStatus status = PyConfig_SetBytesArgv(&config, argc, argv);
	if (PyStatus_Exception(status)) {
		wordfree(&args);
		delete [] argv;
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
		wordfree(&args);
		delete [] argv;
		tech = nullptr;
		PyConfig_Clear(&config);
		if (not PyStatus_IsExit(status)) {
			Py_ExitStatusException(status);
		}
		return false;
	}
	PyConfig_Clear(&config);

	bool success = true;
	FILE *fptr = fopen(args.we_wordv[0], "r");
	if (fptr != nullptr) {
		PyRun_SimpleFile(fptr, args.we_wordv[0]);
		fclose(fptr);
		success = (Py_FinalizeEx() >= 0);
	}

	delete [] argv;
	wordfree(&args);
	tech = nullptr;
	return success;
}


}
