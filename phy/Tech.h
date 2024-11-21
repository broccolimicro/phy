#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>

#include "vector.h"

using namespace std;

int flip(int idx);

namespace phy {

// This structure represents a single GDS layer. There are often different
// types of layers, but for us the most important are the draw, label, and pin
// layer types.
struct Paint {
	Paint();
	Paint(string name, int major = 0, int minor = 0);
	~Paint();

	string name;
	int major;
	int minor;

	// Can we fix min-spacing violations by filling in the space?
	bool fill;

	// negative index into Tech::rules
	vector<int> out;
};

struct Level {
	Level();
	Level(int type, int idx);
	~Level();

	enum {
		INVALID = 0,
		SUBST = 1,
		ROUTE = 2,
		VIA = 3
	};

	int type;
	int idx;

	bool valid() const;
};

bool operator==(Level l0, Level l1);
bool operator!=(Level l0, Level l1);
bool operator<(Level l0, Level l1);
bool operator>(Level l0, Level l1);
bool operator<=(Level l0, Level l1);
bool operator>=(Level l0, Level l1);

// This is a base class that makes it easier for us to draw layers for
// different purposes.
struct Material {
	Material();
	Material(int draw, int label=-1, int pin=-1, float thickness=0.0f, float resistivity=0.0f);
	~Material();
	
	// these index into Tech::paint or Tech::rules
	int draw;
	int label;
	int pin;

	// mask layers that define properties of this specific material
	vector<int> mask;
	vector<int> excl;

	float thickness; // um
	// This is computed by multiplying sheet resistance (mOhms / sq) by thickness
	// (um) to get (mOhms / um) then divide by 1000 to get (Ohms / um)
	// p = Rs*t/1000
	float resistivity; // Ohms / um

	bool contains(int layer) const;

	int size() const;
	int at(int idx) const;

	bool hasDraw() const;
};

// This specifies a diffusion layer for drawing transistors
struct Substrate : Material {
	Substrate();
	Substrate(int draw, int label=-1, int pin=-1, Level well=Level(), float thickness=0.0f, float resistivity=0.0f);
	~Substrate();

	Level well;
};

// This structure records how to draw a transistor
struct Model {
	Model();
	Model(int type, string variant, string name, Level diff, vector<pair<int, int> > bins=vector<pair<int, int> >());
	~Model();

	// Type of transistor (nmos or pmos)
	enum {
		NMOS = 0,
		PMOS = 1,
	};
	int type;
	
	string variant;

	// Name of the device in the PDK, this is used to parse the spice file.
	string name;

	// All of the diffusion layers starting top down
	// index into Tech::subst
	Level diff;
	vector<pair<int, int> > bins;
};

// This represents a routing layer for drawing wires to connect transistor
// terminals
struct Routing : Material {
	Routing();
	Routing(int draw, int label, int pin, float thickness=0.0f, float resistivity=0.0f);
	~Routing();
};

// Connect two layers with a via
struct Via : Material {
	Via();
	Via(Level down, Level up, int draw, int label=-1, int pin=-1, float thickness=0.0f, float resistivity=0.0f);
	~Via();

	Level down;
	Level up;
};

struct Dielectric {
	Dielectric();
	Dielectric(Level down, Level up, float thickness=0.0f, float permitivity=0.0f);
	~Dielectric();

	Level down;
	Level up;
	
	float thickness; // um
	// C = parallel plate capacitance (aF / um^2)
	// d = distance or thickness (um)
	// e = permitivity (aF / um)
	// e = C*d
	float permitivity; // aF / um
};

// This implements a DRC operation on the geometry. There are 3 kinds of DRC
// operations: selectors, operations, and checks. Selectors select a specific
// kind of geometry. Operations manipulate that geometry in various ways.
// Checks verify that geometry against rules that must be enforced.
struct Rule {
	Rule();
	Rule(int type, vector<int> operands=vector<int>(), vector<int> params=vector<int>());
	~Rule();

	enum {
		NOT = 0,
		AND = 1,
		OR = 2,
		INTERACT = 3,
		NOT_INTERACT = 4,
		SPACING = 5,
		ENCLOSING = 6,
		WIDTH = 7,
		// TODO implement remaining DRC checks
		// EDGES, WITH/WITHOUT_AREA, WITH/WITHOUT_LENGTH,
		// ONGRID, GROW, SHRINK
	};

	// The type of DRC rule (See enum above)
	int type;

	// positive operands refer to paint layers (index into Tech::paint)
	// negative operands refer to operation outputs (index into Tech::rules)
	vector<int> operands;

	// These are constant valued parameters to be used in more complex operations
	vector<int> params;

	// negative index into Tech::rules (use flip() to get the index)
	// This is negative to make it consistent with Rule::operands
	vector<int> out;

	bool isOperator() const;
};

// This is the top-level structure for the technology specification. It reads
// in the design rules, transistor models, and GDS configuration to enable
// automated cell layout and design rule checking.
struct Tech {
	Tech();
	~Tech();

	string path;
	string lib;

	// Scale of integer units in micrometers for each rectangle
	// in the Layout.
	double dbunit;
	double scale;

	// index into Tech::paint to represent the cell boundary layer
	int boundary;

	// All of the GDS layers we can use for layout
	vector<Paint> paint;

	// The diffusion and well layers (substrate)
	vector<Substrate> subst;

	// different types of transistors (for example nmos and pmos for svt, hvt, and lvt)
	vector<Model> models;

	// The routing layers starting with poly at index 0, then li (local
	// interconnect) at index 1, then the metal layers (m1, m2, m3, ...)
	vector<Routing> wires;

	// Vias at different layers. Each via has a downLevel and upLevel to dictate
	// which model or wire they connect. If the downLevel or upLevel is negative,
	// then its a negative index into Tech::models connecting to the diffusion
	// layers. If downLevel or upLevel are positive, then they are a positive
	// index into Tech::wires connecting to that routing layer.
	vector<Via> vias;

	// The dielectric layers between routing layers, substrate layers, etc
	vector<Dielectric> dielec;

	// the list of all DRC rules that we need to check. See Rule for more
	// information.
	vector<Rule> rules;

	// layer - paint layers or operations on them
	// layer < 0 refers to "rules"
	// layer >= 0 refers to "paint"
	int findRule(int type, vector<int> operands) const;
	int setRule(int type, vector<int> operands);
	int getOr(vector<int> layers) const;
	int setOr(vector<int> layers);
	int getAnd(vector<int> layers) const;
	int setAnd(vector<int> layers);
	int getNot(int l) const;
	int setNot(int l);
	int getInteract(int l0, int l1) const;
	int setInteract(int l0, int l1);
	int getNotInteract(int l0, int l1) const;
	int setNotInteract(int l0, int l1);
	int getSpacing(int l0, int l1) const;
	int setSpacing(int l0, int l1, int value);
	vec2i getEnclosing(int l0, int l1) const;
	int setEnclosing(int l0, int l1, int lo, int hi);
	int getWidth(int l0) const;
	int setWidth(int l0, int value);

	string print(int layer) const;
	int findPaint(string name) const;
	int findPaint(int major, int minor) const;
	int findModel(string name) const;
	int findModel(int type, string variant) const;
	const Material *findMaterial(int layer) const;

	bool isRouting(int layer) const;
	bool isSubstrate(int layer) const;
	bool isPin(int layer) const;
	bool isLabel(int layer) const;
	bool isWell(int layer) const;

	// level - physical levels on the chip
	const Material &at(Level level) const;
	vector<int> via(Level down, Level up) const;
};

}
