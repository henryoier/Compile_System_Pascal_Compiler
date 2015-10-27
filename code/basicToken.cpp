#include "semanticANLS.h"
#include "symbolTable.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>

using namespace std;
using namespace pascompile;

extern symbolTable gtable;
extern int uniqueNum;
extern int currentScope;


string factor::codeGenerate() {
	string mips, tmp, label, offset, recover;
	stringstream ss;
	switch (choice1) {
		case 1: //id
		{
			//cerr << "in factor, before lookUp,should be simple id: " << child1.choice1->name << endl;
			identifier *targetID = gtable.lookUp(child1.choice1, currentScope);
			//cerr << "in factor, after lookUp,should be simple id: " << targetID->name << endl;
			//cerr<<"id:"<<targetID->name<<" id.stype:"<<targetID->stype<<endl;
			int scopePtr = currentScope;
			mips += "add $t0, $zero, $fp #先找出ID的地址\n";
			while (scopePtr != targetID->scope) {
				mips += "lw $t0, 4($t0)\n";
				scopePtr = gtable.getScope(scopePtr).parent;
			}

			if(targetID->constant){
				if(value.stype == iinteger){
					ss << targetID->value.i;
					ss >> tmp;
					ss.clear();
					mips += "addi $v0, $zero, " + tmp +" #取出id当前值到$v0中\n";
				}
				else
				{
					uniqueNum++;
					ss << "floatNum";
					ss << uniqueNum;
					ss >> label;
					ss.clear();
					ss << targetID->value.f;
					ss >> tmp;
					ss.clear();

					mips += ".data\n";
					mips += label + ":\n";
					mips +=".float "+ tmp +"\n";
					mips +=".text\n";
					mips += "lwc1 $f0, "+ label + " #取出id当前值到$f0中\n";
				}
			}
			else
			{
				ss << targetID->offset;
				ss >> offset;
				ss.clear();
	
				if(value.stype == iinteger){
					mips += "lw $v0, " + offset + "($t0) #取出id当前值到$v0中\n";
				}
				else
				{
					mips += "lwc1 $f0, " + offset + "($t0) #取出id当前值到$f0中\n";
				}
			}
			break;
		}
		case 2: //function
		{
			// 参数压栈
			int i, funcSize, argsNum, scopePtr;
			//identifier * targetFunc = gtable.lookUp(child1.choice1, currentScope);
			identifier * targetFunc = gtable.lookUp(child1.choice2.child1, currentScope);
			if (targetFunc->type != ffunction || targetFunc->paras == NULL)
				abort();
			funcSize = targetFunc->offset;
			argsNum = targetFunc->paras->size();

			scopePtr = -1;
			switch (targetFunc->type){
				case ffunction:
				{
					scopePtr = gtable.lookUpScope(targetFunc->value.func);
					break;
				}
				case pprocedure:
				{
					scopePtr = gtable.lookUpScope(targetFunc->value.proc);
					break;
				}
				default:
					break;
			}
			if (scopePtr == -1)
				abort(); //调用了main

			args_list* a_list = child1.choice2.child2;
			for (i = 0; i < argsNum; i++) {
				mips += a_list->codeGenerate();
				ss << funcSize*-1 + 8 + i*4;
				ss >> tmp;
				ss.clear();
				mips += "sw $v0, " + tmp + "($sp) #把参数压入栈\n";

				identifier* argID = gtable.lookUp(targetFunc->paras->at(i), scopePtr);
				if (argID == NULL)
					abort();
				if (argID->passByReference){ //引用的变量要把参数最后的值写回去
					if (a_list->child3->child3->child3->choice1 != 1)
						abort();
					recover += "lw $v1, " + tmp + "($sp) #把参数最后的值取出来\n";

					int scopePtr2 = currentScope;
					identifier* argID2 = gtable.lookUp(a_list->child3->child3->child3->child1.choice1, currentScope);
					if (argID2 == NULL)
						abort();
					recover += "add $t0, $zero, $fp #先找出ID的地址\n";
					while (scopePtr2 != argID2->scope) {
						recover += "lw $t0, 4($t0)\n";
						scopePtr2 = gtable.getScope(scopePtr2).parent;
					}
					ss << argID2->offset;
					ss >> tmp;
					ss.clear();

					recover += "sw $v1, " + tmp + "($t0) #把参数最后的值写入引用的变量中\n";
				}

				a_list = a_list->next;
			}

			// 获取 access link -> $t0
			scopePtr = currentScope;
			mips += "add $t0, $zero, $fp #先找出调用的函数的地址\n";
			while (scopePtr != targetFunc->scope) {
				mips += "lw $t0, 4($t0)\n";
				scopePtr = gtable.getScope(scopePtr).parent;
			}
			ss << funcSize*-1 + 4;
			ss >> tmp;
			ss.clear();
			mips += "sw $t0, " + tmp + "($sp) #把调用的函数的地址压入栈 access link\n";

			ss << funcSize*-1;
			ss >> tmp;
			ss.clear();
			mips += "addi $sp, $sp, " + tmp + " #移动栈指针\n";
			mips += "add $t7, $zero, $fp\n";		// 保存 $fp for control link
			mips += "add $fp, $zero, $sp\n";

			// 开栈（control link）
			mips += "addi $sp, $sp, -4\n";
			mips += "sw $t7, 0($sp) #把$fp的值压入栈\n";

			// 为数组参数申请空间    代码有问题 wtfwtf！$sp没有复原！！！
			assert(targetFunc->value.func->child2->child1->child4 != NULL);	// var_part
			var_decl_list * vars = targetFunc->value.func->child2->child1->child4->child1;
			// procedure scope
			int pscope = gtable.lookUpScope(targetFunc->value.func);
			while (vars != NULL) {
				if (vars->child2->choice1 == 2) {	// 数组
					assert(vars->child2->child1.choice2->child1->choice1 == 4);	// range
					int arraySize = vars->child2->child1.choice2->child1->child1.choice4->size;

					name_list * varName = vars->child1;
					while (varName != NULL) {
						// 写数组地址
						identifier * newvariden = gtable.lookUp(varName->child2, pscope);
						assert(newvariden->type == aarray_type);
						// 开栈
						ss << arraySize*-4;
						ss >> tmp;
						ss.clear();
						mips += "addi $sp, $sp, " + tmp + "\n";
						ss << newvariden->offset;
						ss >> tmp;
						ss.clear();
						mips += "sw $sp, " + tmp + "($fp)\n";

						varName = varName->child1;
					}
				}
				else {}

				vars = vars->next;
			}

			ss << gtable.lookUp(child1.choice2.child1, currentScope)->scope; //找function地址
			ss >> tmp;
			ss.clear();
			mips += "jal " + child1.choice2.child1->name + tmp + " #调用函数\n";

			mips += "lw $fp, -4($fp)\n";
			mips += "addi $sp, $sp, 4 #弹出control link\n";
			ss << funcSize;
			ss >> tmp;
			ss.clear();
			mips += "addi $sp, $sp, " + tmp + " #弹出access link和所有参数\n";
			mips += recover;
			break;
		}
		case 3:
		{	// sys_func, no params
			break;
		}
		case 4:
		{	// sys_func, with params
			break;
		}
		case 5:
		{	// 常数
			switch (child1.choice5->choice1) {
				case 1: //int
				{
					ss << child1.choice5->child1.choice1;
					ss >> tmp;
					ss.clear();
					mips += "addi $v0, $zero, " + tmp + " #常量的值赋给$v0中\n";
					break;
				}
				case 2: //float
				{
					uniqueNum++;
					ss << "floatNum";
					ss << uniqueNum;
					ss >> label;
					ss.clear();
					ss << child1.choice5->child1.choice2;
					ss >> tmp;
					ss.clear();

					mips += ".data\n";
					mips += label + ":\n";
					mips +=".float " + tmp + "\n";

					mips +=".text\n";
					mips += "lwc1 $f0, " + label + " #常量的值赋给$f0中\n";

					break;
				}
				case 3: //char
				{
					ss << child1.choice5->child1.choice3;
					ss >> tmp;
					ss.clear();
					mips += "addi $v0, $zero, '" + tmp + "'\n";
					break;
				}
				default:
					break;
			}
			break;
		}
		case 6: //expression
		{
			mips += child1.choice6->codeGenerate();
			break;
		}
		case 7: //not factor
		{
			mips += child1.choice7->codeGenerate();
			if (value.stype == iinteger){
				mips += "not $v0, $v0 #not factor\n";
			}
			else
			{
				//error?
			}
			break;
		}
		case 8: //minus factor
		{
			mips += child1.choice8->codeGenerate();
			if (value.stype == iinteger){
				mips += "sub $v0, $zero, $v0 #minus factor\n";
			}
			else
			{
				mips += "sub.s $f0, $f31, $f0 #minus factor\n";
			}
			break;
		}
		case 9: //array
		{
			mips += child1.choice9.child2->codeGenerate();
			mips += "#数组下标存到$v0中\n";

			identifier* targetID = gtable.lookUp(child1.choice9.child1, currentScope);
			if (targetID == NULL)
				abort();
			int scopePtr = currentScope;
			mips += "add $t0, $zero, $fp #先找出ID的地址\n";
			while (scopePtr != targetID->scope) {
				mips += "lw $t0, 4($t0)\n";
				scopePtr = gtable.getScope(scopePtr).parent;
			}
			ss << targetID->offset;
			ss >> tmp;
			ss.clear();
			mips += "lw $t0, " + tmp + "($t0)\n";

			mips += "sll $v0, $v0, 2 #下标乘以4\n";
			mips += "add $t0, $t0, $v0 #取出数组元素地址\n";
			if (value.stype == rreal){
				mips += "lwc1 $f0, 0($t0) #数组元素的值存到$f0中\n";
			}
			else
			{
				mips += "lw  $v0, 0($t0) #数组元素的值存到$v0中\n";	
			}

			break;
		}
		case 10:
		{
			identifier *targetID = gtable.lookUp(child1.choice10.child1, currentScope);


			int scopePtr = currentScope;
			mips += "add $t0, $zero, $fp #先找出ID的地址\n";
			while (scopePtr != targetID->scope) {
				mips += "lw $t0, 4($t0)\n";
				scopePtr = gtable.getScope(scopePtr).parent;
			}
			field_decl_list *my_list = targetID->value.record->child1;
			identifier *targetID2 = NULL;
			int found = 0;
			while (my_list != NULL){
				name_list *tmp_name_list = my_list->child1;
				while (tmp_name_list != NULL){
					identifier *tmp_name = tmp_name_list->child2;
					if(child1.choice10.child2->name.compare(tmp_name->name) == 0){
						found = 1;
						targetID2 = tmp_name;
						//targetID2 = gtable.lookUp(tmp_name, currentScope);
						break;
					}

					tmp_name_list = tmp_name_list->child1;
				}
				if (found == 1)
					break;
				my_list = my_list->next;
			}
			switch(my_list->child2->choice1){
				case 1:
				{
					switch (my_list->child2->child1.choice1->choice1){
						case 1:
						{
							switch (my_list->child2->child1.choice1->child1.choice1->child1){
								case iinteger:{
									ss << targetID2->offset;
									ss >> offset;
									ss.clear();

									mips += "lw $v0, " + offset + "($t0) #取出id当前值到$v0中\n";
									
									break;
								}
								default:
									break;
							}

							break;
						}
						default:
							break;
					}

					break;
				}
				case 2:
				{

				}
				case 3:
				{

				}
				default:
					break;
			}

			break;
		}
		default:
			break;	// Error
	}

	return mips;
}

string term::codeGenerate(){
	string mips;

	if (child1 != NULL) {
		mips += child1->codeGenerate();
		
		if(value.stype == iinteger){
			mips += "addi $sp, $sp, -4\n";
			mips += "sw $v0, 0($sp) #把第一个操作数压入栈\n";
			mips += child3->codeGenerate();
			mips += "lw $v1, 0($sp) #把第一个操作数取出来到$v1中\n";
			switch(child2){
				case mmul:
				{
					mips += "mult $v1, $v0\n";
					mips += "mflo $v0 #将$lo寄存器中的值取到$v0中\n";
					break;
				}
				case ddiv:
					mips += "div $v1, $v0\n";
					mips += "mflo $v0 #将$lo寄存器中的值取到$v0中\n";
					break;
				case mmod:
					mips += "div $v0, $v1, $v0\n";
					mips += "mfhi $v0 #将$hi寄存器中的值取到$v0中\n";
					break;
				case aand:
					mips += "and $v0, $v1, $v0\n";
					break;
				default:
					break;
			}
			
		}
		else
		{	
			mips += "addi $sp, $sp, -4\n";
			mips += "swc1 $f0, 0($sp) #把第一个操作数压入栈\n";
			mips += child3->codeGenerate();
			mips += "lwc1 $f1, 0($sp) #把第一个操作数取出来到$f1中\n";
			switch(child2){
				case mmul:
				{
					mips += "mul.s $f0, $f1, $f0\n";
					break;
				}
				case ddiv:
				{
					mips += "div.s $f0, $f1, $f0\n";
					break;
				}
				default:
					break;
			}

		}

		mips = mips + "addi $sp, $sp, 4 #弹栈\n";
	}
	else
	{
		mips += child3->codeGenerate();
	}

	return mips;
}

string expr::codeGenerate(){
	string mips;

	if(child1 != NULL) {
		mips += child1->codeGenerate();
		
		if(value.stype == iinteger){
			mips += "addi $sp, $sp, -4\n";
			mips += "sw $v0, 0($sp) #把第一个操作数压入栈\n";
			mips += child3->codeGenerate();
			mips += "lw $v1, 0($sp) #把第一个操作数取出来到$v1中\n";
			switch(child2){
				case pplus:
				{
					mips += "add $v0, $v1, $v0\n";
					break;
				}
				case mminus:
				{
					mips += "sub $v0, $v1, $v0\n";
					break;
				}
				case oor:
				{
					mips += "or $v0, $v1, $v0\n";
					break;
				}
				default:
					break;
			}
		}
		else{
			mips += "addi $sp, $sp, -4\n";
			mips += "swc1 $f0, 0($sp) #把第一个操作数压入栈\n";
			mips += child3->codeGenerate();
			mips += "lwc1 $f1, 0($sp) #把第一个操作数取出来到$f1中\n";
			switch(child2){
				case pplus:
				{
					mips += "add.s $f0, $f1, $f0\n";
					break;
				}
				case mminus:
				{
					mips += "sub.s $f0, $f1, $f0\n";
					break;
				}
				default:
					break;
			}
		}

		mips += "addi $sp, $sp, 4 #弹栈\n";
	}
	else
	{
		mips += child3->codeGenerate();
	}

	return mips;
}

string expression::codeGenerate() {
	string mips;

	if (child1 != NULL) {
		mips += child1->codeGenerate();
		mips += "addi $sp, $sp, -4\n";
		mips += "sw $v0, 0($sp) #先把第一个要比较的数存入栈中\n";

		mips += child3->codeGenerate();
	
		mips += "lw $v1, 0($sp) #把第一个要比较的数取出来\n";
		switch (child2) {
			case gge:
				mips += "sge $v0, $v1, $v0\n";
				break;
			case ggt:
				mips += "sgt $v0, $v1, $v0\n";
				break;
			case lle:
				mips += "sle $v0, $v1, $v0\n";
				break;
			case llt:
				mips += "slt $v0, $v1, $v0\n";
				break;
			case eequal:
				mips += "seq $v0, $v1, $v0\n";
				break;
			case uunequal:
				mips += "sne $v0, $v1, $v0\n";
				break;
			default:
				break;	// Error
		}
		mips += "addi $sp, $sp, 4 #弹出第一个要比较的数\n";	// 弹栈
	}
	else
	{
		mips += child3->codeGenerate();
	}

	return mips;
}
