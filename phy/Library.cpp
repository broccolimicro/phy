#include "Library.h"

using namespace std;

namespace phy {

Library::Library() {
	tech = nullptr;
}

Library::Library(const Tech *tech) {
	this->tech = tech;
}

Library::~Library() {
}

}

