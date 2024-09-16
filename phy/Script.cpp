#include "Script.h"

#include <dirent.h>
#include <fnmatch.h>
#include <pwd.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

std::string expandTilde(const std::string& input) {
	if (input[0] == '~') {
		std::string homeDir;

		if (input.length() == 1 || input[1] == '/') {
			// If the tilde is followed by a slash or it's just `~`, use the current user's home directory
			const char* home = getenv("HOME");
			if (!home) {
				struct passwd* pw = getpwuid(getuid());
				homeDir = pw->pw_dir;
			} else {
				homeDir = home;
			}
		}

		return homeDir + input.substr(1);
	}
	return input;
}

std::string expandEnvVariables(const std::string& input) {
	std::string output;
	size_t pos = 0;
	bool inEscape = false;

	while (pos < input.size()) {
		if (inEscape) {
			// If we are in escape mode, just append the character literally
			output += input[pos];
			inEscape = false;
			pos++;
			continue;
		}

		if (input[pos] == '\\') {
			// Escape sequence (e.g., \$), skip the backslash and escape the next char
			inEscape = true;
			pos++;
		} else if (input[pos] == '$') {
			size_t endPos = pos + 1;

			if (endPos < input.size() && input[endPos] == '{') {
				// Handle ${VAR} syntax
				endPos++;  // Skip past '{'
				size_t varEnd = input.find('}', endPos);
				if (varEnd != std::string::npos) {
					std::string varName = input.substr(endPos, varEnd - endPos);
					const char* varValue = getenv(varName.c_str());

					if (varValue) {
						output += varValue;
					}
					pos = varEnd + 1;  // Move past '}'
					continue;
				}
			} else {
				// Handle $VAR syntax (without braces)
				while (endPos < input.size() && (isalnum(input[endPos]) || input[endPos] == '_')) {
					endPos++;
				}

				std::string varName = input.substr(pos + 1, endPos - pos - 1);
				const char* varValue = getenv(varName.c_str());

				if (varValue) {
					output += varValue;
				}

				pos = endPos;
				continue;
			}
		}

		// Append literal characters
		output += input[pos];
		pos++;
	}

	return output;
}

std::vector<std::string> globFiles(const std::string& pattern) {
	std::vector<std::string> matches;
	std::string dirPath = ".";

	size_t lastSlash = pattern.find_last_of('/');
	if (lastSlash != std::string::npos) {
		dirPath = pattern.substr(0, lastSlash);
	}

	std::string filePattern = pattern.substr(lastSlash + 1);
	DIR* dir = opendir(dirPath.c_str());
	if (dir == nullptr) return matches;

	struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
		if (fnmatch(filePattern.c_str(), entry->d_name, 0) == 0) {
			matches.push_back(dirPath + "/" + entry->d_name);
		}
	}
	closedir(dir);
	return matches;
}

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

std::vector<std::string> wordExpand(const std::string& input) {
	std::vector<std::string> args = splitArguments(input);
	std::vector<std::string> expandedArgs;

	for (auto& arg : args) {
		arg = expandTilde(arg);
		arg = expandEnvVariables(arg);

		if (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos) {
			std::vector<std::string> globbedFiles = globFiles(arg);
			expandedArgs.insert(expandedArgs.end(), globbedFiles.begin(), globbedFiles.end());
		} else {
			expandedArgs.push_back(arg);
		}
	}

	return expandedArgs;
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

	vector<string> args = wordExpand(path);
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

