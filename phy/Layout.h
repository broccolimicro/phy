#pragma once

#include <vector>
#include <array>

#include "vector.h"
#include "Tech.h"

using namespace std;

namespace phy {

struct Layout;
struct Rect;
struct Poly;

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

	void normalize();
	Rect &shift_inplace(vec2i pos, vec2i dir=vec2i(1,1));
	Rect shift(vec2i pos, vec2i dir=vec2i(1,1)) const;
	bool merge(Rect r);
	bool overlaps(Rect r) const;
	bool overlaps(vec2i v0, vec2i v1, bool withEdge=true) const;
	bool contains(vec2i p, bool withEdge=true) const;
	Rect &bound(vec2i p);
	Rect &bound(vec2i rll, vec2i rur);
	Rect &bound(Rect r);
	Rect &bound(Poly gon);

	void grow(vec2i d);
	bool shrink(vec2i d);

	vec2i center() const;
	int area() const;
};

Rect operator&(const Rect &r0, const Rect &r1);

struct Poly {
	Poly();
	Poly(int net, vector<vec2i> v);
	Poly(Rect r);
	~Poly();
	
	int net;
	vector<vec2i> v;

	int orientation(vec2i p, vec2i q, vec2i r) const;
	bool intersect(vec2i a0, vec2i a1, vec2i b0, vec2i b1) const;

	bool overlaps(Poly p) const;
	bool overlaps(Rect r) const;
	bool contains(vec2i p) const;

	int area() const;
	int perim() const;

	//bool b_or(Rect r);
	//bool b_or(Poly p);
	//bool b_diff(Rect r);
	//bool b_diff(Poly p);
	//bool b_and(Rect r);
	//bool b_and(Poly p);
	
	vector<Rect> split();
	bool empty(); 
	void normalize();

	Poly &shift_inplace(vec2i pos, vec2i dir=vec2i(1,1));

	void grow(vec2i d);
	bool shrink(vec2i d);
};

struct Label {
	Label();
	Label(int net, vec2i pos, string txt);
	~Label();

	int net;
	vec2i pos;
	string txt;

	Label &shift_inplace(vec2i pos, vec2i dir=vec2i(1,1));
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
	Layer(const Tech &tech, int draw);
	~Layer();

	enum {
		UNKNOWN = -1,
	};

	const Tech *tech;

	// this is the source of truth
	// index into layer stack defined in Tech
	int draw;

	vector<Rect> geo;
	vector<Poly> poly;
	vector<Label> lbl;
	Rect box;

	// flags to help pull apart spacing rules for cell construction
	bool isRouting;
	bool isSubstrate;
	bool isPin;
	bool isWell;

	
	/////////////////////////////////////////////
	// these optimize performance in the minOffset computation
	mutable bool dirty;
	
	// indexed as [axis][fromTo]
	mutable array<array<vector<Bound>, 2>, 2> bound;

	////////////////////////////////////////////

	bool isFill() const;

	bool empty() const;
	void clear();
	void sync() const;

	void push(Rect rect);
	void push(vector<Rect> rects);
	void push(Poly gon);
	void push(vector<Poly> gons);

	void erase(int index);

	void label(Label lbl);
	void label(vector<Label> lbls);

	void normalize();
	Layer &merge();

	Layer clamp(int axis, int lo, int hi) const;
	Layer &shift_inplace(vec2i pos, vec2i dir=vec2i(1,1));

	Layer &fillSpacing();

	vector<vector<int> > trace();
	vector<Layer> split(vector<vector<int> > clusters=vector<vector<int> >());
	int area(vector<int> cluster=vector<int>());

	bool overlaps(const Rect &r0) const;
	bool overlaps(const Layer &l0) const;
	
	void print();
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

struct Net {
	Net();
	Net(string name);
	~Net();

	vector<string> names;

	// These are only necessary for LEF export
	bool isVdd;
	bool isGND;
	bool isInput;
	bool isOutput;
	bool isSub;

	void set(string name);
	bool has(string name) const;
};

struct Instance {
	Instance(int macro=-1, vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1));
	~Instance();

	int macro;
	vector<int> ports;
	
	vec2i pos;
	vec2i dir;
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
	vector<Net> nets;

	// The geometry for this cell
	map<int, Layer> layers;

	vector<Instance> inst;
	
	map<int, Layer>::const_iterator find(int draw) const;
	map<int, Layer>::iterator find(int draw);
	map<int, Layer>::iterator at(int draw);

	Layer get(Level level);

	void push(int layer, Rect rect);
	void push(int layer, vector<Rect> rects);
	vec2i push(Level level, Rect rect, int base=-1, int axis=0);
	vec2i push(Level level, vector<Rect> rects, int base=-1, int axis=0);

	void push(int layer, Poly gon);
	void push(int layer, vector<Poly> gons);
	vec2i push(Level level, Poly gon, int base=-1, int axis=0);
	vec2i push(Level level, vector<Poly> gons, int base=-1, int axis=0);

	void label(int layer, Label lbl);
	void label(int layer, vector<Label> lbls);
	void label(Level level, Label lbl);
	void label(Level level, vector<Label> lbls);

	int netAt(string name);

	void normalize();
	void merge();

	Layout &shift_inplace(vec2i pos, vec2i dir=vec2i(1,1));

	// trace the geometry to understand the structure of the nets
	void trace();

	bool empty() const;
	void clear();

	void print();
};

bool minOffset(int *offset, int axis, const Layer &l0, int l0Shift, const Layer &l1, int l1Shift, vec2i spacing=vec2i(0,0), bool mergeNet=true);
bool minOffset(int *offset, int axis, const Layout &left, int leftShift, const Layout &right, int rightShift, int substrateMode=Layout::DEFAULT, int routingMode=Layout::DEFAULT, bool horizSpacing=true);

}

