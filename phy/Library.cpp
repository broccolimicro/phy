#include "Library.h"

using namespace std;

namespace phy {

Library::Library(const Tech &tech, string libPath) : tech(tech) {
	this->libPath = libPath;
}

Library::Library(const Library &copy) : tech(copy.tech) {
	libPath = copy.libPath;
	macros = copy.macros;
}

Library::~Library() {
}

Library &Library::operator=(const Library &copy) {
	macros = copy.macros;
	return *this;
}

}

