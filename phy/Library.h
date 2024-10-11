#pragma once

#include <vector>

#include "Layout.h"

using namespace std;

namespace phy {

struct Library {
	Library(const Tech &tech, string libPath="");
	~Library();

	const Tech *tech;
	
	string libPath;

	vector<Layout> macros; 
};

}
