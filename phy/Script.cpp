#include "Script.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <filesystem>

#include <iostream>
#include <cstdlib>

#include <dlfcn.h>
#include <fstream>

class PythonLoader {
public:
	static PythonLoader& inst() {
		static PythonLoader instance; // Guaranteed to be initialized only once
		return instance;
	}

	// Access to Python API functions
	PyObject* (*BuildValue)(const char*, ...);
	Py_ssize_t (*Tuple_Size)(PyObject*);
	PyObject* (*Tuple_GetItem)(PyObject*, Py_ssize_t);
	long (*Long_AsLong)(PyObject*);
	PyObject* (*Long_FromLong)(long);
	PyObject* (*Float_FromDouble)(double);
	int (*Arg_ParseTuple)(PyObject*, const char*, ...);
	int (*Arg_ParseTupleAndKeywords)(PyObject*, PyObject*, const char*, const char *const*, ...);
	Py_ssize_t (*List_Size)(PyObject*);
	PyObject* (*List_GetItem)(PyObject*, Py_ssize_t);
	void (*Err_SetString)(PyObject*, const char*);
	int (*Import_AppendInittab)(const char*, PyObject* (*)());
	void (*Initialize)();
	void (*Finalize)();
	int (*Run_SimpleFile)(FILE*, const char*);
	int (*Run_SimpleString)(const char*);
	int (*FinalizeEx)();
	void (*Config_InitPythonConfig)(PyConfig*);
	PyStatus (*Config_SetBytesArgv)(PyConfig*, size_t, char**);
	void (*Config_Clear)(PyConfig*);
	PyStatus (*InitializeFromConfig)(const PyConfig*);
	int (*Status_Exception)(PyStatus);
	int (*Status_IsExit)(PyStatus);
	void (*ExitStatusException)(PyStatus);

	PyObject* Exc_TypeError;
	PyObject* None;
	PyTypeObject* List_Type;
	PyTypeObject* Tuple_Type;

	operator bool() const {
		return handle != nullptr;
	}

	PyObject* Module_Create(PyModuleDef *def, int version=PYTHON_API_VERSION) {
		if (_Module_Create != nullptr) {
			return _Module_Create(def);
		}
		return _Module_Create2(def, version);
	}

private:
	void* handle;

	PyObject* (*_Module_Create)(PyModuleDef*);
	PyObject* (*_Module_Create2)(PyModuleDef*, int);

	PythonLoader() {
		handle = nullptr;
		if (load()) {
			resolve();
		}
	}

	~PythonLoader() {
		close();
	}

	PythonLoader(const PythonLoader&) = delete;
	PythonLoader &operator=(const PythonLoader&) = delete;

	bool load() {
		vector<string> names = {
#if defined(_WIN32) || defined(_WIN64)
			"python315.dll",
			"libpython3.15.dll",
			"python314.dll",
			"libpython3.14.dll",
			"python313.dll",
			"libpython3.13.dll",
			"python312.dll",
			"libpython3.12.dll",
			"python311.dll",
			"libpython3.11.dll",
			"python310.dll",
			"libpython3.10.dll",
			"python309.dll",
			"libpython3.09.dll",
			"python3.dll"
			"libpython3.dll"
#elif defined(__MACH__) || defined(__APPLE__)
			"libpython3.15.dylib",
			"libpython3.14.dylib",
			"libpython3.13.dylib",
			"libpython3.12.dylib",
			"libpython3.11.dylib",
			"libpython3.10.dylib",
			"libpython3.9.dylib",
			"libpython3.dylib"
#else
			"libpython3.15.so",
			"libpython3.14.so",
			"libpython3.13.so",
			"libpython3.12.so",
			"libpython3.11.so",
			"libpython3.10.so",
			"libpython3.9.so",
			"libpython3.so"
#endif
		};

		for (auto name = names.begin(); name != names.end(); name++) {
			handle = dlopen(name->c_str(), RTLD_NOW | RTLD_GLOBAL);
			if (handle) {
				return true;
			}
		}
		printf("Unable to load python3.\n");
		char *err = dlerror();
		if (err != nullptr) {
			printf("%s\n", err);
		}
		handle = nullptr;
		return false;
	}

	template <typename T>
	void lookup(string name, T &fn, bool msg=true) {
		fn = reinterpret_cast<T>(dlsym(handle, name.c_str()));
		if (msg and fn == nullptr) {
			printf("Unable to locate symbol \"%s\".\n", name.c_str());
		}
	}

	void resolve() {
		lookup("Py_BuildValue", BuildValue);
		lookup("PyTuple_Size", Tuple_Size);
		lookup("PyTuple_GetItem", Tuple_GetItem);
		lookup("PyLong_FromLong", Long_FromLong);
		lookup("PyLong_AsLong", Long_AsLong);
		lookup("PyFloat_FromDouble", Float_FromDouble);
		lookup("PyArg_ParseTuple", Arg_ParseTuple);
		lookup("PyArg_ParseTupleAndKeywords", Arg_ParseTupleAndKeywords);
		lookup("PyList_Size", List_Size);
		lookup("PyList_GetItem", List_GetItem);
		lookup("PyErr_SetString", Err_SetString);
		lookup("PyModule_Create", _Module_Create, false);
		lookup("PyModule_Create2", _Module_Create2, false);
		lookup("PyImport_AppendInittab", Import_AppendInittab);
		lookup("Py_Initialize", Initialize);
		lookup("Py_Finalize", Finalize);
		lookup("PyRun_SimpleFile", Run_SimpleFile);
		lookup("PyRun_SimpleString", Run_SimpleString);
		lookup("Py_FinalizeEx", FinalizeEx);
		lookup("PyConfig_InitPythonConfig", Config_InitPythonConfig);
		lookup("PyConfig_SetBytesArgv", Config_SetBytesArgv);
		lookup("PyConfig_Clear", Config_Clear);
		lookup("Py_InitializeFromConfig", InitializeFromConfig);
		lookup("PyStatus_Exception", Status_Exception);
		lookup("PyStatus_IsExit", Status_IsExit);
		lookup("Py_ExitStatusException", ExitStatusException);
		lookup("PyExc_TypeError", Exc_TypeError);
		lookup("_Py_NoneStruct", None);
		lookup("PyList_Type", List_Type);
		lookup("PyTuple_Type", Tuple_Type);
	}

	void* lookup(const std::string &symb) {
		return dlsym(handle, symb.c_str());
	}

	void close() {
		if (handle) {
			dlclose(handle);
			handle = nullptr;
		}
	}
};

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

struct StringArgs {
	char **data;

	StringArgs(vector<string> values) {
		data = new char*[values.size()+1];
		for (int i = 0; i < (int)values.size(); i++) {	
			data[i] = strdup(values[i].c_str());
		}
		data[values.size()] = nullptr;
	}

	~StringArgs() {
		for (int i = 0; data[i] != nullptr; i++) {
			delete data[i];
			data[i] = nullptr;
		}
		delete [] data;
	}

	operator char**() {
		return data;
	}
};

namespace phy {

Tech *tech;

static PyObject* tupleFromLevel(Level level) {
	PythonLoader &Py = PythonLoader::inst();
	return Py.BuildValue("(ii)", level.type, level.idx);
}

static Level levelFromTuple(PyObject *obj) {
	PythonLoader &Py = PythonLoader::inst();
	if (obj == nullptr) {
		return Level();
	} else if (Py.Tuple_Size(obj) != 2) {
		Py.Err_SetString(Py.Exc_TypeError, "Level must have 2 elements (type and idx)");
		return Level();
	}

	PyObject *p0 = Py.Tuple_GetItem(obj, 0);
	PyObject *p1 = Py.Tuple_GetItem(obj, 1);
	if (!PyLong_Check(p0)) {
			Py.Err_SetString(Py.Exc_TypeError, "Level type must be an integer.");
			return Level();
	}
	if (!PyLong_Check(p1)) {
			Py.Err_SetString(Py.Exc_TypeError, "Level idx must be an integer.");
			return Level();
	}

	return Level(Py.Long_AsLong(p0), Py.Long_AsLong(p1));
}

static PyObject* py_dbunit(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	double dbunit = -1;
	if(!Py.Arg_ParseTuple(args, "d:dbunit", &dbunit)) {
		return NULL;
	}

	tech->dbunit = dbunit;
	return Py.Float_FromDouble(dbunit);
}

static PyObject* py_scale(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	double scale = -1;
	if(!Py.Arg_ParseTuple(args, "d:scale", &scale)) {
		return NULL;
	}

	tech->scale = scale;
	return Py.Float_FromDouble(scale);
}


static PyObject* py_paint(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	const char *name = 0;
	int major = -1;
	int minor = -1;
	if(!Py.Arg_ParseTuple(args, "sii:paint", &name, &major, &minor)) {
		return NULL;
	}

	int result = (int)tech->paint.size();
	tech->paint.push_back(Paint(name, major, minor));
	return Py.Long_FromLong(result);
}

static PyObject* py_width(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	int layer = -1;
	int width = -1;
	if(!Py.Arg_ParseTuple(args, "ii:width", &layer, &width)) {
		return NULL;
	}

	tech->setWidth(layer, width);
	return Py.Long_FromLong(layer);
}

static PyObject* py_fill(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	int layer = -1;
	if(!Py.Arg_ParseTuple(args, "i:fill", &layer)) {
		return NULL;
	}

	tech->paint[layer].fill = true;
	return Py.Long_FromLong(layer);
}

static PyObject* py_nmos(PyObject *self, PyObject *args, PyObject *kwargs) {
	PythonLoader &Py = PythonLoader::inst();
	const char *variant = 0;
	const char *name = 0;

	PyObject *diff = nullptr;
	
	PyObject *l2 = nullptr;
	PyObject *pItem;
	PyObject *pSub;
	Py_ssize_t n;

	if(!Py.Arg_ParseTupleAndKeywords(args, kwargs, "ssO!|O!:nmos", StringArgs({"variant", "name", "diff", "bins"}), &variant, &name, Py.Tuple_Type, &diff, Py.List_Type, &l2)) {
		return NULL;
	}

	vector<pair<int, int> > bins;
	if (l2 != nullptr) {
		n = Py.List_Size(l2);
		for (int i = 0; i < n; i++) {
			pItem = Py.List_GetItem(l2, i);
			if(!PyTuple_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			if (Py.Tuple_Size(pItem) != 2) {
				Py.Err_SetString(Py.Exc_TypeError, "bins must have 2 elements (min and max)");
				return NULL;
			}

			int lo = 0;
			int hi = std::numeric_limits<int>::max();
			pSub = Py.Tuple_GetItem(pItem, 0);
			if(!PyLong_Check(pSub)) {
					Py.Err_SetString(Py.Exc_TypeError, "min value must be an integer.");
					return NULL;
			}
			lo = Py.Long_AsLong(pSub);

			pSub = Py.Tuple_GetItem(pItem, 1);
			if(!PyLong_Check(pSub)) {
					Py.Err_SetString(Py.Exc_TypeError, "max value must be an integer.");
					return NULL;
			}
			hi = Py.Long_AsLong(pSub);
			
			bins.push_back(pair<int, int>(lo, hi));
		}
	}

	tech->models.push_back(Model(Model::NMOS, variant, name, levelFromTuple(diff), bins));
	Py_INCREF(Py.None);
	return Py.None;
}

static PyObject* py_pmos(PyObject *self, PyObject *args, PyObject *kwargs) {
	PythonLoader &Py = PythonLoader::inst();
	const char *variant = 0;
	const char *name = 0;
	
	PyObject *diff = nullptr;

	PyObject *l2 = nullptr;
	PyObject *pItem;
	PyObject *pSub;
	Py_ssize_t n;

	if(!Py.Arg_ParseTupleAndKeywords(args, kwargs, "ssO!|O!:pmos", StringArgs({"variant", "name", "diff", "bins"}), &variant, &name, Py.Tuple_Type, &diff, Py.List_Type, &l2)) {
		return NULL;
	}

	vector<pair<int, int> > bins;
	if (l2 != nullptr) {
		n = Py.List_Size(l2);
		for (int i = 0; i < n; i++) {
			pItem = Py.List_GetItem(l2, i);
			if(!PyTuple_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			if (Py.Tuple_Size(pItem) != 2) {
				Py.Err_SetString(Py.Exc_TypeError, "bins must have 2 elements (min and max)");
				return NULL;
			}

			int lo = 0;
			int hi = std::numeric_limits<int>::max();
			pSub = Py.Tuple_GetItem(pItem, 0);
			if(!PyLong_Check(pSub)) {
					Py.Err_SetString(Py.Exc_TypeError, "min value must be an integer.");
					return NULL;
			}
			lo = Py.Long_AsLong(pSub);

			pSub = Py.Tuple_GetItem(pItem, 1);
			if(!PyLong_Check(pSub)) {
					Py.Err_SetString(Py.Exc_TypeError, "max value must be an integer.");
					return NULL;
			}
			hi = Py.Long_AsLong(pSub);
			
			bins.push_back(pair<int, int>(lo, hi));
		}
	}

	tech->models.push_back(Model(Model::PMOS, variant, name, levelFromTuple(diff), bins));
	Py_INCREF(Py.None);
	return Py.None;
}

static PyObject* py_dielec(PyObject *self, PyObject *args, PyObject *kwargs) {
	PythonLoader &Py = PythonLoader::inst();
	PyObject *down = nullptr;
	PyObject *up = nullptr;
	float thickness = 0.0f;
	float permitivity = 0.0f;

	if(!Py.Arg_ParseTupleAndKeywords(args, kwargs, "O!O!f|f:dielec", StringArgs({"down", "up", "thick", "permit"}), Py.Tuple_Type, &down, Py.Tuple_Type, &up, &thickness, &permitivity)) {
		return NULL;
	}

	tech->dielec.push_back(Dielectric(levelFromTuple(down), levelFromTuple(up), thickness, permitivity));
	Py_INCREF(Py.None);
	return Py.None;
}

static PyObject* py_subst(PyObject *self, PyObject *args, PyObject *kwargs) {
	PythonLoader &Py = PythonLoader::inst();
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

	if(!Py.Arg_ParseTupleAndKeywords(args, kwargs, "i|iiO!O!O!ff:subst", StringArgs({"draw", "label", "pin", "mask", "excl", "well", "thick", "resist"}), &draw, &label, &pin, Py.List_Type, &l0, Py.List_Type, &l1, Py.Tuple_Type, &well, &thickness, &resistivity)) {
		return NULL;
	}

	vector<int> mask;
	if (l0 != nullptr) {
		n = Py.List_Size(l0);
		for (int i = 0; i < n; i++) {
			pItem = Py.List_GetItem(l0, i);
			if(!PyLong_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			mask.push_back(Py.Long_AsLong(pItem));
		}
	}

	vector<int> excl;
	if (l1 != nullptr) {
		n = Py.List_Size(l1);
		for (int i = 0; i < n; i++) {
			pItem = Py.List_GetItem(l1, i);
			if(!PyLong_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			excl.push_back(Py.Long_AsLong(pItem));
		}
	}

	int result = (int)tech->subst.size();
	tech->subst.push_back(Substrate(draw, label, pin, levelFromTuple(well), thickness, resistivity));
	tech->subst.back().mask = mask;
	tech->subst.back().excl = excl;
	return tupleFromLevel(Level(Level::SUBST, result));
}

static PyObject* py_well(PyObject *self, PyObject *args, PyObject *kwargs) {
	PythonLoader &Py = PythonLoader::inst();
	int draw = -1;
	int label = -1;
	int pin = -1;
	float thickness = 0.0f;
	float resistivity = 0.0f;

	PyObject *l0 = nullptr;
	PyObject *l1 = nullptr;
	PyObject *pItem;
	Py_ssize_t n;

	if(!Py.Arg_ParseTupleAndKeywords(args, kwargs, "i|iiO!O!ff:well", StringArgs({"draw", "label", "pin", "mask", "excl", "thick", "resist"}), &draw, &label, &pin, Py.List_Type, &l0, Py.List_Type, &l1, &thickness, &resistivity)) {
		return NULL;
	}

	vector<int> mask;
	if (l0 != nullptr) {
		n = Py.List_Size(l0);
		for (int i = 0; i < n; i++) {
			pItem = Py.List_GetItem(l0, i);
			if(!PyLong_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			mask.push_back(Py.Long_AsLong(pItem));
		}
	}

	vector<int> excl;
	if (l1 != nullptr) {
		n = Py.List_Size(l1);
		for (int i = 0; i < n; i++) {
			pItem = Py.List_GetItem(l1, i);
			if(!PyLong_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			excl.push_back(Py.Long_AsLong(pItem));
		}
	}

	int result = (int)tech->subst.size();
	tech->subst.push_back(Substrate(draw, label, pin, Level(), thickness, resistivity));
	tech->subst.back().mask = mask;
	tech->subst.back().excl = excl;
	return tupleFromLevel(Level(Level::SUBST, result));
}

static PyObject* py_via(PyObject *self, PyObject *args, PyObject *kwargs) {
	PythonLoader &Py = PythonLoader::inst();
	PyObject *dn = nullptr;
	PyObject *up = nullptr;
	int draw = -1;
	int label = -1;
	int pin = -1;
	float thickness = 0.0f;
	float resistivity = 0.0f;

	if(!Py.Arg_ParseTupleAndKeywords(args, kwargs, "O!O!i|iiff:via", StringArgs({"down", "up", "draw", "label", "pin", "thick", "resist"}), Py.Tuple_Type, &dn, Py.Tuple_Type, &up, &draw, &label, &pin, &thickness, &resistivity)) {
		return NULL;
	}

	int result = (int)tech->vias.size();
	tech->vias.push_back(Via(levelFromTuple(dn), levelFromTuple(up), draw, label, pin, thickness, resistivity));
	return tupleFromLevel(Level(Level::VIA, result));
}

static PyObject* py_route(PyObject *self, PyObject *args, PyObject *kwargs) {
	PythonLoader &Py = PythonLoader::inst();
	int draw = -1;
	int label = -1;
	int pin = -1;
	float thickness = 0.0f;
	float resistivity = 0.0f;

	if(!Py.Arg_ParseTupleAndKeywords(args, kwargs, "i|iiff:route", StringArgs({"draw", "label", "pin", "thick", "resist"}), &draw, &label, &pin, &thickness, &resistivity)) {
		return NULL;
	}

	int result = (int)tech->wires.size();
	tech->wires.push_back(Routing(draw, label, pin, thickness, resistivity));
	return tupleFromLevel(Level(Level::ROUTE, result));
}

static PyObject* py_spacing(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	int l0 = -1;
	int l1 = -1;
	int value = -1;
	if(!Py.Arg_ParseTuple(args, "iii:spacing", &l0, &l1, &value)) {
		return NULL;
	}

	int result = tech->setSpacing(l0, l1, value);
	return Py.Long_FromLong(result);
}

static PyObject* py_enclosing(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	int l0 = -1;
	int l1 = -1;
	int lo = -1;
	int hi = -1;
	if(!Py.Arg_ParseTuple(args, "iii|i:enclosing", &l0, &l1, &lo, &hi)) {
		return NULL;
	}

	if (l0 < 0) {
		Py.Err_SetString(Py.Exc_TypeError, "enclosing layer must be a paint layer.");
		return NULL;
	}

	int result = tech->setEnclosing(l0, l1, lo, hi);

	return Py.Long_FromLong(result);
}

static PyObject* py_b_and(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	vector<int> l;
	Py_ssize_t n;
	PyObject *pItem;
	if (args != nullptr) {
		n = Py.Tuple_Size(args);
		for (int i = 0; i < n; i++) {
			pItem = Py.Tuple_GetItem(args, i);
			if (not PyLong_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			l.push_back(Py.Long_AsLong(pItem));
		}
	}
	if ((int)l.size() < 2) {
		Py.Err_SetString(Py.Exc_TypeError, "b_and requires at least two layers.");
		return NULL;
	}

	int result = tech->setAnd(l);
	return Py.Long_FromLong(result);
}

static PyObject* py_b_or(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	vector<int> l;
	Py_ssize_t n;
	PyObject *pItem;
	if (args != nullptr) {
		n = Py.Tuple_Size(args);
		for (int i = 0; i < n; i++) {
			pItem = Py.Tuple_GetItem(args, i);
			if (not PyLong_Check(pItem)) {
					Py.Err_SetString(Py.Exc_TypeError, "list items must be integers.");
					return NULL;
			}
			l.push_back(Py.Long_AsLong(pItem));
		}
	}
	if ((int)l.size() < 2) {
		Py.Err_SetString(Py.Exc_TypeError, "b_or requires at least two layers.");
		return NULL;
	}

	int result = tech->setOr(l);
	return Py.Long_FromLong(result);
}

static PyObject* py_b_not(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	int l0 = -1;
	if(!Py.Arg_ParseTuple(args, "i:b_not", &l0)) {
		return NULL;
	}

	int result = tech->setNot(l0);
	return Py.Long_FromLong(result);
}

static PyObject* py_bound(PyObject *self, PyObject *args) {
	PythonLoader &Py = PythonLoader::inst();
	int l0 = -1;
	if(!Py.Arg_ParseTuple(args, "i:bound", &l0)) {
		return NULL;
	}

	tech->boundary = l0;
	return Py.Long_FromLong(l0);
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
	PythonLoader &Py = PythonLoader::inst();
	return Py.Module_Create(&EmbModule, PYTHON_API_VERSION);
}

bool loadTech(Tech &dst, string path, string cells) {
	PythonLoader &Py = PythonLoader::inst();
	if (not Py) {
		return false;
	}

	tech = &dst;

	vector<string> args = splitArguments(path);
	if (not filesystem::exists(args[0])) {
		printf("technology file '%s' not found.\n", args[0].c_str());
		return false;
	}

	dst.path = args[0];
	dst.lib = cells;

	PyConfig config;
	Py.Config_InitPythonConfig(&config);

	vector<char*> argv;
	argv.push_back(const_cast<char*>(args[0].c_str()));
	for (auto arg = args.begin(); arg != args.end(); arg++) {
		argv.push_back(const_cast<char*>(arg->c_str()));
	}

	PyStatus status = Py.Config_SetBytesArgv(&config, argv.size(), argv.data());
	if (Py.Status_Exception(status)) {
		tech = nullptr;
		Py.Config_Clear(&config);
		if (not Py.Status_IsExit(status)) {
			Py.ExitStatusException(status);
		}
		printf("config argv failed.\n");
		return false;
	}

	Py.Import_AppendInittab("loom", &PyInit_loom);
	status = Py.InitializeFromConfig(&config);
	if (Py.Status_Exception(status)) {
		tech = nullptr;
		Py.Config_Clear(&config);
		if (not Py.Status_IsExit(status)) {
			Py.ExitStatusException(status);
		}
		printf("initialize failed.\n");
		return false;
	}
	Py.Config_Clear(&config);

	bool success = true;
#if defined(_WIN32) || defined(_WIN64)
	FILE *fptr = fopen(argv[0], "rb");
#else
	FILE *fptr = fopen(argv[0], "r");
#endif
	if (fptr == nullptr) {
		printf("unable to open file.\n");
		return false;
	}

	Py.Run_SimpleFile(fptr, argv[0]);
	fclose(fptr);
	success = (Py.FinalizeEx() >= 0);
	if (not success) {
		printf("run failed.\n");
	}

	tech = nullptr;
	return success;
}


}

