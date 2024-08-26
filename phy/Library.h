#pragma once

#include <vector>

#include "Layout.h"

using namespace std;

namespace phy {

struct Library {
	Library();
	~Library();

	vector<Layout> cells; 
};

}
