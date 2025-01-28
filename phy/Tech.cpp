#include "Tech.h"

#include <limits>
#include <algorithm>

using namespace std;

int flip(int idx) {
	return -idx-1;
}

namespace phy {

Paint::Paint() {
	this->name = "";
	this->major = 0;
	this->minor = 0;
	this->fill = false;
}

Paint::Paint(string name, int major, int minor) {
	this->name = name;
	this->major = major;
	this->minor = minor;
	this->fill = false;
}

Paint::~Paint() {
}

Level::Level() {
	type = Level::INVALID;
	idx = 0;
}

Level::Level(int type, int idx) {
	this->type = type;
	this->idx = idx;
}

Level::~Level() {
}

bool Level::valid() const {
	return type != Level::INVALID;
}

bool operator==(Level l0, Level l1) {
	return l0.type == l1.type and l0.idx == l1.idx;
}

bool operator!=(Level l0, Level l1) {
	return l0.type != l1.type or l0.idx != l1.idx;
}

bool operator<(Level l0, Level l1) {
	return l0.type < l1.type or (l0.type == l1.type and l0.idx < l1.idx);
}

bool operator>(Level l0, Level l1) {
	return l0.type > l1.type or (l0.type == l1.type and l0.idx > l1.idx);
}

bool operator<=(Level l0, Level l1) {
	return l0.type < l1.type or (l0.type == l1.type and l0.idx <= l1.idx);
}

bool operator>=(Level l0, Level l1) {
	return l0.type > l1.type or (l0.type == l1.type and l0.idx >= l1.idx);
}

Material::Material() {
	draw = -1;
	label = -1;
	pin = -1;
	thickness = 0;
	resistivity = 0;
}

Material::Material(int draw, int label, int pin, float thickness, float resistivity) {
	this->draw = draw;
	this->pin = pin;
	this->label = label;
	this->thickness = thickness;
	this->resistivity = resistivity;
}

Material::~Material() {
}

bool Material::contains(int layer) const {
	return layer == draw or layer == label or layer == pin or find(mask.begin(), mask.end(), layer) != mask.end();
}

int Material::size() const {
	return 3;
}

int Material::at(int idx) const {
	array<int, 3> arr({draw, pin, label});
	return arr[idx];
}

bool Material::hasDraw() const {
	return draw >= 0;
}

Substrate::Substrate() : Material() {
}

Substrate::Substrate(int draw, int label, int pin, int tap, Level well, float thickness, float resistivity) : Material(draw, label, pin, thickness, resistivity) {
	this->tap = tap;
	this->well = well;
}

Substrate::~Substrate() {
}

Model::Model() {
	variant = "";
	name = "";
	type = NMOS;
}

Model::Model(int type, string variant, string name, Level diff, vector<pair<int, int> > bins) {
	this->variant = variant;
	this->name = name;
	this->type = type;
	this->diff = diff;
	this->bins = bins;
	std::sort(this->bins.begin(), this->bins.end());
}

Model::~Model() {
}

Routing::Routing() : Material() {
}

Routing::Routing(int draw, int label, int pin, float thickness, float resistivity) : Material(draw, label, pin, thickness, resistivity) {
}

Routing::~Routing() {
}

Via::Via() : Material() {
}

Via::Via(Level down, Level up, int draw, int label, int pin, float thickness, float resistivity) : Material(draw, label, pin, thickness, resistivity) {
	this->down = down;
	this->up = up;
}

Via::~Via() {
}

Dielectric::Dielectric() {
	thickness = 0.0f;
	permitivity = 0.0f;
}

Dielectric::Dielectric(Level down, Level up, float thickness, float permitivity) {
	this->down = down;
	this->up = up;
	this->thickness = thickness;
	this->permitivity = permitivity;
}

Dielectric::~Dielectric() {
}

Rule::Rule() {
	this->type = -1;
}

Rule::Rule(int type, vector<int> operands, vector<int> params) {
	this->type = type;
	this->operands = operands;
	this->params = params;
}

Rule::~Rule() {
}

bool Rule::isOperator() const {
	return type < Rule::SPACING;
}

Tech::Tech() {
	boundary = -1;
	dbunit = 1.0;
	scale = 1.0;
}

Tech::~Tech() {
}

int Tech::findRule(int type, vector<int> operands) const {
	if (operands.empty()) {
		return std::numeric_limits<int>::max();
	}

	for (int i = 0; i < (int)rules.size(); i++) {
		if (rules[i].type == type and rules[i].operands.size() == operands.size()) {
			bool found = true;
			for (int j = 0; j < (int)operands.size() and found; j++) {
				found = (rules[i].operands[j] == operands[j]);
			}
			if (found) {
				return flip(i);
			}
		}
	}
	return std::numeric_limits<int>::max();
}

int Tech::setRule(int type, vector<int> operands) {
	int result = findRule(type, operands);
	if (result < 0) {
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(type, operands));
	for (auto l = operands.begin(); l != operands.end(); l++) {
		if (*l >= 0) {
			paint[*l].out.push_back(result);
		} else {
			rules[flip(*l)].out.push_back(result);
		}
	}
	return result;
}

int Tech::getOr(vector<int> layers) const {
	return findRule(Rule::OR, layers);
}

int Tech::setOr(vector<int> layers) {
	return setRule(Rule::OR, layers);
}

int Tech::getAnd(vector<int> layers) const {
	return findRule(Rule::AND, layers);
}

int Tech::setAnd(vector<int> layers) {
	return setRule(Rule::AND, layers);
}

int Tech::getNot(int l) const {
	return findRule(Rule::NOT, {l});
}

int Tech::setNot(int l) {
	return setRule(Rule::NOT, {l});
}

int Tech::getInteract(int l0, int l1) const {
	return findRule(Rule::INTERACT, {l0, l1});
}

int Tech::setInteract(int l0, int l1) {
	return setRule(Rule::INTERACT, {l0, l1});
}

int Tech::getNotInteract(int l0, int l1) const {
	return findRule(Rule::NOT_INTERACT, {l0, l1});
}

int Tech::setNotInteract(int l0, int l1) {
	return setRule(Rule::NOT_INTERACT, {l0, l1});
}

int Tech::getSpacing(int l0, int l1) const {
	int result = findRule(Rule::SPACING, {l0, l1});
	if (result < 0) {
		return rules[flip(result)].params[0];
	}
	return 0;
}

int Tech::setSpacing(int l0, int l1, int value) {
	int result = setRule(Rule::SPACING, {l0, l1});
	if (result != std::numeric_limits<int>::max()) {
		int idx = flip(result);
		if ((int)rules[idx].params.size() < 1) {
			rules[idx].params.resize(1, 0);
		}
		if (value > rules[idx].params[0]) {
			rules[idx].params[0] = value;
		}
	}
	return result;
}

vec2i Tech::getEnclosing(int l0, int l1) const {
	int result = findRule(Rule::ENCLOSING, {l0, l1});
	if (result < 0) {
		vector<int> params = rules[flip(result)].params;
		if ((int)params.size() == 1) {
			return vec2i(std::numeric_limits<int>::min(), params[0]);
		} else if ((int)params.size() >= 2) {
			return vec2i(params[0], params[1]);
		}
	}
	return vec2i(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
}

// -1 means that one of the sides doesn't need to be enclosed (extension)
int Tech::setEnclosing(int l0, int l1, int lo, int hi) {
	int result = setRule(Rule::ENCLOSING, {l0, l1});
	if (result != std::numeric_limits<int>::max()) {
		int idx = flip(result);
		if ((int)rules[idx].params.size() < 2) {
			rules[idx].params.resize(2, std::numeric_limits<int>::min());
		}
		if (lo > rules[idx].params[0]) {
			rules[idx].params[0] = lo;
		}
		if (hi > rules[idx].params[1]) {
			rules[idx].params[1] = hi;
		}
	}
	return result;
}

int Tech::getWidth(int l0) const {
	int result = findRule(Rule::WIDTH, {l0});
	if (result < 0) {
		return rules[flip(result)].params[0];
	}
	return 0;
}

int Tech::setWidth(int l0, int value) {
	int result = setRule(Rule::WIDTH, {l0});
	if (result != std::numeric_limits<int>::max()) {
		int idx = flip(result);
		if ((int)rules[idx].params.size() < 1) {
			rules[idx].params.resize(1, 0);
		}
		if (value > rules[idx].params[0]) {
			rules[idx].params[0] = value;
		}
	}
	return result;
}

string Tech::print(int layer) const {
	if (layer >= 0) {
		return paint[layer].name;
	}

	const Rule &rule = rules[flip(layer)];
	switch (rule.type) {
	case Rule::NOT: return string("~") + print(rule.operands[0]);
	case Rule::AND: return print(rule.operands[0]) + "&" + print(rule.operands[1]);
	case Rule::OR:  return string("(") + print(rule.operands[0]) + "|" + print(rule.operands[1]) + ")";
	case Rule::INTERACT: return "interact(" + print(rule.operands[0]) + "," + print(rule.operands[1]) + ")";
	case Rule::NOT_INTERACT: return "not_interact(" + print(rule.operands[0]) + "," + print(rule.operands[1]) + ")";
	case Rule::SPACING:  return print(rule.operands[0]) + "<->" + print(rule.operands[1]);
	case Rule::ENCLOSING:  return "enclosing(" + print(rule.operands[0]) + "," + print(rule.operands[1]) + ")";
	default: printf("%s:%d error: unsupported operation (rule[%d].type=%d).\n", __FILE__, __LINE__, flip(layer), rule.type);
	}
	return "";
}


int Tech::findPaint(string name) const {
	for (int i = 0; i < (int)paint.size(); i++) {
		if (paint[i].name == name) {
			return i;
		}
	}

	return -1;
}

int Tech::findPaint(int major, int minor) const {
	for (int i = 0; i < (int)paint.size(); i++) {
		if (paint[i].major == major and paint[i].minor == minor) {
			return i;
		}
	}

	return -1;
}

int Tech::findModel(string name) const {
	for (int i = 0; i < (int)models.size(); i++) {
		if (models[i].name == name) {
			return i;
		}
	}

	return -1;
}

int Tech::findModel(int type, string variant) const {
	for (int i = 0; i < (int)models.size(); i++) {
		if (models[i].type == type and models[i].variant == variant) {
			return i;
		}
	}

	return -1;
}

const Material *Tech::findMaterial(int layer) const {
	for (int j = 0; j < (int)subst.size(); j++) {
		if (subst[j].contains(layer)) {
			return &subst[j];
		}
	}

	for (int j = 0; j < (int)wires.size(); j++) {
		if (wires[j].contains(layer)) {
			return &wires[j];
		}
	}

	for (int j = 0; j < (int)vias.size(); j++) {
		if (vias[j].contains(layer)) {
			return &vias[j];
		}
	}
	return nullptr;
}

bool Tech::isRouting(int layer) const {
	for (auto wire = wires.begin(); wire != wires.end(); wire++) {
		if (layer == wire->draw) {
			return true;
		}
	}

	for (auto via = vias.begin(); via != vias.end(); via++) {
		if (layer == via->draw) {
			return true;
		}
	}

	return false;
}

bool Tech::isSubstrate(int layer) const {
	for (auto mat = subst.begin(); mat != subst.end(); mat++) {
		if (mat->contains(layer)) {
			return true;
		}
	}

	return false;
}

bool Tech::isPin(int layer) const {
	for (auto wire = wires.begin(); wire != wires.end(); wire++) {
		if (layer == wire->pin) {
			return true;
		}
	}

	for (auto via = vias.begin(); via != vias.end(); via++) {
		if (layer == via->pin) {
			return true;
		}
	}

	for (auto mat = subst.begin(); mat != subst.end(); mat++) {
		if (layer == mat->pin) {
			return true;
		}
	}

	return false;
}

bool Tech::isLabel(int layer) const {
	for (auto wire = wires.begin(); wire != wires.end(); wire++) {
		if (layer == wire->label) {
			return true;
		}
	}

	for (auto via = vias.begin(); via != vias.end(); via++) {
		if (layer == via->label) {
			return true;
		}
	}

	for (auto mat = subst.begin(); mat != subst.end(); mat++) {
		if (layer == mat->label) {
			return true;
		}
	}

	return false;
}

bool Tech::isWell(int layer) const {
	for (auto mat = subst.begin(); mat != subst.end(); mat++) {
		if (not mat->well.valid() and mat->contains(layer)) {
			return true;
		}
	}

	return false;
}

const Material &Tech::at(Level level) const {
	if (level.type == Level::SUBST) {
		return subst[level.idx];
	} else if (level.type == Level::VIA) {
		return vias[level.idx];
	}
	return wires[level.idx];
}

vector<int> Tech::via(Level down, Level up) const {
	Level curr = down;
	
	vector<int> result;
	bool done = false;
	while (not done) {
		done = true;
		for (int i = 0; curr != up and i < (int)vias.size(); i++) {
			if (vias[i].down == curr) {
				result.push_back(i);
				curr = vias[i].up;
				done = false;
			}
		}
	}
	return result;
}

}
