#include "semanticANLS.h"
#include "symbolTable.h"

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;
using namespace pascompile;

int compound_stmt::serial = 0;
char compound_stmt::buff[20];


symbolTable pascompile::gtable;
pascompile::program *pascompile::savedRoot;
bool pascompile::error = false;

void pascompile::stprint(SimpleType s) {
	switch(s) {
		case iinteger: printf("integer\n"); break;
		case rreal: printf("real\n"); break;
		case cchar: printf("char\n"); break;
		case sstring: printf("string\n"); break;
		case ssys_con: printf("sys_con\n"); break;
		case iidref: printf("idref\n"); break;
		case eenum: printf("enum\n"); break;
		case rrange: printf("range\n"); break;
		case iidrange: printf("idrange\n"); break;
		case bbool: printf("bool\n"); break;
		default: printf("error\n"); break;
	}
}

	bool program::semanticCheck(int scope) {
		bool temp1 = child1->semanticCheck(this, scope);
		bool temp2 = child2->semanticCheck(gtable.insertScope(this, scope));
		
		return temp1&&temp2;
	}

	bool program_head::semanticCheck(program *root, int scope) { 
		child1->constant = true; 
		child1->typedefinition = false;
		child1->type = pprogram;
		child1->value.prog = root;
		child1->scope = scope;
		child1->offset = 8;
		bool temp = gtable.insertID(child1, scope); 
		if(!temp)
			reportNameCollision(child1, scope);
		return temp;
	}

	bool routine::semanticCheck(int scope) { 
		bool temp1 = child1->semanticCheck(scope);
		bool temp2 = child2->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool sub_routine::semanticCheck(int scope) { 
		bool temp1 = child1->semanticCheck(scope);
		bool temp2 = child2->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool routine_head::semanticCheck(int scope) {
		bool temp1 = child1->semanticCheck(scope);
		bool temp2 = child2->semanticCheck(scope);
		bool temp3 = child3->semanticCheck(scope);
		bool temp4 = child4->semanticCheck(scope);
		bool temp5 = child5->semanticCheck(scope);
		return temp1&&temp2&&temp3&&temp4&&temp5;
	}
	
	bool const_part::semanticCheck(int scope) { 	
		return child1==NULL?true:child1->semanticCheck(scope); 
	}

	bool const_expr::semanticCheck(int scope) { 	
		child2->semanticCheck(child1, scope);
		bool temp1 = gtable.insertID(child1, scope); 
		if(!temp1)
			reportNameCollision(child1, scope);
		gtable.addConst(child1);
		bool temp2 = next==NULL?true:next->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool const_value::semanticCheck(identifier *id, int scope) {
		id->typedefinition = false;
		id->constant = true;
		id->type = ssimple_type;
		id->scope = scope;
		switch(choice1) {
		case 1: id->stype = iinteger; id->value.i = child1.choice1; break;
		case 2: id->stype = rreal; id->value.f = child1.choice2; break;
		case 3: id->stype = cchar; id->value.c = child1.choice3; break;
		case 4: id->stype = sstring; id->value.s = child1.choice4; break;
		case 5: id->stype = ssys_con; /*to be continued*/break;
		default: break;//error
		}
		return true;
	}
	bool record_type_decl::semanticCheck(identifier *id, int scope, int type){
		id->constant = false;
		id->type = rrecord_type;
		id->scope = scope;
		//id->value.record = this;

		return true;	
	}

	bool type_part::semanticCheck(int scope) { 
		return child1==NULL?true:child1->semanticCheck(scope); 
	}

	bool type_definition::semanticCheck(int scope) {
		child2->semanticCheck(child1, scope, 1); //it is right, don't change!
		if (child2->choice1 == 3){
			child1->value.record = child2->child1.choice3;	
		}
		
		bool temp1 = gtable.insertID(child1, scope);//1 stands for type definition
		if(!temp1)
			reportNameCollision(child1, scope);
		bool temp2 = next==NULL?true:next->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool type_decl::semanticCheck(identifier *id, int scope, int type) {//1 stands for type definition, 2stands for variable definition
		switch(choice1) {
		case 1:
		{
			identifier *array_name = NULL;
			bool mybool = child1.choice1->semanticCheck(id, scope, type, &choice1);
			if (choice1 == 2){
				//child1.choice2 = array_name->value.array;
				//cerr << array_name->type << array_name->stype << endl;
				array_name = child1.choice1->getArrayDecl(scope);
				child1.choice2 = array_name->value.array;

			}
			return mybool;
		}
		case 2: return child1.choice2->semanticCheck(id, scope, type);
		case 3: return child1.choice3->semanticCheck(id, scope, type);
		default: break;//error
		}
		return true;
	}
	identifier* simple_type_decl::getArrayDecl(int scope){
		return gtable.lookUp(child1.choice2, scope);
	}
	bool simple_type_decl::semanticCheck(identifier *id, int scope, int type, int *fatherchoice) {
		id->constant = id->typedefinition = type==1;
		id->scope = scope;
		id->type = ssimple_type;
		switch(choice1) {
		case 1: id->stype = child1.choice1->child1; /*cout << id->name << " type : " << id->stype << endl;*/ break;
		case 2: 
		{
			identifier *targetID = gtable.lookUp(child1.choice2, scope);
			if (targetID == NULL){
				cerr << "the NAME type is not in the gtable!" << endl;
			}
			else
			{
				//hereherehere
				if (targetID->type == aarray_type){
					*fatherchoice = 2;
					id->type = aarray_type;
					id->stype = targetID->stype;
				}
			}

			break;
		}
		case 3: //TODO
		case 4:
		case 5:
		default: break;//error
		}
		return true;
	}

	bool simple_type_decl::semanticCheck(identifier *id, int scope, int type) {
		id->constant = id->typedefinition = type==1;
		id->scope = scope;
		id->type = ssimple_type;
		switch(choice1) {
		case 1: id->stype = child1.choice1->child1; /*cout << id->name << " type : " << id->stype << endl;*/ break;
		case 2: 
		{
			identifier *targetID = gtable.lookUp(child1.choice2, scope);
			if (targetID == NULL){
				cerr << "the NAME type is not in the gtable!" << endl;
			}
			else
			{
				//hereherehere
				if (targetID->type == aarray_type){
					id->type = aarray_type;
					id->stype = targetID->stype;
				}
			}

			break;
		}
		case 3: //to be implemented
		case 4:
		case 5:
		default: break;//error
		}
		/*cout << id->name << " finished simple type" << endl;*/
		return true;
	}

	bool simple_type_decl::semanticCheck(name_list *ids, int scope, int type) {
		while(ids!=NULL) {
			semanticCheck(ids->child2, scope, type);
			ids = ids->child1;
		}
		return true;
	}

	bool type_decl::semanticCheck(name_list *ids, int scope, int type) {//1 stands for type definition, 2stands for variable definition
		bool temp1 = ids->child1==NULL?true:semanticCheck(ids->child1, scope, type);
		bool temp2 = semanticCheck(ids->child2, scope, type);
		return temp1&&temp2;
	}
/*********************************************/
	bool array_type_decl::semanticCheck(identifier *id, int scope, int type) {
		id->scope = scope;
		id->typedefinition = id->constant = type==1;
		id->type = aarray_type;
		
		id->stype = child2->child1.choice1->child1.choice1->child1;
		//bool temp = gtable.insertID(id, scope);
		//if(!temp)
		//	reportNameCollision(id, scope);
		assert(child1->choice1==4);
		assert(child2->choice1==1);
		bool temp2 = child1->child1.choice4->semanticCheck();

		id->value.array = this;
		return temp2;
	}
	
/*******************************/
	bool range::semanticCheck() {
		if(child1->choice1!=1||child2->choice1!=1) {
			printf("Error at line %d : can only use integer to index array.\n", lineno);
			return false;
		}
		size = child2->child1.choice1 - child1->child1.choice1 + 1;
		assert(size>0);
		return true;
	}

	bool name_list::semanticCheck(int scope) { 
		bool temp1 = child1==NULL?true:child1->semanticCheck(scope);
		scopeValue s = gtable.getScope(scope);
		switch(s.choice) {
		case 1: {
			program *pp = s.type.type1; 
			child2->offset = pp->child1->child1->offset; 
			pp->child1->child1->offset +=4; 
			break;
		}
		case 2: {
			function_decl *fp = s.type.type2; 
			child2->offset = fp->child1->child1->offset; 
			fp->child1->child1->offset+=4; 
			break;
		}
		case 3: {
			procedure_decl *rp = s.type.type3; 
			child2->offset = rp->child1->child1->offset; 
			rp->child1->child1->offset+=4; 
			break;
		}
		default: break; //error
		}
		bool temp2 = gtable.insertID(child2, scope);
		if(!temp2)
			reportNameCollision(child2, scope);
		return temp1&&temp2;
	}

	bool var_part::semanticCheck(int scope) { 
		return child1==NULL?true:child1->semanticCheck(scope); 
	}

	bool var_decl::semanticCheck(int scope) {
		child2->semanticCheck(child1, scope, 2);//2 stands for variable definition
		bool temp1 = child1->semanticCheck(scope);
		bool temp2 = next==NULL?true:next->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool routine_part::semanticCheck(int scope) {
		bool temp1 = child1==NULL?true:child1->nameCheck(scope);
		bool temp2 = child2==NULL?true:child2->nameCheck(scope);
		if(!temp1||!temp2)
			return false;
		bool temp3 = child1==NULL?true:child1->semanticCheck(scope);
		bool temp4 = child2==NULL?true:child2->semanticCheck(scope);
		return temp3&&temp4;
	}

	bool function_decl::nameCheck(int scope) { 
		bool temp1 = child1->semanticCheck(this, scope);
		bool temp2 = next==NULL?true:next->nameCheck(scope);
		//reversePosition(gtable.lookUpScope(this));
		return temp1&&temp2;
	}

	void function_decl::reversePosition(int scope) {
		int off = child1->child1->offset;
		const std::vector<std::string> *lst = child1->child1->paras;
		assert(lst!=NULL);
		for(int i=0; i< lst->size(); ++i) {
			identifier *id = gtable.lookUp(lst->at(i), scope);
			assert(id!=NULL);
			int no = (id->offset-8)/4+1;
			id->offset = off-no*4;
		}
		/*
		cout << "function " << child1->child1->name << " : size->" << child1->child1->offset << endl;
		for(int i=0; i< lst->size(); ++i) {
			identifier *id = gtable.lookUp(lst->at(i), scope);
			cout << "identifier " << id->name << " : offset->" << id->offset << endl;
		}
		*/
	}

	bool function_decl::semanticCheck(int scope) {
		int temp = gtable.lookUpScope(this); 		
		//printf("!!!%d\n", scope);
		//printf("!!!%d\n", temp);
		bool temp1 = child2->semanticCheck(temp);
		bool temp2 = next==NULL?true:next->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool function_head::semanticCheck(function_decl *root, int scope) {
		child1->scope = scope;
		child1->value.func = root;
		child3->semanticCheck(child1, scope, 2);
		child1->type = ffunction;
		child1->constant = false;
		child1->offset = 8;
		if(!gtable.insertID(child1, scope)) {
			reportNameCollision(child1, scope);
			return false;
		}
		int temp = gtable.insertScope(root, scope);
		return child2==NULL?true:child2->semanticCheck(child1, temp);
	}

	bool procedure_decl::nameCheck(int scope) { 
		bool temp1 = child1->semanticCheck(this, scope);
		bool temp2 = next==NULL?true:next->nameCheck(scope);
		//reversePosition(gtable.lookUpScope(this));
		return temp1&&temp2;
	}

	void procedure_decl::reversePosition(int scope) {
		int off = child1->child1->offset;
		const std::vector<std::string> *lst = child1->child1->paras;
		assert(lst!=NULL);
		for(int i=0; i< lst->size(); ++i) {
			identifier *id = gtable.lookUp(lst->at(i), scope);
			assert(id!=NULL);
			int no = (id->offset-8)/4+1;
			id->offset = off-no*4;
		}
		/*
		cout << "procedure " << child1->child1->name << " : size->" << child1->child1->offset << endl;
		for(int i=0; i< lst->size(); ++i) {
			identifier *id = gtable.lookUp(lst->at(i), scope);
			cout << "identifier " << id->name << " : offset->" << id->offset << endl;
		}
		*/
	}

	bool procedure_decl::semanticCheck(int scope) {
		int temp = gtable.lookUpScope(this); 		
		//printf("!!!%d\n", scope);
		//printf("!!!%d\n", temp);
		bool temp1 = child2->semanticCheck(temp);
		bool temp2 = next==NULL?true:next->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool procedure_head::semanticCheck(procedure_decl *root, int scope) {
		child1->scope = scope;
		child1->type = pprocedure;
		child1->value.proc = root;
		child1->constant = true;
		child1->typedefinition = false;
		child1->paras = NULL;
		child1->offset = 8;
		if(!gtable.insertID(child1, scope)) {
			reportNameCollision(child1, scope);
			return false;
		}
		int temp =  gtable.insertScope(root, scope);
		return child2==NULL?true:child2->semanticCheck(child1, temp);
	}

	bool parameters::semanticCheck(identifier *root, int scope) {
		if(child1==NULL) {
			root->paras = NULL;
			return true; 
		}
		root->paras = new std::vector<std::string>();
		return child1->semanticCheck(root, scope);
	}

	bool para_type_list::semanticCheck(identifier *root, int scope) {
		bool temp1, temp2, temp3;
		if(choice1==1) {
			addParameters(root, child1.choice1, 1);
			temp1 = child2->semanticCheck(child1.choice1, scope, 2);
			temp2 = child1.choice1->semanticCheck(scope);
		}
		else {
			addParameters(root, child1.choice2, 2);
			temp1 = child2->semanticCheck(child1.choice1, scope, 2);
			temp2 = child1.choice2->semanticCheck(scope);
		}
		temp3 = next==NULL?true:next->semanticCheck(root, scope);
		return temp1&&temp2&&temp3;
	}

	void para_type_list::addParameters(identifier *root, name_list *ids, int type) {//1 for passByReference
		if(ids->child1!=NULL)
			addParameters(root, ids->child1, type);
		root->paras->push_back(ids->child2->name);
		ids->child2->passByReference = type==1;
	}

	bool routine_body::semanticCheck(int scope) {
		return child1==NULL?true:child1->semanticCheck(scope);
	}

	bool compound_stmt::semanticCheck(int scope) {
		return child1==NULL?true:child1->semanticCheck(scope);
	}

	bool assign_stmt::semanticCheck(int scope) {
		identifier *id = gtable.lookUp(child1, scope);
		if(id==NULL) {
			reportNameMissing(child1, scope);
			return false;
		}
		if(id->constant) {
			fprintf(stderr, "Error at line %d : can't assign value to constant identifier '%s'.\n",
				id->lineno, id->name.c_str());
			return false;
		}
		bool temp;
		switch(choice2) {
		case 1:	{
			if(!child2.choice1->semanticCheck(scope))
				return false;
			temp = typeEqual(id, child2.choice1);
			if(!temp)
				reportTypeMismatch(child1->lineno, id, child2.choice1);
			return temp;
		}
		case 2: {
			bool temp1 = child2.choice2.child1->semanticCheck(scope);
			bool temp2 = child2.choice2.child2->semanticCheck(scope);
			if(!temp1||!temp2)
				return false;
			if(child2.choice2.child1->type!=ssimple_type||child2.choice2.child1->value.stype!=iinteger) {
				reportTypeMismatch(child1->lineno, child2.choice2.child1, iinteger);
				return false;
			}
			//for simplicity, if id is an array, assume id is an array of systype, and systype is stored in stype
			if(!typeEqual(id, child2.choice2.child2)) {//need judge id is an array
				cerr << "in assign_stmt, id and final exp mismatch" << endl;
				cerr << "id->type: " << id->type << endl;
				if (id->type==aarray_type){
					cerr << "id is arrary!" << endl;
				}
				cerr << "id->stype: " << id->stype << endl;
				if (id->stype == rreal){
					cerr << "array is REAL!" << endl;
				}
				reportTypeMismatch(child1->lineno, id, child2.choice2.child2);
				return false;
			}
			return temp1&&temp2;
		}
		case 3: {
			identifier *idd = gtable.lookUp(child2.choice3.child1, scope);
			child2.choice3.child2->semanticCheck(scope);
			if(idd==NULL) {
				reportNameMissing(child2.choice3.child1, scope);
				return false;
			}
			return true;
		}
		default: return false; //error
		}
	}

	bool proc_stmt::semanticCheck(int scope) {
		switch(choice1) {
		case 1: {
			identifier *id = gtable.lookUp(child1.choice1, scope);
			if(id==NULL) {
				reportNameMissing(child1.choice1, scope);
				return false;
			}
			return (id->type==ffunction||id->type==pprocedure)&&id->paras==NULL;
		}
		case 2: {
			identifier *id = gtable.lookUp(child1.choice2.child1, scope);
			if(id==NULL) {
				reportNameMissing(child1.choice2.child1, scope);
				return false;
			} 
			if(id->type!=ffunction&&id->type!=pprocedure) {
				reportTypeMismatch(child1.choice2.child1->lineno, id, ccallable);
				return false;
			}
			if(!child1.choice2.child2->semanticCheck(scope))
				return false;
			//return parametersMatch(id, child1.choice2.child2, child1.choice2.child1->lineno);
			//no arguments check
			return true;
		}
		case 3: //system procedure with no parameters
			return true;
		case 4:	// write writeln
			return child1.choice4.child2->semanticCheck(scope);
		case 5: {
			if(!child1.choice5->semanticCheck(scope))
				return false;
			if(child1.choice5->choice1!=1&&child1.choice5->choice1!=9) {
				printf("Error at line %d : expression inside read() is not assignable.\n", lineno);//!!!
				return false;
			}
			return true;
		}
		default: return false; // error
		}
	}

	bool if_stmt::semanticCheck(int scope) {
		bool temp1 = child1->semanticCheck(scope);
		bool temp2 = child2==NULL?true:child2->semanticCheck(scope);
		bool temp3 = child3==NULL?true:child3->semanticCheck(scope);
		return temp1&&temp2&&temp3;
	}

	bool else_clause::semanticCheck(int scope) {
		return choice1==0?true:child1->semanticCheck(scope);
	}

	bool repeat_stmt::semanticCheck(int scope) {
		bool temp1 = child1==NULL?true:child1->semanticCheck(scope);
		bool temp2 = child2==NULL?true:child2->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool while_stmt::semanticCheck(int scope) {
		bool temp1 = child1==NULL?true:child1->semanticCheck(scope);
		bool temp2 = child2==NULL?true:child2->semanticCheck(scope);
		return temp1&&temp2;
	}

/*****************************************/
	bool for_stmt::semanticCheck(int scope) {
		bool temp1 = gtable.lookUp(child1, scope)!=NULL;
		bool temp2 = child2==NULL?true:child2->semanticCheck(scope);
		bool temp3 = child4==NULL?true:child4->semanticCheck(scope);
		bool temp4 = child5==NULL?true:child5->semanticCheck(scope);
		if(!temp1)
			reportNameMissing(child1, scope);
		if(!temp2||!temp3) return false;
		temp2 = child2==NULL?true:
							(child2->type==ssimple_type&&(child2->value.stype==iinteger||child2->value.stype==cchar));
		temp3 = child4==NULL?true:
							(child4->type==ssimple_type&&(child4->value.stype==iinteger||child4->value.stype==cchar));
		return temp1&&temp2&&temp3&&temp4&&typeEqual(child2, child4);
	}

	bool case_stmt::semanticCheck(int scope) {
		bool temp1 = child1->semanticCheck(scope);
		bool temp2 = child2->semanticCheck(scope);
		return temp1&&temp2;
	}

	bool case_expr::semanticCheck(int scope) {
		bool temp1;
		switch(choice1) {
		case 1: 
			temp1 = true; 
			break;
		case 2: {
			identifier *id = gtable.lookUp(child1.choice2, scope);
			if(id==NULL) {
				temp1=false;
				reportNameMissing(child1.choice2, scope);
				break;
			}
			if(!id->constant) {
				fprintf(stderr, "Error at line %d : %s is not a constant value.\n", 
					id->lineno, id->name.c_str());
				temp1=false;
				break;
			}
			temp1 = true;
			break;
		}
		default: temp1=false; break;//error
		}
		bool temp2 = child2->semanticCheck(scope);
		bool temp3 = next==NULL?true:next->semanticCheck(scope);
		return temp1&&temp2&&temp3;
	}

	bool expression::semanticCheck(int scope) {
		bool temp1 = child1==NULL?true:child1->semanticCheck(scope);
		bool temp2 = child3->semanticCheck(scope);
		bool temp3 = next==NULL?true:next->semanticCheck(scope);
		
		if(!temp1||!temp2)
			return false;
		
		if(child1!=NULL&&!typeEqual(child1, child3)) {
			//reportTypeMismatch(child1, child3);
			return false;
		}

		type = child3->type;
		value = child3->value;
			//printf("expression");
			//stprint(value.stype);
		return temp3;
	}

	bool expr::semanticCheck(int scope) {
		bool temp1 = child1==NULL?true:child1->semanticCheck(scope);
		bool temp2 = child3->semanticCheck(scope);
		if(!temp1||!temp2)
			return false;
		// no type check
		type = child3->type;
		value = child3->value;
			//printf("expr");
			//stprint(value.stype);
		return true;
	}

	bool term::semanticCheck(int scope) {
		bool temp1 = child1==NULL?true:child1->semanticCheck(scope);
		bool temp2 = child3->semanticCheck(scope);
		if(!temp1||!temp2)
			return false;
		// no type check
		type = child3->type;
		value = child3->value;
		//	printf("term");
		//	stprint(value.stype);
		return true;
	}

/**************************************/
	bool factor::semanticCheck(int scope) {
		switch(choice1) {
		case 1: {
			identifier *id = gtable.lookUp(child1.choice1, scope);
			if(id==NULL) {
				reportNameMissing(child1.choice1, scope);
				return false;
			}
			//no identifier type (proc func or var) check 
			type = id->type; 
			value.stype = id->stype;
			return true;
		}
		case 2: {
			identifier *id = gtable.lookUp(child1.choice2.child1, scope);
			if(id==NULL) {
				reportNameMissing(child1.choice2.child1, scope);
				return false;
			}
			//no identifier type (proc func or var) check 
			type = id->type; 
			value.stype = id->stype;
			return child1.choice2.child2->semanticCheck(scope);		
		}	
		case 3: return true;//to be implemented
		case 4: return child1.choice4.child2->semanticCheck(scope); //to beimplemented
		case 5: {
			type = ssimple_type;
			switch(child1.choice5->choice1) {
			case 1: value.stype = iinteger; break;
			case 2: value.stype = rreal; break;
			case 3: value.stype = cchar; break;
			case 4: value.stype = sstring; break;
			default: break; //error
			}
			//printf("factor");
			//stprint(value.stype);
			return true;
		}
		case 6:
			if(child1.choice6->semanticCheck(scope)) {
				type = child1.choice6->type;
				value = child1.choice6->value;
				return true;
			}
			return false;
		case 7://not feasible
			if(child1.choice7->semanticCheck(scope)) {
				type = child1.choice6->type;
				value = child1.choice6->value;
				return true;
			}
			return false;
		case 8:
			if(child1.choice8->semanticCheck(scope)) {
				type = child1.choice6->type;
				value = child1.choice6->value;
				return true;
			}
			return false;
		case 9: {
			identifier *id = gtable.lookUp(child1.choice9.child1, scope);
			if(id==NULL) {
				reportNameMissing(id, scope);
				return false;
			}
			if(id->type!=aarray_type) {//看ID是否为数组
				printf("Error at line %d : %s is not array type.\n", child1.choice9.child1->lineno, child1.choice9.child1->name.c_str());
				return false;
			}
			if(!child1.choice9.child2->semanticCheck(scope))//看expression semanticCheck
				return false;
			if(child1.choice9.child2->type!=ssimple_type||child1.choice9.child2->value.stype!=iinteger) {//看[]内是否为integer
				printf("Error at line %d : expression inside '[]' is not an integer.\n", child1.choice9.child2->lineno);//!!!
				return false;
			}
			//以下设置value
			value.stype = id->stype;
			return true;
		}
		case 10: return true;//to be implemented
		default: return false; //error
		}
	}

	bool stmt::semanticCheck(int scope) {
		bool temp0 = choice1==0?true:gtable.addLabel(child1);

		if(!temp0)
			fprintf(stderr, "Error : label %d redefined.\n", child1);
		bool temp1 = child2->semanticCheck(scope);
		bool temp2 = next==NULL?true:next->semanticCheck(scope);
		return temp0&&temp1&&temp2;
	}

	bool non_label_stmt::semanticCheck(int scope) {
		switch(choice1) {
		case 1: return child1.choice1->semanticCheck(scope);
		case 2: return child1.choice2->semanticCheck(scope);
		case 3: return child1.choice3->semanticCheck(scope);
		case 4: return child1.choice4->semanticCheck(scope);
		case 5: return child1.choice5->semanticCheck(scope);
		case 6: return child1.choice6->semanticCheck(scope);
		case 7: return child1.choice7->semanticCheck(scope);
		case 8: return child1.choice8->semanticCheck(scope);
		case 9: return child1.choice9->semanticCheck(scope);
		default: return false;//error
		}
	}

	bool goto_stmt::semanticCheck(int scope) {
		gtable.addUsedLabel(child1);
		return true;
	}




