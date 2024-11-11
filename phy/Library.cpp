#include "Library.h"

using namespace std;

namespace phy {

Library::Library(const Tech &tech) {
	this->tech = &tech;
}

Library::~Library() {
}

}

