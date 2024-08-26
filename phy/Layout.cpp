#include "Layout.h"
#include <algorithm>
#include <limits>

using namespace std;

namespace phy {

Rect::Rect() {
	net = -1;
	ll = vec2i(0,0);
	ur = vec2i(0,0);
}

Rect::Rect(int net, vec2i ll, vec2i ur) {
	this->net = net;
	this->ll = ll;
	this->ur = ur;

	if (this->ur[1] < this->ll[1]) {
		int tmp = this->ur[1];
		this->ur[1] = this->ll[1];
		this->ll[1] = tmp;
	}

	if (this->ur[0] < this->ll[0]) {
		int tmp = this->ur[0];
		this->ur[0] = this->ll[0];
		this->ll[0] = tmp;
	}
}

Rect::~Rect() {
}

vec2i Rect::operator[](int corner) const {
	return corner ? ur : ll;
}

vec2i &Rect::operator[](int corner) {
	return corner ? ur : ll;
}

Rect Rect::shift(vec2i pos, vec2i dir) const {
	return Rect(net, pos+ll*dir, pos+ur*dir);
}

bool Rect::merge(Rect r) {
	if (net == r.net and ll[0] == r.ll[0] and ur[0] == r.ur[0] and ll[1] <= r.ur[1] and ur[1] >= r.ll[1]) {
		if (r.ll[1] < ll[1]) {
			ll[1] = r.ll[1];
		}
		if (r.ur[1] > ur[1]) {
			ur[1] = r.ur[1];
		}
		return true;
	} else if (net == r.net and ll[1] == r.ll[1] and ur[1] == r.ur[1] and ll[0] <= r.ur[0] and ur[0] >= r.ll[0]) {
		if (r.ll[0] < ll[0]) {
			ll[0] = r.ll[0];
		}
		if (r.ur[0] > ur[0]) {
			ur[0] = r.ur[0];
		}
		return true;
	} else if (net == r.net and ll[1] <= r.ll[1] and ur[1] >= r.ur[1] and ll[0] <= r.ll[0] and ur[0] >= r.ur[0]) {
		return true;
	} else if (net == r.net and ll[1] >= r.ll[1] and ur[1] <= r.ur[1] and ll[0] >= r.ll[0] and ur[0] <= r.ur[0]) {
		ll[0] = r.ll[0];
		ur[0] = r.ur[0];
		ll[1] = r.ll[1];
		ur[1] = r.ur[1];
		return true;
	}
	return false;
}

bool Rect::overlaps(Rect r) const {
	return ll[0] <= r.ur[0] and r.ll[0] <= ur[0] and ll[1] <= r.ur[1] and r.ll[1] <= ur[1];
}

Rect &Rect::bound(vec2i rll, vec2i rur) {
	if (ll[0] == ur[0] and ll[1] == ur[1]) {
		ll = rll;
		ur = rur;
		return *this;
	}

	if (rll[0] < ll[0]) {
		ll[0] = rll[0];
	}

	if (rll[1] < ll[1]) {
		ll[1] = rll[1];
	}

	if (rll[0] > ur[0]) {
		ur[0] = rll[0];
	}

	if (rll[1] > ur[1]) {
		ur[1] = rll[1];
	}

	if (rur[0] < ll[0]) {
		ll[0] = rur[0];
	}

	if (rur[1] < ll[1]) {
		ll[1] = rur[1];
	}

	if (rur[0] > ur[0]) {
		ur[0] = rur[0];
	}

	if (rur[1] > ur[1]) {
		ur[1] = rur[1];
	}

	return *this;
}

Rect &Rect::bound(Rect r) {
	if (ll[0] == ur[0] and ll[1] == ur[1]) {
		ll = r.ll;
		ur = r.ur;
		return *this;
	}

	if (r.ll[0] < ll[0]) {
		ll[0] = r.ll[0];
	}

	if (r.ll[1] < ll[1]) {
		ll[1] = r.ll[1];
	}

	if (r.ll[0] > ur[0]) {
		ur[0] = r.ll[0];
	}

	if (r.ll[1] > ur[1]) {
		ur[1] = r.ll[1];
	}

	if (r.ur[0] < ll[0]) {
		ll[0] = r.ur[0];
	}

	if (r.ur[1] < ll[1]) {
		ll[1] = r.ur[1];
	}

	if (r.ur[0] > ur[0]) {
		ur[0] = r.ur[0];
	}

	if (r.ur[1] > ur[1]) {
		ur[1] = r.ur[1];
	}
	return *this;
}


bool Rect::hasLabel() const {
	return net >= 0;
}

Bound::Bound() {
	idx = -1;
	pos = 0;
}

Bound::Bound(int pos, int idx) {
	this->idx = idx;
	this->pos = pos;
}

Bound::~Bound() {
}

bool operator<(const Bound &b0, const Bound &b1) {
	return (b0.pos < b1.pos);
}

bool operator<(const Bound &b, int p) {
	return (b.pos < p);
}

Layer::Layer() {
	draw = Layer::UNKNOWN;
	label = Layer::UNKNOWN;
	pin = Layer::UNKNOWN;
	dirty = false;
	isRouting = false;
	isSubstrate = false;
}

Layer::Layer(bool value) {
	draw = Layer::UNKNOWN;
	label = Layer::UNKNOWN;
	pin = Layer::UNKNOWN;
	dirty = false;
	isRouting = value;
	isSubstrate = not value;
	if (value) {
		int lo = std::numeric_limits<int>::min();
		int hi = std::numeric_limits<int>::max();
		push(Rect(-1, vec2i(lo, lo), vec2i(hi, hi)));
	}
}

Layer::Layer(const Tech &tech, int draw, int label, int pin) {
	this->draw = draw;
	this->label = label;
	this->pin = pin;
	this->dirty = false;

	this->isRouting = tech.isRouting(draw);
	this->isSubstrate = tech.isSubstrate(draw);
}

Layer::~Layer() {
}

bool Layer::isFill(const Tech &tech) {
	if (draw < 0) {
		return false;
	}

	return tech.paint[draw].fill;
}

void Layer::clear() {
	geo.clear();
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			bound[i][j].clear();
		}
	}
	dirty = false;
}

void Layer::sync() const {
	for (int axis = 0; axis < 2; axis++) {
		for (int fromTo = 0; fromTo < 2; fromTo++) {
			vector<Bound> &bounds = bound[axis][fromTo];

			bounds.clear();
			bounds.reserve(geo.size());
			for (int j = 0; j < (int)geo.size(); j++) {
				bounds.push_back(Bound(geo[j][fromTo][axis], j));
			}
			sort(bounds.begin(), bounds.end());
		}
	}
	dirty = false;
}

void Layer::push(Rect rect, bool doSync) {
	int index = (int)geo.size();
	geo.push_back(rect);
	dirty = dirty or not doSync;
	if (not dirty) {
		for (int axis = 0; axis < 2; axis++) {
			for (int fromTo = 0; fromTo < 2; fromTo++) {
				vector<Bound> &bounds = bound[axis][fromTo];

				auto loc = lower_bound(bounds.begin(), bounds.end(), rect[fromTo][axis]); 
				bounds.insert(loc, Bound(rect[fromTo][axis], index));
			}
		}
	}
}

void Layer::push(vector<Rect> rects, bool doSync) {
	int index = (int)geo.size();
	geo.reserve(geo.size()+rects.size());
	geo.insert(geo.end(), rects.begin(), rects.end());
	dirty = dirty or not doSync;
	if (not dirty) {
		for (int axis = 0; axis < 2; axis++) {
			for (int fromTo = 0; fromTo < 2; fromTo++) {
				vector<Bound> &bounds = bound[axis][fromTo];

				int sz = (int)bounds.size();
				bounds.reserve(bounds.size()+rects.size());
				for (int j = 0; j < (int)rects.size(); j++) {
					bounds.push_back(Bound(rects[j][fromTo][axis], index+j));
				}
				sort(bounds.begin() + sz, bounds.end());
				inplace_merge(bounds.begin(), bounds.begin()+sz, bounds.end());
			}
		}
	}
}

void Layer::erase(int idx, bool doSync) {
	geo.erase(geo.begin()+idx);
	dirty = dirty or not doSync;
	if (not dirty) {
		for (int axis = 0; axis < 2; axis++) {
			for (int fromTo = 0; fromTo < 2; fromTo++) {
				vector<Bound> &bounds = bound[axis][fromTo];

				for (int j = (int)bounds.size()-1; j >= 0; j--) {
					if (bounds[j].idx == idx) {
						bounds.erase(bounds.begin() + j);
					} else if (bounds[j].idx > idx) {
						bounds[j].idx--;
					}
				}
			}
		}
	}
}

Rect Layer::bbox() const {
	if (geo.empty()) {
		return Rect();
	}
	
	// DESIGN(edward.bingham) Naive approach first
	Rect box(-1, geo[0].ll, geo[0].ur);
	for (int i = 1; i < (int)geo.size(); i++) {
		box.bound(geo[i].ll, geo[i].ur);
	}
	return box;
}

void Layer::merge(bool doSync) {
	// TODO use the bounds array to improve performance
	for (int i = (int)geo.size()-1; i >= 0; i--) {
		for (int j = (int)geo.size()-1; j > i; j--) {
			if (geo[i].merge(geo[j])) {
				erase(j, doSync);
			}
		}
	}
}

bool operator<(const Layer &l0, const Layer &l1) {
	return l0.draw < l1.draw;
}

bool operator<(const Layer &l0, int id) {
	return l0.draw < id;
}

// TODO(edward.bingham) These are just the naive implementations of these
// operators. The assumption is that cells are generally fairly small layouts.
// If the DRC engine ends up getting more use, these functions need to be
// highly optimized.
Layer operator&(const Layer &l0, const Layer &l1) {
	//l0.sync();
	//l1.sync();

	Layer result;
	result.isRouting = l0.isRouting and l1.isRouting;
	result.isSubstrate = l0.isSubstrate or l1.isSubstrate;
	for (int i = 0; i < (int)l0.geo.size(); i++) {
		for (int j = 0; j < (int)l1.geo.size(); j++) {
			if (l0.geo[i].overlaps(l1.geo[j])) {
				int net = -1;
				if (l0.geo[i].net < 0 and l1.geo[j].net >= 0) {
					net = l1.geo[j].net;
				} else if (l0.geo[i].net >= 0 and l1.geo[j].net < 0) {
					net = l0.geo[i].net;
				} else if (l0.geo[i].net == l1.geo[j].net) {
					net = l0.geo[i].net;
				}

				result.push(Rect(net, max(l0.geo[i].ll, l1.geo[j].ll), min(l0.geo[i].ur, l1.geo[j].ur)));
			}
		}
	}
	//result.merge();
	return result;
}

Layer interact(const Layer &l0, const Layer &l1) {
	//l0.sync();
	//l1.sync();

	Layer result;
	result.draw = l0.draw;
	result.label = l0.label;
	result.pin = l0.pin;
	result.isRouting = l0.isRouting;
	result.isSubstrate = l0.isSubstrate;
	for (int i = 0; i < (int)l0.geo.size(); i++) {
		for (int j = 0; j < (int)l1.geo.size(); j++) {
			if (l0.geo[i].overlaps(l1.geo[j])) {
				result.push(l0.geo[i]);
				break;
			}
		}
	}
	return result;
}

Layer not_interact(const Layer &l0, const Layer &l1) {
	//l0.sync();
	//l1.sync();

	Layer result;
	result.draw = l0.draw;
	result.label = l0.label;
	result.pin = l0.pin;
	result.isRouting = l0.isRouting;
	result.isSubstrate = l0.isSubstrate;
	for (int i = 0; i < (int)l0.geo.size(); i++) {
		bool found = false;
		for (int j = 0; j < (int)l1.geo.size(); j++) {
			if (l0.geo[i].overlaps(l1.geo[j])) {
				found = true;
				break;
			}
		}
		if (not found) {
			result.push(l0.geo[i]);
		}
	}
	return result;
}

Layer operator|(const Layer &l0, const Layer &l1) {
	//l0.sync();
	//l1.sync();

	Layer result;
	result.isRouting = l0.isRouting and l1.isRouting;
	result.isSubstrate = l0.isSubstrate or l1.isSubstrate;
	result.push(l0.geo);
	result.push(l1.geo);
	result.merge(true);
	return result;
}

Layer operator~(const Layer &l) {
	//l.sync();

	int lo = std::numeric_limits<int>::min();
	int hi = std::numeric_limits<int>::max();

	Layer result(true);
	result.isRouting = not l.isRouting;
	result.isSubstrate = not l.isSubstrate;
	for (int i = 0; i < (int)l.geo.size(); i++) {
		Layer step;
		step.push(Rect(-1, vec2i(l.geo[i].ur[0], lo), vec2i(hi, hi)));
		step.push(Rect(-1, vec2i(lo, lo), vec2i(l.geo[i].ll[0], hi)));
		step.push(Rect(-1, vec2i(l.geo[i].ll[0], lo), vec2i(l.geo[i].ur[0], l.geo[i].ll[1])));
		step.push(Rect(-1, vec2i(l.geo[i].ll[0], l.geo[i].ur[1]), vec2i(l.geo[i].ur[0], hi)));
		result = result & step;
		result.merge();
	}
	return result;
}

Evaluation::Evaluation() {
	this->layout = nullptr;
}

Evaluation::Evaluation(Layout &layout) {
	this->layout = &layout;
	evaluate();
}

Evaluation::~Evaluation() {
}

void Evaluation::init() { layers.clear(); incomplete.clear();
	// Do the stupid thing first.

	// TODO(edward.bingham) Really, we should identify all of the DRC rules that
	// could possibly be enabled as a result of the paint we have in this layout,
	// and the ORed rules and the NOT rules. For now we're just starting by
	// assuming we have every layer of paint possible in this layout.
	for (auto paint = layout->tech->paint.begin(); paint != layout->tech->paint.end(); paint++) {
		for (auto j = paint->out.begin(); j != paint->out.end(); j++) {
			auto pos = incomplete.insert(pair<int, int>(*j, 0)).first;
			pos->second++;
		}
	}

	/*for (auto i = layout->layers.begin(); i != layout->layers.end(); i++) {
		for (auto j = layout->tech->paint[i->draw].out.begin(); j != layout->tech->paint[i->draw].out.end(); j++) {
			auto pos = incomplete.insert(pair<int, int>(*j, 0)).first;
			pos->second++;
		}
	}*/
}

bool Evaluation::has(int idx) {
	if (idx >= 0) {
		for (int i = 0; i < (int)layout->layers.size(); i++) {
			if (layout->layers[i].draw == idx) {
				return true;
			}
		}
	}
	return (layers.find(idx) != layers.end());
}

Layer &Evaluation::at(int idx) {
	if (idx >= 0) {
		for (int i = 0; i < (int)layout->layers.size(); i++) {
			if (layout->layers[i].draw == idx) {
				return layout->layers[i];
			}
		}
	}
	return (layers.insert(pair<int, Layer>(idx, Layer(*(layout->tech), idx))).first)->second;
}

void Evaluation::evaluate() {
	init();

	bool progress = true;
	while (progress) {
		progress = false;
		for (auto i = incomplete.begin(); i != incomplete.end(); ) {
			const Rule &rule = layout->tech->rules[flip(i->first)];
			const vector<int> &arg = rule.operands;

			if (i->second != (int)rule.operands.size() or not rule.isOperator()) {
				i++;
				continue;
			}

			switch (rule.type) {
			case Rule::NOT: at(i->first) = ~at(arg[0]); break;
			case Rule::AND: at(i->first) = at(arg[0]) & at(arg[1]); break;
			case Rule::OR:  at(i->first) = at(arg[0]) | at(arg[1]); break;
			case Rule::INTERACT: at(i->first) = interact(at(arg[0]), at(arg[1])); break;
			case Rule::NOT_INTERACT: at(i->first) = not_interact(at(arg[0]), at(arg[1])); break;
			default: printf("%s:%d error: unsupported operation (rule[%d].type=%d).\n", __FILE__, __LINE__, flip(i->first), rule.type);
			}

			for (auto j = rule.out.begin(); j != rule.out.end(); j++) {
				auto pos = incomplete.insert(pair<int, int>(*j, 0)).first;
				pos->second++;
			}
			
			i = incomplete.erase(i);
			progress = true;
		}
	}
}

/*Layout::Layout() {
	tech = nullptr;
	box = Rect();
}*/

Layout::Layout(const Tech &tech) {
	this->tech = &tech;
	this->box = Rect();
}

Layout::~Layout() {
}

vector<Layer>::const_iterator Layout::find(int draw, int label, int pin) const {
	auto layer = lower_bound(layers.begin(), layers.end(), draw);
	if (layer == layers.end() or layer->draw != draw) {
		return layers.end();
	}
	return layer;
}

vector<Layer>::iterator Layout::find(int draw, int label, int pin) {
	auto layer = lower_bound(layers.begin(), layers.end(), draw);
	if (layer == layers.end() or layer->draw != draw) {
		return layers.end();
	}
	return layer;
}

vector<Layer>::iterator Layout::at(int draw, int label, int pin) {
	auto layer = lower_bound(layers.begin(), layers.end(), draw);
	if (layer == layers.end() or layer->draw != draw) {
		layer = layers.insert(layer, Layer(*tech, draw, label, pin));
	}
	if (layer->draw < 0) {
		layer->draw = draw;
	}
	if (layer->label < 0) {
		layer->label = label;
	}
	if (layer->pin < 0) {
		layer->pin = pin;
	}
	return layer;
}

Rect Layout::bbox() const {
	if (layers.empty()) {
		return Rect();
	}
	
	Rect box = layers[0].bbox();
	for (int i = 1; i < (int)layers.size(); i++) {
		Rect sub = layers[i].bbox();
		box.bound(sub.ll, sub.ur);
	}
	return box;
}

void Layout::merge(bool doSync) {
	for (auto layer = layers.begin(); layer != layers.end(); layer++) {
		layer->merge(doSync);
	}
}

void Layout::push(int layer, Rect rect, bool doSync) {
	at(layer)->push(rect, doSync);
}

void Layout::push(int layer, vector<Rect> rects, bool doSync) {
	at(layer)->push(rects, doSync);
}

void Layout::push(const Material &mat, Rect rect, bool doSync) {
	at(mat.draw, mat.label, mat.pin)->push(rect, doSync);
}

void Layout::push(const Material &mat, vector<Rect> rects, bool doSync) {
	at(mat.draw, mat.label, mat.pin)->push(rects, doSync);
}

void Layout::clear() {
	name.clear();
	box = Rect();
	layers.clear();
	nets.clear();
}

struct StackElem {
	StackElem() {
		net = -1;
		pos = 0;
	}
	StackElem(int net, int pos) {
		this->net = net;
		this->pos = pos;
	}
	~StackElem() {}

	int net;
	int pos;
};

bool operator==(const StackElem &e0, const StackElem &e1) {
	return e0.pos == e1.pos and e0.net == e1.net;
}

bool operator<(const StackElem &e0, const StackElem &e1) {
	return e0.pos < e1.pos or (e0.pos == e1.pos and e0.net < e1.net);
}

// Compute the offset from (0,0) of the l0 geometry to (0,0) of the l1 geometry
// along axis at which l0 and l1 abut and save into offset. Require spacing on
// the opposite axis for non-intersection (default is 0). Return false if the two geometries
// will never intersect.
bool minOffset(int *offset, const Tech &tech, int axis, Layer &l0, int l0Shift, Layer &l1, int l1Shift, vec2i spacing, bool mergeNet) {
	if (l0.dirty) {
		l0.sync();
	}
	if (l1.dirty) {
		l1.sync();
	}

	bool conflict = false;

	// indexed as [layer]
	vector<StackElem> stack[2] = {vector<StackElem>(), vector<StackElem>()};
	// indexed as [layer][fromTo]
	int idx[2][2] = {{0, 0}, {0, 0}};
	while (true) {
		// We're simultaneously iterating over four arrays in sorted order. So we
		// need to find the index that points to the minimal element in the sort
		// order and then increment it.
		int minValue = -1;
		int minLayer = -1;
		int minFromTo = -1;
		for (int layer = 0; layer < 2; layer++) {
			for (int fromTo = 0; fromTo < 2; fromTo++) {
				int boundIdx = idx[layer][fromTo];
				const vector<Bound> &bounds = layer ? l1.bound[1-axis][fromTo] : l0.bound[1-axis][fromTo];
				int shift = layer ? l1Shift : l0Shift;

				if (boundIdx < (int)bounds.size()) {
					int value = bounds[boundIdx].pos + shift + (2*fromTo - 1)*spacing[1-axis]/2;
					if (minLayer < 0 or value < minValue or (value == minValue and minFromTo < fromTo)) {
						minValue = value;
						minLayer = layer;
						minFromTo = fromTo;
					}
				}
			}
		}

		if (minLayer < 0) {
			break;
		}

		int boundIdx = idx[minLayer][minFromTo];
		const Bound &bound = minLayer ? l1.bound[1-axis][minFromTo][boundIdx] : l0.bound[1-axis][minFromTo][boundIdx];
		Rect rect = minLayer ? l1.geo[bound.idx] : l0.geo[bound.idx];

		// Since we're measuring distance from layer 0 to layer 1, then we need to
		// look at layer 0's to boundary and layer 1's from boundary. From is index
		// 0 and to is index 1, so we need 1-layer.
		StackElem elem(rect.net, rect[1-minLayer][axis]);

		auto loc = lower_bound(stack[minLayer].begin(), stack[minLayer].end(), elem);
		if (minFromTo) {
			// When the index found is the end of a rectangle, we remove that bound
			// from the stack.
			if (loc != stack[minLayer].end() and *loc == elem) {
				stack[minLayer].erase(loc);
			}
		} else {
			// When the index found is the start of a rectangle, then we add that
			// rectangle to the associated stack.
			stack[minLayer].insert(loc, elem);
			// Then we need to check the distances to the opposite layer along the
			// opposite axis. We need to compute the distances from left to right or
			// bottom to top
			if (minLayer == 0) {
				// from layer 0 to layer 1
				for (int i = 0; i < (int)stack[1].size(); i++) {
					if (l0.draw != l1.draw or stack[1][i].net != elem.net or not mergeNet) {
						int diff = elem.pos + spacing[axis] - stack[1][i].pos;
						if (diff > *offset) {
							*offset = diff;
							conflict = true;
						}
						break;
					}
				}
			} else if (minLayer == 1) {
				// from layer 1 to layer 0
				for (int i = (int)stack[0].size()-1; i >= 0; i--) {
					if (l0.draw != l1.draw or stack[0][i].net != elem.net or not mergeNet) {
						int diff = stack[0][i].pos + spacing[axis] - elem.pos;
						if (diff > *offset) {
							*offset = diff;
							conflict = true;
						}
						break;
					}
				}
			}
		}

		idx[minLayer][minFromTo]++;
	}

	return conflict;
}

bool minOffset(int *offset, const Tech &tech, int axis, Layout &left, int leftShift, Layout &right, int rightShift, int substrateMode, int routingMode, bool horizSpacing) {
	Evaluation e0(left);
	Evaluation e1(right);

	/*printf("e0 layers:\n");
	for (int i = 0; i < (int)e0.layout->layers.size(); i++) {
		printf("%d: %s\n", e0.layout->layers[i].draw, tech.print(e0.layout->layers[i].draw).c_str());
	}
	for (auto i = e0.layers.begin(); i != e0.layers.end(); i++) {
		printf("%d: %s\n", i->first, tech.print(i->first).c_str());
	}
	printf("e1 layers:\n");
	for (int i = 0; i < (int)e1.layout->layers.size(); i++) {
		printf("%d: %s\n", e1.layout->layers[i].draw, tech.print(e1.layout->layers[i].draw).c_str());
	}
	for (auto i = e1.layers.begin(); i != e1.layers.end(); i++) {
		printf("%d: %s\n", i->first, tech.print(i->first).c_str());
	}
	printf("\n");*/


	bool conflict = false;
	auto i0 = e0.incomplete.begin();
	auto i1 = e1.incomplete.begin();
	//printf("checking %d and %d rules\n", (int)e0.incomplete.size(), (int)e1.incomplete.size());
	while (i0 != e0.incomplete.end() and i1 != e1.incomplete.end()) {
		if (i0->first < i1->first) {
			//printf("unmatched i0=%d: %s\n", i0->second, tech.print(i0->first).c_str());
			i0++;
		} else if (i1->first < i0->first) {
			//printf("unmatched i1=%d: %s\n", i1->second, tech.print(i1->first).c_str());
			i1++;
		} else {
			//printf("matched rule %d i0=%d i1=%d: %s\n", i0->first, i0->second, i1->second, tech.print(i0->first).c_str());
			const Rule &rule = tech.rules[flip(i0->first)];

			vec2i spacing(rule.params[0], rule.params[0]);

			// TODO(edward.bingham) This is a hack. Really, we need to understand a
			// more complicated relationship between additive and subtractive
			// expressions in DRC spacing rules, then apply the subtractive piece of
			// the expression on one side to the addive piece on the other to
			// understand whether that additive part actually represents a potential
			// spacing violation. Realistically, this is only affecting transistor
			// spacing on the stack, and so we can just turn off the horizontal
			// spacing rules to prevent the problematic conflicts in those spacing
			// rules. See pages 201-204 of notes
			if (not horizSpacing) {
				spacing[1-axis] = 0;
			}

			if (rule.type == Rule::SPACING) {
				if (e0.has(rule.operands[0]) and e1.has(rule.operands[1])) {
					Layer &l0 = e0.at(rule.operands[0]);
					Layer &l1 = e1.at(rule.operands[1]);
				
					int leftMode = (l0.isRouting ? routingMode : (l0.isSubstrate ? substrateMode : Layout::DEFAULT));
					int rightMode = (l1.isRouting ? routingMode : (l1.isSubstrate ? substrateMode : Layout::DEFAULT));
					//printf("found e0 <-> e1: %d %d\n", leftMode, rightMode);

					if (leftMode != Layout::IGNORE and rightMode != Layout::IGNORE and (not l0.isFill(tech) or not l1.isFill(tech))) {
						bool newConflict = minOffset(offset, tech, axis, l0, leftShift, l1, rightShift, spacing, leftMode == Layout::MERGENET and rightMode == Layout::MERGENET);
						/*if (newConflict) {
							printf("found conflict: %d\n", *offset);
						} else {
							printf("no conflict\n");
						}*/
						conflict = conflict or newConflict;
					}
				}

				if (rule.operands[0] != rule.operands[1] and e0.has(rule.operands[1]) and e1.has(rule.operands[0])) {
					Layer &l0 = e0.at(rule.operands[1]);
					Layer &l1 = e1.at(rule.operands[0]);
					
					int leftMode = (l0.isRouting ? routingMode : (l0.isSubstrate ? substrateMode : Layout::DEFAULT));
					int rightMode = (l1.isRouting ? routingMode : (l1.isSubstrate ? substrateMode : Layout::DEFAULT));
					//printf("found e1 <-> e0: %d %d\n", leftMode, rightMode);

					if (leftMode != Layout::IGNORE and rightMode != Layout::IGNORE and (not l0.isFill(tech) or not l1.isFill(tech))) {
						bool newConflict = minOffset(offset, tech, axis, l0, leftShift, l1, rightShift, spacing, leftMode == Layout::MERGENET and rightMode == Layout::MERGENET);
						/*if (newConflict) {
							printf("found conflict: %d\n", *offset);
						} else {
							printf("no conflict\n");
						}*/
						conflict = conflict or newConflict;
					}
				}
			}

			i0++;
			i1++;
		}
	}
	return conflict;
}

}

