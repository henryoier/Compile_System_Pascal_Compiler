#include "semanticANLS.h"
#include "symbolTable.h"

#include <cstdio>
#include <iostream>
#include <sstream>

using namespace pascompile;
using namespace std;

int main(int argc, char *argv[]) {
	string tmp;
	stringstream ss;
	if(yyparse()) {
		cerr << endl;
		return 1;
	}

	program *root = savedRoot;
	if(!root->semanticCheck(gtable.insertScope())||!gtable.checkLabel()) {
		cerr << "semantic error!" << endl;
		return 1;
	}


	cout << "main:" << endl;
	cout << "addi $sp, $sp, -100" << endl;
	cout << "add $fp, $zero, $sp" << endl;

	// 为数组参数申请空间
	if (root->child2->child1->child4 == NULL){
		abort();
	}
	var_decl_list* vars = root->child2->child1->child4->child1;
	int pscope = 1;
	while (vars != NULL) {
		if (vars->child2->choice1 == 2) {//是数组
			if (vars->child2->child1.choice2->child1->choice1 != 4)//是数字区间
				abort();
			int arraySize = vars->child2->child1.choice2->child1->child1.choice4->size;

			name_list* array_list = vars->child1;
			while (array_list != NULL) {
				// 写数组地址
				identifier * targetID = gtable.lookUp(array_list->child2, pscope);
				if (targetID->type != aarray_type)
					abort();
				// 开栈
				ss << -1 * arraySize * 4;
				ss >> tmp;
				ss.clear();
				cout << "addi $sp, $sp, " + tmp << endl;
				ss << targetID->offset;
				ss >> tmp;
				ss.clear();
				cout << "sw $sp, " + tmp + "($fp)" << endl;

				array_list = array_list->child1;
			}
		}
		else
		{

		}

		vars = vars->next;
	}

	cout << root->codeGenerate() << endl; //代码生成！

	return 0;
}
