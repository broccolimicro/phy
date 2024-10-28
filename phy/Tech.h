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

	// minimum width/height of layer
	int minWidth;

	// Can we fix min-spacing violations by filling in the space?
	bool fill;

	// negative index into Tech::rules
	vector<int> out;
};

// This is a base class that makes it easier for us to draw layers for
// different purposes.
struct Material {
	Material();
	Material(int draw, int label=-1, int pin=-1);
	~Material();
	
	// these index into Tech::paint
	int draw;
	int label;
	int pin;

	bool contains(int layer) const;
};

// This specifies a diffusion layer for drawing transistors
struct Diffusion : Material {
	Diffusion();
	Diffusion(int draw, int label=-1, int pin=-1, bool isWell=false);
	~Diffusion();

	bool isWell;
};

// This structure records how to draw a transistor
struct Model {
	Model();
	Model(int type, string variant, string name, vector<int> stack);
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
	vector<int> stack;
};

// This represents a routing layer for drawing wires to connect transistor
// terminals
struct Routing : Material {
	Routing();
	Routing(int draw, int label, int pin);
	~Routing();
};

// Connect two layers with a via
struct Via : Material {
	Via();
	Via(int downLevel, int upLevel, int draw, int label=-1, int pin=-1);
	~Via();

	// index into Tech::wires when >= 0
	// index into Tech::models when < 0
	// use flip() to access the index when negative.
	int downLevel;
	int upLevel;
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
		ENCLOSING = 6
		// TODO implement remaining DRC checks
		// EDGES, WITH/WITHOUT_AREA, WITH/WITHOUT_LENGTH,
		// ENCLOSING, ONGRID, WIDTH, GROW, SHRINK
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

	// Scale of integer units in micrometers for each rectangle
	// in the Layout.
	double dbunit;
	double scale;

	// index into Tech::paint to represent the cell boundary layer
	int boundary;

	// All of the GDS layers we can use for layout
	vector<Paint> paint;

	// different types of transistors (for example nmos and pmos for svt, hvt, and lvt)
	vector<Model> models;

	// Vias at different layers. Each via has a downLevel and upLevel to dictate
	// which model or wire they connect. If the downLevel or upLevel is negative,
	// then its a negative index into Tech::models connecting to the diffusion
	// layers. If downLevel or upLevel are positive, then they are a positive
	// index into Tech::wires connecting to that routing layer.
	vector<Via> vias;

	// The routing layers starting with poly at index 0, then li (local
	// interconnect) at index 1, then the metal layers (m1, m2, m3, ...)
	vector<Routing> wires;

	// The diffusion and well layers (substrate)
	vector<Diffusion> subst;

	// the list of all DRC rules that we need to check. See Rule for more
	// information.
	vector<Rule> rules;

	int getOr(int l0, int l1) const;
	int setOr(int l0, int l1);
	int getAnd(int l0, int l1) const;
	int setAnd(int l0, int l1);
	int getNot(int l) const;
	int setNot(int l);
	int getInteract(int l0, int l1) const;
	int setInteract(int l0, int l1);
	int getNotInteract(int l0, int l1) const;
	int setNotInteract(int l0, int l1);

	int ruleIdx(int type, int l0, int l1) const;
	int getSpacing(int l0, int l1) const;
	int setSpacing(int l0, int l1, int value);
	vec2i getEnclosing(int l0, int l1) const;
	int setEnclosing(int l0, int l1, int lo, int hi);

	string print(int layer) const;
	int findPaint(string name) const;
	int findPaint(int major, int minor) const;
	int findModel(string name) const;
	const Material *atMaterial(int level) const;
	const Material *findMaterial(int layer) const;
	vector<int> findVias(int downLevel, int upLevel) const;

	bool isRouting(int layer) const;
	bool isSubstrate(int layer) const;
	bool isPin(int layer) const;
	bool isLabel(int layer) const;
	bool isWell(int layer) const;
};

}
