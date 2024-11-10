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

Material::Material() {
	draw = -1;
	label = -1;
	pin = -1;
}

Material::Material(int draw, int label, int pin) {
	this->draw = draw;
	this->pin = pin;
	this->label = label;
}

Material::~Material() {
}

bool Material::contains(int layer) const {
	return layer == draw or layer == label or layer == pin;
}

int Material::size() const {
	return 3;
}

int Material::at(int idx) const {
	array<int, 3> arr({draw, pin, label});
	return arr[idx];
}

Diffusion::Diffusion() : Material() {
	isWell = false;
}

Diffusion::Diffusion(int draw, int label, int pin, bool isWell) : Material(draw, label, pin) {
	this->isWell = isWell;
}

Diffusion::~Diffusion() {
}

Model::Model() {
	variant = "";
	name = "";
	type = NMOS;
}

Model::Model(int type, string variant, string name, vector<int> stack, vector<int> excl, vector<pair<int, int> > bins) {
	this->variant = variant;
	this->name = name;
	this->type = type;
	this->stack = stack;
	this->excl = excl;
	this->bins = bins;
	std::sort(this->bins.begin(), this->bins.end());
}

Model::~Model() {
}

Routing::Routing() : Material() {
}

Routing::Routing(int draw, int label, int pin) : Material(draw, label, pin) {
}

Routing::~Routing() {
}

Via::Via() : Material() {
	upLevel = 0;
	downLevel = 0;
}

Via::Via(int downLevel, int upLevel, int draw, int label, int pin) : Material(draw, label, pin) {
	this->downLevel = downLevel;
	this->upLevel = upLevel;
}

Via::~Via() {
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

int Tech::ruleIdx(int type, int l0) const {
	if (l0 >= (int)paint.size()) {
		return std::numeric_limits<int>::max();
	}

	for (int i = 0; i < (int)rules.size(); i++) {
		if (rules[i].type == type and (int)rules[i].operands.size() == 1 and rules[i].operands[0] == l0) {
			return flip(i);
		}
	}
	return std::numeric_limits<int>::max();
}

int Tech::ruleIdx(int type, int l0, int l1) const {
	if (l0 >= (int)paint.size() or l1 >= (int)paint.size()) {
		return std::numeric_limits<int>::max();
	}

	for (int i = 0; i < (int)rules.size(); i++) {
		if (rules[i].type == type and (int)rules[i].operands.size() == 2 and rules[i].operands[0] == l0 and rules[i].operands[1] == l1) {
			return flip(i);
		}
	}
	return std::numeric_limits<int>::max();
}

int Tech::getOr(int l0, int l1) const {
	return ruleIdx(Rule::OR, l0, l1);
}

int Tech::setOr(int l0, int l1) {
	int result = ruleIdx(Rule::OR, l0, l1);
	if (result < 0) {
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::OR, {l0, l1}));
	if (l0 >= 0) {
		paint[l0].out.push_back(result);
	} else {
		rules[flip(l0)].out.push_back(result);
	}
	if (l1 >= 0) {
		paint[l1].out.push_back(result);
	} else {
		rules[flip(l1)].out.push_back(result);
	}
	return result;
}

int Tech::getAnd(int l0, int l1) const {
	return ruleIdx(Rule::AND, l0, l1);
}

int Tech::setAnd(int l0, int l1) {
	int result = ruleIdx(Rule::AND, l0, l1);
	if (result < 0) {
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::AND, {l0, l1}));
	if (l0 >= 0) {
		paint[l0].out.push_back(result);
	} else {
		rules[flip(l0)].out.push_back(result);
	}
	if (l1 >= 0) {
		paint[l1].out.push_back(result);
	} else {
		rules[flip(l1)].out.push_back(result);
	}
	return result;
}

int Tech::getNot(int l) const {
	return ruleIdx(Rule::NOT, l);
}

int Tech::setNot(int l) {
	int result = ruleIdx(Rule::NOT, l);
	if (result < 0) {
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::NOT, {l}));
	if (l >= 0) {
		paint[l].out.push_back(result);
	} else {
		rules[flip(l)].out.push_back(result);
	}
	return result;
}

int Tech::getInteract(int l0, int l1) const {
	return ruleIdx(Rule::INTERACT, l0, l1);
}

int Tech::setInteract(int l0, int l1) {
	int result = ruleIdx(Rule::INTERACT, l0, l1);
	if (result < 0) {
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::INTERACT, {l0, l1}));
	if (l0 >= 0) {
		paint[l0].out.push_back(result);
	} else {
		rules[flip(l0)].out.push_back(result);
	}
	if (l1 >= 0) {
		paint[l1].out.push_back(result);
	} else {
		rules[flip(l1)].out.push_back(result);
	}
	return result;
}

int Tech::getNotInteract(int l0, int l1) const {
	return ruleIdx(Rule::NOT_INTERACT, l0, l1);
}

int Tech::setNotInteract(int l0, int l1) {
	int result = ruleIdx(Rule::NOT_INTERACT, l0, l1);
	if (result < 0) {
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::NOT_INTERACT, {l0, l1}));
	if (l0 >= 0) {
		paint[l0].out.push_back(result);
	} else {
		rules[flip(l0)].out.push_back(result);
	}
	if (l1 >= 0) {
		paint[l1].out.push_back(result);
	} else {
		rules[flip(l1)].out.push_back(result);
	}
	return result;
}

int Tech::getSpacing(int l0, int l1) const {
	int result = ruleIdx(Rule::SPACING, l0, l1);
	if (result < 0) {
		return rules[flip(result)].params[0];
	}
	return 0;
}

int Tech::setSpacing(int l0, int l1, int value) {
	int result = ruleIdx(Rule::SPACING, l0, l1);
	if (result != std::numeric_limits<int>::max()) {
		int idx = flip(result);
		if (value < rules[idx].params[0]) {
			rules[idx].params[0] = value;
		}
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::SPACING, {l0, l1}, {value}));
	if (l0 >= 0) {
		paint[l0].out.push_back(result);
	} else {
		rules[flip(l0)].out.push_back(result);
	}
	if (l1 >= 0) {
		paint[l1].out.push_back(result);
	} else {
		rules[flip(l1)].out.push_back(result);
	}
	return result;
}

vec2i Tech::getEnclosing(int l0, int l1) const {
	int result = ruleIdx(Rule::ENCLOSING, l0, l1);
	if (result < 0) {
		vector<int> params = rules[flip(result)].params;
		if ((int)params.size() == 1) {
			return vec2i(-1, params[0]);
		} else if ((int)params.size() >= 2) {
			return vec2i(params[0], params[1]);
		}
	}
	return vec2i(-1, -1);
}

// -1 means that one of the sides doesn't need to be enclosed (extension)
int Tech::setEnclosing(int l0, int l1, int lo, int hi) {
	int result = ruleIdx(Rule::ENCLOSING, l0, l1);
	if (result != std::numeric_limits<int>::max()) {
		int idx = flip(result);
		if (rules[idx].params.empty()) {
			rules[idx].params.push_back(lo);
		} else if (lo < rules[idx].params[0]) {
			rules[idx].params[0] = lo;
		}
		if ((int)rules[idx].params.size() < 2) {
			rules[idx].params.push_back(hi);
		} else if (hi < rules[idx].params[1]) {
			rules[idx].params[1] = hi;
		}
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::ENCLOSING, {l0, l1}, {lo, hi}));
	if (l0 >= 0) {
		paint[l0].out.push_back(result);
	} else {
		rules[flip(l0)].out.push_back(result);
	}
	if (l1 >= 0) {
		paint[l1].out.push_back(result);
	} else {
		rules[flip(l1)].out.push_back(result);
	}
	return result;
}

int Tech::getWidth(int l0) const {
	int result = ruleIdx(Rule::WIDTH, l0);
	if (result < 0) {
		return rules[flip(result)].params[0];
	}
	return 0;
}

int Tech::setWidth(int l0, int value) {
	int result = ruleIdx(Rule::WIDTH, l0);
	if (result != std::numeric_limits<int>::max()) {
		int idx = flip(result);
		if (value < rules[idx].params[0]) {
			rules[idx].params[0] = value;
		}
		return result;
	}

	result = flip((int)rules.size());
	rules.push_back(Rule(Rule::WIDTH, {l0}, {value}));
	if (l0 >= 0) {
		paint[l0].out.push_back(result);
	} else {
		rules[flip(l0)].out.push_back(result);
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

const Material *Tech::atMaterial(int level) const {
	if (level < 0) {
		return &subst[models[flip(level)].stack[0]];
	}
	return &wires[level];
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

vector<int> Tech::findVias(int downLevel, int upLevel) const {
	int curr = downLevel;
	
	vector<int> result;
	bool done = false;
	while (not done) {
		done = true;
		for (int i = 0; curr != upLevel and i < (int)vias.size(); i++) {
			if (vias[i].downLevel == curr) {
				result.push_back(i);
				curr = vias[i].upLevel;
				done = false;
			}
		}
	}
	return result;
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
		if (layer == mat->draw) {
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
		if (mat->isWell and mat->contains(layer)) {
			return true;
		}
	}

	return false;
}


}
