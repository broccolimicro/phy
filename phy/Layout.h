#pragma once

#include <vector>
#include <array>

#include "vector.h"
#include "Tech.h"

using namespace std;

namespace phy {

struct Layout;

struct Rect {
	Rect();
	Rect(int net, vec2i ll, vec2i ur);
	~Rect();

	// The index of the variable this rectangle is connected to or -1 if floating.
	// index into Layout::nets
	int net;
	// locations in db units of lower left corner and upper right corner
	// respectively
	vec2i ll;
	vec2i ur;

	vec2i operator[](int corner) const;
	vec2i &operator[](int corner);

	Rect shift(vec2i pos, vec2i dir=vec2i(1,1)) const;
	bool merge(Rect r);
	bool overlaps(Rect r) const;
	bool hasLabel() const;
	Rect &bound(vec2i rll, vec2i rur);
	Rect &bound(Rect r);
};

// is this bound compared along the x or y boundary?
struct Bound {
	Bound();
	Bound(int pos, int idx);
	~Bound();
	
	// index into Layer::geo
	int idx;
	
	int pos;
};

bool operator<(const Bound &b0, const Bound &b1);
bool operator<(const Bound &b, int p);

struct Layer {
	Layer(const Tech &tech);
	Layer(const Tech &tech, bool value);
	Layer(const Tech &tech, int draw, int label = -1, int pin = -1);
	~Layer();

	enum {
		UNKNOWN = -1,
	};

	const Tech *tech;

	// this is the source of truth
	// index into layer stack defined in Tech
	int draw;
	int label;
	int pin;
	vector<Rect> geo;

	// flags to help pull apart spacing rules for cell construction
	bool isRouting;
	bool isSubstrate;
	
	/////////////////////////////////////////////
	// these optimize performance in the minOffset computation
	mutable bool dirty;
	
	// indexed as [axis][fromTo]
	mutable array<array<vector<Bound>, 2>, 2> bound;

	////////////////////////////////////////////

	bool isFill() const;

	void clear();
	void sync() const;

	void push(Rect rect, bool doSync=false);
	void push(vector<Rect> rects, bool doSync=false);
	void erase(int index, bool doSync=false);

	Rect bbox() const;
	void merge(bool doSync=false);

	Layer clamp(int axis, int lo, int hi) const;
	Layer &shift(vec2i pos, vec2i dir=vec2i(1,1));

	Layer &fillSpacing();
};

bool operator<(const Layer &l0, const Layer &l1);
bool operator<(const Layer &l, int id);

Layer operator&(const Layer &l0, const Layer &l1);
Layer interact(const Layer &l0, const Layer &l1);
Layer not_interact(const Layer &l0, const Layer &l1);
Layer operator|(const Layer &l0, const Layer &l1);
Layer operator~(const Layer &l);

struct Evaluation {
	Evaluation(const Tech &tech);
	Evaluation(const Layout &layout);
	~Evaluation();

	const Layout *layout;
	// negative index into Tech::rules -> geometry
	Layer empty;
	map<int, Layer> layers;

	// negative index into Tech::rules -> count of ready operands in layers
	map<int, int> incomplete;

	void init();
	bool has(int idx);
	const Layer &at(int idx) const;
	Layer &set(int idx);
	void evaluate();
};

struct Port {
	Port();
	Port(string name, bool isInput=false, bool isOutput=false, bool isVdd=false, bool isGND=false, bool isSub=false);
	~Port();

	int label;
	vec2i pos;

	string name;
	bool isInput;
	bool isOutput;
	bool isVdd;
	bool isGND;
	bool isSub;
};

struct Layout {
	// Layout(); we shouldn't be able to create a layout without a pointer to the
	// technology node specification
	Layout(const Tech &tech);
	~Layout();

	// used in the minOffset() functions for substrateMode and routingMode
	enum {
		DEFAULT = 0,
		MERGENET = 1,
		IGNORE = 2,
	};

	// The technology node specification with the DRC rules
	const Tech *tech;

	// The name of the cell in the cell library
	string name;
	// The bounding box of the cell
	Rect box;

	// The names for all of the nets
	vector<Port> nets;

	// The geometry for this cell
	vector<Layer> layers;
	
	
	vector<Layer>::const_iterator find(int draw, int layer=-1, int pin=-1) const;
	vector<Layer>::iterator find(int draw, int layer=-1, int pin=-1);
	vector<Layer>::iterator at(int draw, int layer=-1, int pin=-1);
	void push(int layer, Rect rect, bool doSync=false);
	void push(int layer, vector<Rect> rects, bool doSync=false);
	void push(const Material &mat, Rect rect, bool doSync=false);
	void push(const Material &mat, vector<Rect> rects, bool doSync=false);

	Rect bbox() const;
	void merge(bool doSync=false);

	void clear();
};

bool minOffset(int *offset, int axis, const Layer &l0, int l0Shift, const Layer &l1, int l1Shift, vec2i spacing=vec2i(0,0), bool mergeNet=true);
bool minOffset(int *offset, int axis, const Layout &left, int leftShift, const Layout &right, int rightShift, int substrateMode=Layout::DEFAULT, int routingMode=Layout::DEFAULT, bool horizSpacing=true);

}

