#pragma once

#include <vector>

#include "Layout.h"

using namespace std;

namespace phy {

struct Library {
	Library();
	Library(const Tech *tech);
	~Library();

	const Tech *tech;

	vector<Layout> cells; 
};

}
