#include "semanticANLS.h"
#include "symbolTable.h"

#include <cassert>
#include <map>
#include <vector>
#include <iterator>

using namespace std;
using namespace pascompile;

int symbolTable::lookUpScope(program *id) {
	for(int i=0; i<scope.size(); ++i)
		if(scope.at(i).choice==1&&
			scope.at(i).type.type1->child1->child1->name==id->child1->child1->name) {
			return i;
	}
	return -1;
}

int symbolTable::lookUpScope(function_decl *id) {
	for(int i=0; i<scope.size(); ++i)
		if(scope.at(i).choice==2&&
			scope.at(i).type.type2->child1->child1->name==id->child1->child1->name) {
			return i;
}
	return -1;
}

int symbolTable::lookUpScope(procedure_decl *id) {
	for(int i=0; i<scope.size(); ++i)
		if(scope.at(i).choice==3&&
			scope.at(i).type.type3->child1->child1->name==id->child1->child1->name) {
			return i;
	}
	return -1;
}

bool symbolTable::insertID(pascompile::identifier *id, int s) {
	assert(s<0||s<scope.size());
	identifier *temp = lookUp(id, s);
	if(temp!=NULL&&temp->scope==s) {
		
		return false;
	}
	map<string,vector<pascompile::identifier *> >::iterator iter = table.find(id->name);
	if(iter==table.end()) {
		table.insert(pair<string,vector<pascompile::identifier *> >(id->name,vector<pascompile::identifier *>()));
		iter = table.find(id->name);
	}
	iter->second.push_back(id);
	id->scope = s;
	return true;
}

pascompile::identifier *symbolTable::lookUp(const pascompile::identifier * const id, int s, int &is) const {
	assert(s<scope.size());
	map<string,vector<pascompile::identifier *> >::const_iterator iter = table.find(id->name);
	if(iter==table.end()) {
		return NULL;
	}
	const vector<pascompile::identifier *> & ids = iter->second;
	while(s!=-1) {
		for(int i=0; i<ids.size(); ++i) {
			if(ids[i]->scope==s) {
				is = s;
				return ids[i];
			}
		}
		s = scope[s].parent;
	}
	return NULL;
}

pascompile::identifier * symbolTable::lookUp(const std::string &name, int s, int &is) const {
	assert(s<scope.size());
	map<string,vector<pascompile::identifier *> >::const_iterator iter = table.find(name);
	if(iter==table.end()) {
		return NULL;
	}
	const vector<pascompile::identifier *> & ids = iter->second;
	while(s!=-1) {
		for(int i=0; i<ids.size(); ++i) {
			if(ids[i]->scope==s) {
				is = s;
				return ids[i];
			}
		}
		s = scope[s].parent;
	}
	return NULL;
}

bool symbolTable::checkLabel() {
	set<int> diff;
	for(set<int>::iterator iter = usedlabel.begin(); iter!=usedlabel.end(); ++iter) 
		if(label.find(*iter)==label.end())
			diff.insert(*iter);
	for(set<int>::iterator iter = diff.begin(); iter!=diff.end(); ++iter) 
		printf("Label %d used but not defined.\n", *iter);
	return diff.empty();
}
void symbolTable::printScope() {
	for(int i=0; i<scope.size(); ++i) {
		printf("scope %d -> ", i);	
		scope[i].print();
	}
}

void symbolTable::printIdentifier() {
	for(map<string,vector<identifier *> >::iterator iter = table.begin(); iter!=table.end(); ++iter) {
		printf("id name : %s, occurred scope : ", iter->first.c_str());
		for(int i=0; i< iter->second.size(); ++i) 
			printf("%d ", iter->second[i]->scope);
		printf("\n");
	}
}

void symbolTable::printConst() {
	for(set<identifier *>::iterator iter = const_pool.begin(); iter!=const_pool.end(); ++iter) {
		printf("constant %s, ", (*iter)->name.c_str());
		switch((*iter)->stype) {
		case iinteger: printf("type integer, value %d", (*iter)->value.i); break;
		case rreal: printf("type real, value %f", (*iter)->value.f); break;
		case cchar: printf("type char, value %c", (*iter)->value.c); break;
		case sstring: printf("type string, value %s", (*iter)->value.s->c_str()); break;
		}
		printf("\n");
	}
}
