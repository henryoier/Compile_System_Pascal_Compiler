#ifndef _pascompile_SYMBOLTABLE_H_
#define _pascompile_SYMBOLTABLE_H_

#include "semanticANLS.h"

#include <cassert>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstdio>
#include <iostream>


namespace pascompile {

struct scopeValue{
	int parent;//-1 stands for root
	int choice;//0=universal, only valid for the program name in pascal
	union {
		pascompile::program *type1;
		pascompile::function_decl *type2;
		pascompile::procedure_decl *type3;
	} type;
	scopeValue(int p, pascompile::program *t):parent(p),choice(1) { type.type1 = t; }
	scopeValue(int p, pascompile::function_decl *t):parent(p),choice(2) { type.type2 = t; }
	scopeValue(int p, pascompile::procedure_decl *t):parent(p),choice(3) { type.type3 = t; }
	scopeValue(int p):parent(p),choice(0) { }
	scopeValue(int p, const std::string &s): parent(p), choice(0) { }
	void print() const { 
		printf("parent : %d, type : ", parent);
		switch(choice) {
		case 0: printf("universal"); break;
		case 1: printf("program, name : %s", type.type1->child1->child1->name.c_str()); break;
		case 2: printf("function, name : %s", type.type2->child1->child1->name.c_str()); break;
		case 3: printf("procedure, name : %s", type.type3->child1->child1->name.c_str()); break;
		case 4: printf("error choice"); break;
		}
		printf("\n");
	}
};

class symbolTable {
private :
	std::vector<scopeValue> scope;
	std::map<std::string, std::vector<pascompile::identifier *> > table;
	std::set<int> label;
	std::set<int> usedlabel;
	std::set<identifier *> const_pool;
public :
	void printScope();
	void printIdentifier();
	void printConst();
	void addConst(pascompile::identifier *id) { const_pool.insert(id); }
	std::set<pascompile::identifier *> &getConstPoll() { return const_pool; }

	int insertScope() {//global scope

		scope.push_back(scopeValue(-1, "global"));
		return scope.size()-1;
	}

	int insertScope(pascompile::program *id, int parent_scope) {
		if(lookUpScope(id)!=-1) {
			return -1;
		}
		scope.push_back(scopeValue(parent_scope, id));
		return scope.size()-1;
	}
	int insertScope(pascompile::function_decl *id, int parent_scope) {
		if(lookUpScope(id)!=-1) {
			return -1;
		}
		scope.push_back(scopeValue(parent_scope, id));
		return scope.size()-1;
	}
	int insertScope(pascompile::procedure_decl *id, int parent_scope) {
		if(lookUpScope(id)!=-1) {
			return -1;
		}
		scope.push_back(scopeValue(parent_scope, id));
		return scope.size()-1;
	}

	int lookUpScope(pascompile::program *id);
	int lookUpScope(pascompile::function_decl *id);
	int lookUpScope(pascompile::procedure_decl *id);


	scopeValue getScope(int s) const { 
		assert(s<scope.size()); 
		return scope.at(s); 
	}
	

	bool insertID(pascompile::identifier *id, int scope);
	
	pascompile::identifier *lookUp(const pascompile::identifier * const id, int s, int &is) const ;
	
	pascompile::identifier *lookUp(const std::string &id, int s, int &is) const;

	pascompile::identifier *lookUp(const pascompile::identifier * const id, int s) const {
		return lookUp(id, s, s);
	}

	pascompile::identifier *lookUp(const std::string &id, int s) const {
		int temp;
		return lookUp(id, s, temp);
	}

	bool addLabel(int l) { 
		label.insert(l);
		return true;
	}
	bool addUsedLabel(int l) {
		usedlabel.insert(l);
		return true;
	}
	bool checkLabel();
};

extern symbolTable gtable;

}

#endif
