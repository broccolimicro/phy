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

bool Rect::contains(vec2i p) const {
	return p[0] >= ll[0] and p[0] <= ur[0] and p[1] >= ll[1] and p[1] <= ur[1];
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

vec2i Rect::center() const {
	return (ll+ur)/2;
}

int Rect::area() const {
	return (ur[0] - ll[0])*(ur[1] - ll[1]);
}

Rect operator&(const Rect &r0, const Rect &r1) {
	int net = -1;
	if (r0.net < 0 and r1.net >= 0) {
		net = r1.net;
	} else if (r0.net >= 0 and r1.net < 0) {
		net = r0.net;
	} else if (r0.net == r1.net) {
		net = r0.net;
	}

	return Rect(net, max(r0.ll, r1.ll), min(r0.ur, r1.ur));
}

Label::Label() {
	net = -1;
	pos = vec2i(0,0);
	txt = "";
}

Label::Label(int net, vec2i pos, string txt) {
	this->net = net;
	this->pos = pos;
	this->txt = txt;
}

Label::~Label() {
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

Layer::Layer(const Tech &tech) {
	this->tech = &tech;
	draw = Layer::UNKNOWN;
	dirty = false;
	isRouting = false;
	isSubstrate = false;
	isPin = false;
	isWell = false;
}

Layer::Layer(const Tech &tech, bool value) {
	this->tech = &tech;
	draw = Layer::UNKNOWN;
	dirty = false;
	isRouting = value;
	isSubstrate = not value;
	isPin = false;
	isWell = false;
	if (value) {
		int lo = std::numeric_limits<int>::min();
		int hi = std::numeric_limits<int>::max();
		push(Rect(-1, vec2i(lo, lo), vec2i(hi, hi)));
	}
}

Layer::Layer(const Tech &tech, int draw) {
	this->tech = &tech;
	this->draw = draw;
	this->dirty = false;

	this->isRouting = tech.isRouting(draw);
	this->isSubstrate = tech.isSubstrate(draw);
	this->isPin = tech.isPin(draw);
	this->isWell = tech.isWell(draw);
}

Layer::~Layer() {
}

bool Layer::isFill() const {
	if (draw < 0) {
		return false;
	}

	return tech->paint[draw].fill;
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
	if (rect.ll[0] < rect.ur[0] and rect.ll[1] < rect.ur[1]) {
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
}

void Layer::push(vector<Rect> rects, bool doSync) {
	int index = (int)geo.size();
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

void Layer::label(Label lbl) {
	this->lbl.push_back(lbl);
}

void Layer::label(vector<Label> lbls) {
	this->lbl.insert(this->lbl.end(), lbls.begin(), lbls.end());
}

Rect Layer::bbox() const {
	if (geo.empty()) {
		return Rect();
	}
	
	bool conflict = false;
	// DESIGN(edward.bingham) Naive approach first
	Rect box(geo[0].net, geo[0].ll, geo[0].ur);
	for (int i = 1; i < (int)geo.size(); i++) {
		if (box.net < 0 and not conflict) {
			box.net = geo[i].net;
		} else if (geo[i].net >= 0 and geo[i].net != box.net) {
			box.net = -1;
			conflict = true;
		}
		box.bound(geo[i].ll, geo[i].ur);
	}
	return box;
}

void Layer::merge(bool doSync) {
	// TODO use the bounds array to improve performance
	for (int i = (int)geo.size()-2; i >= 0; i--) {
		for (int j = (int)geo.size()-1; j > i; j--) {
			if (geo[i].merge(geo[j])) {
				erase(j, doSync);
			}
		}
	}
}

Layer Layer::clamp(int axis, int lo, int hi) const {
	Layer result(*tech, draw);
	result.isRouting = isRouting;
	result.isSubstrate = isSubstrate;

	for (auto r = geo.begin(); r != geo.end(); r++) {
		Rect n = *r;
		if (n.ll[axis] < lo) {
			n.ll[axis] = lo;
		}
		if (n.ur[axis] > hi) {
			n.ur[axis] = hi;
		}
		if (n.ll[axis] < n.ur[axis]) {
			result.geo.push_back(n);
			result.dirty = true;
		}
	}
	return result;
}

Layer &Layer::shift(vec2i pos, vec2i dir) {
	for (auto r = geo.begin(); r != geo.end(); r++) {
		r->shift(pos, dir);
	}
	dirty = true;
	return *this;
}

Layer &Layer::fillSpacing() {
	int minSpacing = tech->getSpacing(draw, draw);
	for (int i = (int)geo.size()-1; i >= 0; i--) {
		auto ri = geo.begin()+i;
		if (ri->net < 0) {
			continue;
		}

		for (int j = i-1; j >= 0; j--) {
			auto rj = geo.begin()+j;
			if (ri->net != rj->net) {
				continue;
			}

			if (rj->ll[1] < ri->ur[1] and ri->ll[1] < rj->ur[1]
				and ((ri->ur[0] < rj->ll[0] and rj->ll[0] < ri->ur[0]+minSpacing)
					or (rj->ur[0] < ri->ll[0] and ri->ll[0] < rj->ur[0]+minSpacing))) {
				// horizontal spacing violation
				push(Rect(ri->net,
					vec2i(min(ri->ur[0], rj->ur[0]), max(ri->ll[1], rj->ll[1])),
					vec2i(max(ri->ll[0], rj->ll[0]), min(ri->ur[1], rj->ur[1]))));
			}

			if (rj->ll[0] < ri->ur[0] and ri->ll[0] < rj->ur[0]
				and ((ri->ur[1] < rj->ll[1] and rj->ll[1] < ri->ur[1]+minSpacing)
					or (rj->ur[1] < ri->ll[1] and ri->ll[1] < rj->ur[1]+minSpacing))) {
				// vertical spacing violation
				push(Rect(ri->net,
					vec2i(min(ri->ur[0], rj->ur[0]), max(ri->ll[1], rj->ll[1])),
					vec2i(max(ri->ll[0], rj->ll[0]), min(ri->ur[1], rj->ur[1]))));
			}
		}
	}
	return *this;
}

vector<vector<int> > Layer::trace() {
	//printf("tracing %d\n", draw);
	vector<vector<int> > clusters;
	for (int r = 0; r < (int)geo.size(); r++) {
		auto rect = geo.begin()+r;

		// find clusters with overlapping geometry
		vector<int> found;
		for (int c = 0; c < (int)clusters.size(); c++) {
			for (auto i = clusters[c].begin(); i != clusters[c].end(); i++) {
				if (rect->overlaps(geo[*i])) {
					found.push_back(c);
					break;
				}
			}
		}

		if (not found.empty()) {
			// If more than one cluster is found, then we combine those clusters
			for (int i = (int)found.size()-1; i >= 1; i--) {
				clusters[found[0]].insert(clusters[found[0]].end(), clusters[found[i]].begin(), clusters[found[i]].end());
				clusters.erase(clusters.begin() + found[i]);
			}
			// Then add this rectangle to the remaining cluster
			clusters[found[0]].push_back(r);
		} else {
			// No overlapping clusters were found, create a new cluster for this rectangle
			clusters.push_back(vector<int>(1, r));
		}
	}
	//printf("done tracing %d\n", draw);
	return clusters;
}

vector<Layer> Layer::split(vector<vector<int> > clusters) {
	if (clusters.empty()) {
		clusters = trace();
	}

	vector<Layer> result;
	result.reserve(clusters.size());
	for (auto i = clusters.begin(); i != clusters.end(); i++) {
		result.push_back(Layer(*tech, draw));
		result.back().isRouting = isRouting;
		result.back().isSubstrate = isSubstrate;
		result.back().isPin = isPin;
		result.back().isWell = isWell;
		for (auto j = i->begin(); j != i->end(); j++) {
			result.back().push(geo[*j]);
		}
	}
	return result;
}

int Layer::area(vector<int> cluster) {
	int result = 0;
	for (auto i = geo.begin(); i != geo.end(); i++) {
		result += i->area();
		for (auto j = geo.begin(); j != i; j++) {
			if (i->overlaps(*j)) {
				result -= (*i & *j).area();
			}
		}
	}
	return result;
}

bool Layer::overlaps(const Rect &r0) const {
	for (auto i = geo.begin(); i != geo.end(); i++) {
		if (i->overlaps(r0)) {
			return true;
		}
	}
	return false;
}

bool Layer::overlaps(const Layer &l0) const {
	for (auto i = geo.begin(); i != geo.end(); i++) {
		if (l0.overlaps(*i)) {
			return true;
		}
	}
	return false;
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

	Layer result(*l0.tech);
	result.isRouting = l0.isRouting and l1.isRouting;
	result.isSubstrate = l0.isSubstrate or l1.isSubstrate;
	result.isPin = l0.isPin or l1.isPin;
	result.isWell = l0.isWell and l1.isWell;
	for (auto r0 = l0.geo.begin(); r0 != l0.geo.end(); r0++) {
		for (auto r1 = l1.geo.begin(); r1 != l1.geo.end(); r1++) {
			if (r0->overlaps(*r1)) {
				int net = -1;
				if (r0->net < 0 and r1->net >= 0 and not l1.isWell) {
					net = r1->net;
				} else if (r0->net >= 0 and not l0.isWell and r1->net < 0) {
					net = r0->net;
				} else if (r0->net == r1->net) {
					net = r0->net;
				}

				result.push(Rect(net, max(r0->ll, r1->ll), min(r0->ur, r1->ur)));
			}
		}
	}
	//result.merge();
	return result;
}

Layer interact(const Layer &l0, const Layer &l1) {
	//l0.sync();
	//l1.sync();

	Layer result(*l0.tech);
	result.draw = l0.draw;
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

	Layer result(*l0.tech);
	result.draw = l0.draw;
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

	Layer result(*l0.tech);
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

	Layer result(*l.tech, true);
	result.isRouting = not l.isRouting;
	result.isSubstrate = not l.isSubstrate;
	for (int i = 0; i < (int)l.geo.size(); i++) {
		Layer step(*l.tech);
		step.push(Rect(-1, vec2i(l.geo[i].ur[0], lo), vec2i(hi, hi)));
		step.push(Rect(-1, vec2i(lo, lo), vec2i(l.geo[i].ll[0], hi)));
		step.push(Rect(-1, vec2i(l.geo[i].ll[0], lo), vec2i(l.geo[i].ur[0], l.geo[i].ll[1])));
		step.push(Rect(-1, vec2i(l.geo[i].ll[0], l.geo[i].ur[1]), vec2i(l.geo[i].ur[0], hi)));
		result = result & step;
		result.merge();
	}
	return result;
}

Evaluation::Evaluation(const Tech &tech) : empty(tech) {
	this->layout = nullptr;
}

Evaluation::Evaluation(const Layout &layout) : empty(*layout.tech) {
	this->layout = &layout;
	evaluate();
}

Evaluation::~Evaluation() {
}

void Evaluation::init() {
	layers.clear();
	incomplete.clear();
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
		return (layout->find(idx) != layout->layers.end());
	}
	return (layers.find(idx) != layers.end());
}

const Layer &Evaluation::at(int idx) const {
	if (idx >= 0) {
		auto pos = layout->find(idx);
		if (pos != layout->layers.end()) {
			return pos->second;
		}
	}
	auto pos = layers.find(idx);
	if (pos != layers.end()) {
		return pos->second;
	} else {
		return empty;
	}
}

Layer &Evaluation::set(int idx) {
	return layers.insert(pair<int, Layer>(idx, Layer(*layout->tech, idx))).first->second;
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
			case Rule::NOT: set(i->first) = ~at(arg[0]); break;
			case Rule::AND: set(i->first) = at(arg[0]) & at(arg[1]); break;
			case Rule::OR:  set(i->first) = at(arg[0]) | at(arg[1]); break;
			case Rule::INTERACT: set(i->first) = interact(at(arg[0]), at(arg[1])); break;
			case Rule::NOT_INTERACT: set(i->first) = not_interact(at(arg[0]), at(arg[1])); break;
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

Net::Net() {
	isVdd = false;
	isGND = false;
	isInput = false;
	isOutput = false;
	isSub = false;
}

Net::Net(string name) {
	names.push_back(name);
	isVdd = false;
	isGND = false;
	isInput = false;
	isOutput = false;
	isSub = false;
}

Net::~Net() {
}

void Net::set(string name) {
	auto pos = std::lower_bound(names.begin(), names.end(), name);
	if (pos == names.end() or *pos != name) {
		names.insert(pos, name);
	}
}

bool Net::has(string name) const {
	auto pos = std::lower_bound(names.begin(), names.end(), name);
	return (pos != names.end() and *pos == name);
}

Layout::Layout(const Tech &tech) {
	this->tech = &tech;
	this->box = Rect();
}

Layout::~Layout() {
}

map<int, Layer>::const_iterator Layout::find(int draw) const {
	return layers.find(draw);
}

map<int, Layer>::iterator Layout::find(int draw) {
	return layers.find(draw);
}

map<int, Layer>::iterator Layout::at(int draw) {
	auto result = layers.insert(pair<int, Layer>(draw, Layer(*tech, draw)));
	return result.first;
}

void Layout::push(int layer, Rect rect, bool doSync) {
	at(layer)->second.push(rect, doSync);
}

void Layout::push(int layer, vector<Rect> rects, bool doSync) {
	at(layer)->second.push(rects, doSync);
}

void Layout::push(const Material &mat, Rect rect, bool doSync) {
	at(mat.draw < 0 ? mat.label : mat.draw)->second.push(rect, doSync);
}

void Layout::push(const Material &mat, vector<Rect> rects, bool doSync) {
	at(mat.draw < 0 ? mat.label : mat.draw)->second.push(rects, doSync);
}

void Layout::label(int layer, Label lbl) {
	at(layer)->second.label(lbl);
}

void Layout::label(int layer, vector<Label> lbls) {
	at(layer)->second.label(lbls);
}

void Layout::label(const Material &mat, Label lbl) {
	at(mat.label)->second.label(lbl);
}

void Layout::label(const Material &mat, vector<Label> lbls) {
	at(mat.label)->second.label(lbls);
}

int Layout::netAt(string name) {
	for (int i = 0; i < (int)nets.size(); i++) {
		if (nets[i].has(name)) {
			return i;
		}
	}
	int result = (int)nets.size();
	nets.push_back(name);
	return result;
}

Rect Layout::bbox() const {
	if (layers.empty()) {
		return Rect();
	}
	
	Rect box = layers.begin()->second.bbox();
	for (auto i = std::next(layers.begin()); i != layers.end(); i++) {
		box.bound(i->second.bbox());
	}
	return box;
}

void Layout::merge(bool doSync) {
	for (auto layer = layers.begin(); layer != layers.end(); layer++) {
		layer->second.merge(doSync);
	}
}

void Layout::trace() {
	//printf("tracing cell %s\n", name.c_str());
	// index into Layout::layers, index into Layer::geo
	vector<map<int, vector<int> > > traces;

	/*for (auto layer = layers.begin(); layer != layers.end(); layer++) {
		printf("layer %s(%d)\n", tech->paint[layer->second.draw].name.c_str(), layer->second.draw);
		for (auto rect = layer->second.geo.begin(); rect != layer->second.geo.end(); rect++) {
			printf("rect %d (%d %d) (%d %d)\n", rect->net, rect->ll[0], rect->ll[1], rect->ur[0], rect->ur[1]);
		}
		for (auto lbl = layer->second.lbl.begin(); lbl != layer->second.lbl.end(); lbl++) {
			printf("label %d (%d %d) %s\n", lbl->net, lbl->pos[0], lbl->pos[1], lbl->txt.c_str());
		}
	}*/

	// For a given layer
	//printf("tracing layers\n");
	vector<vector<int> > clusters;
	for (auto layer = layers.begin(); layer != layers.end(); layer++) {
		if (not layer->second.isRouting and not layer->second.isPin and not layer->second.isWell) {
			continue;
		}

		clusters = layer->second.trace();
		for (auto c = clusters.begin(); c != clusters.end(); c++) {
			traces.push_back(map<int, vector<int> >());
			traces.back().insert(pair<int, vector<int> >(layer->first, *c));
		}
		clusters.clear();
	}

	/*printf("traces\n");
	for (int i = 0; i < (int)traces.size(); i++) {
		printf("%d {\n", i);
		for (auto t = traces[i].begin(); t != traces[i].end(); t++) {
			printf("\t%d -> {", t->first);
			for (int j = 0; j < (int)t->second.size(); j++) {
				printf("%d ", t->second[j]);
			}
			printf("}\n");
		}
		printf("}, ");
	}
	printf("}\n");

	printf("looking for via connections\n");*/
	for (auto via = tech->vias.begin(); via != tech->vias.end(); via++) {
		const Material *up = tech->atMaterial(via->upLevel);
		const Material *dn = tech->atMaterial(via->downLevel);
		if (up == nullptr or dn == nullptr) {
			printf("error: unable to find connecting layers for via\n");
			continue;
		}

		vector<pair<int, int> > connect = {
			{dn->draw, via->draw},
			{dn->pin, via->draw},
			{dn->draw, via->pin},
			{dn->pin, via->pin},
			{via->draw, up->draw},
			{via->pin, up->draw},
			{via->draw, up->pin},
			{via->pin, up->pin},
			{via->draw, dn->draw},
			{via->draw, dn->pin},
			{via->pin, dn->draw},
			{via->pin, dn->pin},
			{up->draw, via->draw},
			{up->draw, via->pin},
			{up->pin, via->draw},
			{up->pin, via->pin},
			{dn->draw, dn->pin},
			{dn->pin, dn->draw},
			{via->draw, via->pin},
			{via->pin, via->draw},
			{up->draw, up->pin},
			{up->pin, up->draw},
		};

		for (int i = (int)traces.size()-2; i >= 0; i--) {
			for (int j = (int)traces.size()-1; j > i; j--) {
				for (auto k = connect.begin(); k != connect.end(); k++) {
					auto lo = traces[i].find(k->first);
					auto hi = traces[j].find(k->second);
					auto glo = find(k->first);
					auto ghi = find(k->second);
					if (lo != traces[i].end() and hi != traces[j].end() and glo != layers.end() and ghi != layers.end()) {
						// check to see if these layers intersect at all
						bool found = false;
						for (auto r0 = lo->second.begin(); r0 != lo->second.end() and not found; r0++) {
							for (auto r1 = hi->second.begin(); r1 != hi->second.end() and not found; r1++) {
								found = glo->second.geo[*r0].overlaps(ghi->second.geo[*r1]);
							}
						}

						if (found) {
							auto ii = traces[i].begin();
							auto ji = traces[j].begin();
							while (ii != traces[i].end() and ji != traces[j].end()) {
								if (ii->first == ji->first) {
									ii->second.insert(ii->second.end(), ji->second.begin(), ji->second.end());
									ii++;
									ji++;
								} else if (ii->first < ji->first) {
									ii++;
								} else {
									traces[i].insert(ii, *ji);
									ii++;
									ji++;
								}
							}
							if (ji != traces[j].end()) {
								traces[i].insert(ji, traces[j].end());
							}
							traces.erase(traces.begin()+j);
							break;
						}
					}
				}
			}
		}
	}

	/*printf("traces\n");
	for (int i = 0; i < (int)traces.size(); i++) {
		printf("%d {\n", i);
		for (auto t = traces[i].begin(); t != traces[i].end(); t++) {
			printf("\t%d -> {", t->first);
			for (int j = 0; j < (int)t->second.size(); j++) {
				printf("%d ", t->second[j]);
			}
			printf("}\n");
		}
		printf("}, ");
	}
	printf("}\n");

	printf("finding labels\n");*/
	vector<int> mapping(traces.size(), -1);

	nets.clear();
	for (auto layer = layers.begin(); layer != layers.end(); layer++) {
		for (auto r = layer->second.geo.begin(); r != layer->second.geo.end(); r++) {
			r->net = -1;
		}
		for (auto lbl = layer->second.lbl.begin(); lbl != layer->second.lbl.end(); lbl++) {
			lbl->net = -1;
		}

		if (layer->second.lbl.empty()) {
			//printf("no labels %d\n", i);
			continue;
		}

		const Material *mat = tech->findMaterial(layer->first);
		if (mat == nullptr) {
			printf("mat not found %s(%d)\n", tech->paint[layer->first].name.c_str(), layer->first);
			continue;
		}

		for (int n = 0; n < (int)traces.size(); n++) {
			for (int k = 0; k < 2; k++) {
				auto pos = traces[n].find(k == 0 ? mat->draw : mat->pin);
				auto gpos = layers.find(k == 0 ? mat->draw : mat->pin);
				if (pos == traces[n].end() or gpos == layers.end()) {
					//printf("mat not in trace %d\n", k == 0 ? mat->draw : mat->pin);
					continue;
				}

				for (auto lbl = layer->second.lbl.begin(); lbl != layer->second.lbl.end(); lbl++) {
					for (auto r = pos->second.begin(); r != pos->second.end(); r++) {
						if (gpos->second.geo[*r].contains(lbl->pos)) {
							if (mapping[n] < 0) {
								mapping[n] = netAt(lbl->txt);
							}
							lbl->net = mapping[n];
							nets[mapping[n]].set(lbl->txt);
							break;
						}
					}
				}
			}
		}
	}

	for (auto layer = layers.begin(); layer != layers.end(); layer++) {
		if (layer->second.isRouting or layer->second.isWell or layer->second.isPin or layer->second.isSubstrate) {
			for (auto lbl = layer->second.lbl.begin(); lbl != layer->second.lbl.end(); lbl++) {
				if (lbl->net < 0) {
					lbl->net = netAt(lbl->txt);

					if (layer->second.isPin or layer->second.isWell) {
						nets[lbl->net].isInput = true;
						nets[lbl->net].isOutput = true;
					}
					if (layer->second.isWell) {
						nets[lbl->net].isSub = true;
					}
				}
			}
		}
	}

	//printf("saving net ids\n");
	for (int n = 0; n < (int)traces.size(); n++) {
		for (auto l = traces[n].begin(); l != traces[n].end(); l++) {
			if (mapping[n] < 0) {
				mapping[n] = netAt("_" + to_string((int)nets.size()));
			}
			auto layer = layers.find(l->first);
			if (layer == layers.end()) {
				continue;
			}
			if (layer->second.isPin or layer->second.isWell) {
				nets[mapping[n]].isInput = true;
				nets[mapping[n]].isOutput = true;
			}
			if (layer->second.isWell) {
				nets[mapping[n]].isSub = true;
			}
			for (auto r = l->second.begin(); r != l->second.end(); r++) {
				layer->second.geo[*r].net = mapping[n];
			}
		}
	}

	/*for (auto layer = layers.begin(); layer != layers.end(); layer++) {
		printf("layer %s(%d)\n", tech->paint[layer->second.draw].name.c_str(), layer->second.draw);
		for (auto rect = layer->second.geo.begin(); rect != layer->second.geo.end(); rect++) {
			printf("rect %d (%d %d) (%d %d)\n", rect->net, rect->ll[0], rect->ll[1], rect->ur[0], rect->ur[1]);
		}
		for (auto lbl = layer->second.lbl.begin(); lbl != layer->second.lbl.end(); lbl++) {
			printf("label %d (%d %d) %s\n", lbl->net, lbl->pos[0], lbl->pos[1], lbl->txt.c_str());
		}
	}

	printf("done\n");*/
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
bool minOffset(int *offset, int axis, const Layer &l0, int l0Shift, const Layer &l1, int l1Shift, vec2i spacing, bool mergeNet) {
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

bool minOffset(int *offset, int axis, const Layout &left, int leftShift, const Layout &right, int rightShift, int substrateMode, int routingMode, bool horizSpacing) {
	Evaluation e0(left);
	Evaluation e1(right);

	/*printf("e0 layers:\n");
	for (int i = 0; i < (int)e0.layout->layers.size(); i++) {
		printf("%d: %s\n", e0.layout->layers[i].draw, tech->print(e0.layout->layers[i].draw).c_str());
	}
	for (auto i = e0.layers.begin(); i != e0.layers.end(); i++) {
		printf("%d: %s\n", i->first, tech->print(i->first).c_str());
	}
	printf("e1 layers:\n");
	for (int i = 0; i < (int)e1.layout->layers.size(); i++) {
		printf("%d: %s\n", e1.layout->layers[i].draw, tech->print(e1.layout->layers[i].draw).c_str());
	}
	for (auto i = e1.layers.begin(); i != e1.layers.end(); i++) {
		printf("%d: %s\n", i->first, tech->print(i->first).c_str());
	}
	printf("\n");*/


	bool conflict = false;
	auto i0 = e0.incomplete.begin();
	auto i1 = e1.incomplete.begin();
	//printf("checking %d and %d rules\n", (int)e0.incomplete.size(), (int)e1.incomplete.size());
	while (i0 != e0.incomplete.end() and i1 != e1.incomplete.end()) {
		if (i0->first < i1->first) {
			//printf("unmatched i0=%d: %s\n", i0->second, tech->print(i0->first).c_str());
			i0++;
		} else if (i1->first < i0->first) {
			//printf("unmatched i1=%d: %s\n", i1->second, tech->print(i1->first).c_str());
			i1++;
		} else {
			//printf("matched rule %d i0=%d i1=%d: %s\n", i0->first, i0->second, i1->second, tech->print(i0->first).c_str());
			const Rule &rule = left.tech->rules[flip(i0->first)];

			if (rule.type == Rule::SPACING) {
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

				if (e0.has(rule.operands[0]) and e1.has(rule.operands[1])) {
					const Layer &l0 = e0.at(rule.operands[0]);
					const Layer &l1 = e1.at(rule.operands[1]);
			
					int leftMode = (l0.isRouting ? routingMode : (l0.isSubstrate ? (l0.isFill() ? Layout::MERGENET : substrateMode) : Layout::DEFAULT));
					int rightMode = (l1.isRouting ? routingMode : (l1.isSubstrate ? (l1.isFill() ? Layout::MERGENET : substrateMode) : Layout::DEFAULT));
					//printf("found e0 <-> e1: %d %d\n", leftMode, rightMode);

					if (leftMode != Layout::IGNORE and rightMode != Layout::IGNORE) {// and (not l0.isFill() or not l1.isFill())) {
						bool newConflict = minOffset(offset, axis, l0, leftShift, l1, rightShift, spacing, leftMode == Layout::MERGENET and rightMode == Layout::MERGENET);
						/*if (newConflict) {
							printf("found conflict: %d\n", *offset);
						} else {
							printf("no conflict\n");
						}*/
						conflict = conflict or newConflict;
					}
				}

				if (rule.operands[0] != rule.operands[1] and e0.has(rule.operands[1]) and e1.has(rule.operands[0])) {
					const Layer &l0 = e0.at(rule.operands[1]);
					const Layer &l1 = e1.at(rule.operands[0]);
					
					int leftMode = (l0.isRouting ? routingMode : (l0.isSubstrate ? (l0.isFill() ? Layout::MERGENET : substrateMode) : Layout::DEFAULT));
					int rightMode = (l1.isRouting ? routingMode : (l1.isSubstrate ? (l1.isFill() ? Layout::MERGENET : substrateMode) : Layout::DEFAULT));
					//printf("found e1 <-> e0: %d %d\n", leftMode, rightMode);

					if (leftMode != Layout::IGNORE and rightMode != Layout::IGNORE) {// and (not l0.isFill() or not l1.isFill())) {
						bool newConflict = minOffset(offset, axis, l0, leftShift, l1, rightShift, spacing, leftMode == Layout::MERGENET and rightMode == Layout::MERGENET);
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

