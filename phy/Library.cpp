#include "Library.h"

using namespace std;

namespace phy {

Library::Library(const Tech &tech, string libPath) {
	this->libPath = libPath;
	this->tech = &tech;
}

Library::~Library() {
}

}

