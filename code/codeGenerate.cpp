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
int uniqueNum = 0;
int currentScope = 0;


string program::codeGenerate() {
	string mips;
	int originalScope = currentScope;

	// program_head
	mips += child1->codeGenerate();

	currentScope = gtable.lookUpScope(this);
	// routine
	mips += child2->codeGenerate();

	currentScope = originalScope;

	return mips;
}

string program_head::codeGenerate() {
	string mips, tmp;
	stringstream ss;

	ss << currentScope;
	ss >> tmp;

	// 打印标号，压栈 ra
	mips += child1->name + tmp + ":\n";
	mips += "sw $ra, 0($fp) #保存返回地址\n";

	return mips;
}

string routine::codeGenerate() {
	string mips;

	mips += child2->codeGenerate();	// routine_body
	mips += "addi $sp, $sp, 100\n";		// System stack clean
	mips += "addi $v0, $zero, 10\n";		// Exit
	mips += "syscall\n\n";
	mips += child1->codeGenerate();	// routine_head  让函数在main后面出现

	return mips;
}

string routine_head::codeGenerate() {
	string mips;

	mips += child1->codeGenerate();
	mips += child2->codeGenerate();
	mips += child3->codeGenerate();
	mips += child4->codeGenerate();
	mips += child5->codeGenerate();

	return mips;
}

string routine_part::codeGenerate() {
	string mips;

	function_decl_list * f_list = child1;
	while (f_list != NULL){
		mips += f_list->codeGenerate();
		f_list = f_list->next;
	}

	procedure_decl_list * p_list = child2;
	while (p_list != NULL){
		mips += p_list->codeGenerate();
		p_list = p_list->next;
	}

	return mips;
}

string function_decl::codeGenerate() {
	string mips;
	int originalScope = currentScope;

	// function_head
	mips += child1->codeGenerate();

	currentScope = gtable.lookUpScope(this);
	// sub_routine
	mips += child2->codeGenerate();

	currentScope = originalScope;

	return mips;
}

string function_head::codeGenerate() {
	string mips, tmp;
	stringstream ss;

	ss << currentScope;
	ss >> tmp;

	// 打印标号，压栈 ra
	mips += child1->name + tmp + ":\n";
	mips += "sw $ra, 0($fp) #保存返回地址\n\n";

	return mips;
}

string procedure_decl::codeGenerate() {
	string mips;

	int originalScope = currentScope;

	// procedure_head
	mips += child1->codeGenerate();

	currentScope = gtable.lookUpScope(this);
	// sub_routine
	mips += child2->codeGenerate();

	currentScope = originalScope;

	return mips;
}

string procedure_head::codeGenerate() {
	string mips, tmp;
	stringstream ss;

	ss << currentScope;
	ss >> tmp;

	// 打印标号，压栈 ra
	mips += child1->name + tmp + ":\n";
	mips += "sw $ra, 0($fp) #保存返回地址\n\n";

	return mips;
}


string sub_routine::codeGenerate() {
	string mips;

	mips += child2->codeGenerate();	// routine_body
	mips += "lw $ra, 0($fp) #取出返回地址\n";
	mips += "jr $ra #跳回返回地址\n\n";		// return

	mips += child1->codeGenerate();	// routine_head

	return mips;
}

string routine_body::codeGenerate() {
	string mips;

	mips += child1->codeGenerate();

	return mips;
}

string sys_proc::codeGenerate(SimpleType choice1) {
	string mips;

	//mips += child1->codeGenerate();

	switch (choice1) {
		case  iinteger: // int
			mips += "add $a0, $zero, $v0 #要输出的数存到$a0中\n";
			mips += "addi $v0, $zero, 1\n";
			mips += "syscall\n\n";
			break;
		case  rreal:	// float
			mips += "add.s $f12, $f31, $f0\n";
			mips += "addi $v0, $zero, 2\n";
			mips += "syscall\n\n";
			break;
		case  cchar:	// char
			mips += "add $a0, $zero, $v0 #要输出的字符存到$a0中\n";
			mips += "addi $v0, $zero, 11\n";
			mips += "syscall\n\n";
			break;
		default:
			break;
	}
	
	return mips;
}


string compound_stmt::codeGenerate() {
	string mips;

	stmt_list * s_list = child1;
	while (s_list != NULL) {
		mips += s_list->codeGenerate();
		s_list = s_list->next;
	}

	return mips;
}

string stmt::codeGenerate() {
	string mips, tmp;
	stringstream ss;

	if (choice1) {	// 有标号
		ss << child1;
		ss >> tmp;
		mips += "mipsLabel" + tmp + ":\n";
	}

	mips += child2->codeGenerate();

	return mips;
}

string non_label_stmt::codeGenerate() {
	string mips;

	switch (choice1) {
		case  1:
			mips += child1.choice1->codeGenerate();
			break;
		case  2:
			mips += child1.choice2->codeGenerate();
			break;
		case  3:
			mips += child1.choice3->codeGenerate();
			break;
		case  4:
			mips += child1.choice4->codeGenerate();
			break;
		case  5:
			mips += child1.choice5->codeGenerate();
			break;
		case  6:
			mips += child1.choice6->codeGenerate();
			break;
		case  7:
			mips += child1.choice7->codeGenerate();
			break;
		case  8:
			mips += child1.choice8->codeGenerate();
			break;
		case  9:
			mips += child1.choice9->codeGenerate();
			break;
		default:
			break; // Error
	}

	return mips;
}

string assign_stmt::codeGenerate() {
	string mips, tmp, offset;
	stringstream ss;

	switch (choice2) {
		case  1:	// ID
		{
			mips += child2.choice1->codeGenerate();
			// 查 child1 地址
			identifier *targetID = gtable.lookUp(child1, currentScope);
			if (targetID->type == ffunction) {
			}
			else {
				int scopePtr = currentScope;
				mips += "add $t0, $zero, $fp #先找出ID的地址\n";
				while (scopePtr != targetID->scope) {
					mips += "lw $t0, 4($t0)\n"; //wtfwtf
					scopePtr = (gtable.getScope(scopePtr)).parent;
				}
				ss << targetID->offset;
				ss >> offset;
				if(targetID->stype == rreal){
					mips += "swc1 $f0, " + offset + "($t0)\n\n";
				}
				else{
					mips += "sw $v0, " + offset + "($t0)\n\n";
				}
			}

			break;
		}
		case  2:	// Array 貌似有问题 case3 才是array吧？
		{
			identifier *targetID = gtable.lookUp(child1, currentScope);

			mips += child2.choice2.child2->codeGenerate();  //表达式从后往前算
			mips += "addi $sp, $sp, -4\n";
			if (targetID->stype == rreal){
				mips += "swc1 $f0, 0($sp) #先把要赋的值存在栈中\n ";
			}
			else
			{
				mips += "sw $v0, 0($sp) #先把要赋的值存在栈中\n ";	
			}
			

			//数组偏移地址 -> $v0
			mips += child2.choice2.child1->codeGenerate();

			//数组首地址 -> $t0
			int scopePtr = currentScope;
			mips += "add $t0, $zero, $fp #先找出ID的地址\n";
			while (scopePtr != targetID->scope) {
				mips += "lw $t0, 4($t0)\n";
				scopePtr = (gtable.getScope(scopePtr)).parent;
			}
			ss << targetID->offset;
			ss >> offset;
			mips += "lw $t0, " + offset + "($t0) #取出数组首地址\n";

			// 计算数组元素地址 -> $t0
			mips += "sll $v0, $v0, 2\n";
			mips += "add $t0, $t0, $v0 #算出数组元素地址\n";

			// 赋值
			if (targetID->stype == rreal){
				mips += "lwc1 $f0, 0($sp) #取出要赋的值\n";
				mips += "addi $sp, $sp, 4\n";
				mips += "swc1 $f0, 0($t0) #把要赋的值写到素组元素地址中\n\n";
			}
			else
			{
				mips += "lw $v0, 0($sp) #取出要赋的值\n";
				mips += "addi $sp, $sp, 4\n";
				mips += "sw $v0, 0($t0) #把要赋的值写到素组元素地址中\n\n";	
			}
			

			break;
		}
		case  3:
		{
			mips += child2.choice3.child2->codeGenerate();
			// TODO:
			break;
		}
		default:
			break;	// Error
	}

	return mips;
}

string proc_stmt::codeGenerate() {
	string mips, tmp;
	stringstream ss;
	int funcSize;
	char temp[30];

	switch (choice1) {
		case  1:		// 没有参数的 Procedure
		{
			// 参数压栈
			identifier* targetFunc = gtable.lookUp(child1.choice1, currentScope);
			if (targetFunc->type != ffunction && targetFunc->type != pprocedure)
				abort();
			funcSize = targetFunc->offset;
			int argsCounter = 0;
			int varSize = funcSize - 8 - argsCounter * 4;
			int childscope = -1;
			switch (targetFunc->type) {
				case ffunction:
					childscope = gtable.lookUpScope(targetFunc->value.func);
					break;
				case pprocedure:
					childscope = gtable.lookUpScope(targetFunc->value.proc);
					break;
				default: break;		// Error
			}
			//cout << "CURRENT SCOPE: |" << currentScope << "|" << endl;
			//cout << "PROC CHILDSCOPE: |" << childscope << "|" << endl;
			assert(childscope != -1);

			// 获取 access link -> $t0
			int scopePtr = currentScope;
			mips += "add $t0, $zero, $fp #先找出ID的地址\n";
			while (scopePtr != targetFunc->scope) {
				mips += "lw $t0, 4($t0)\n";
				scopePtr = gtable.getScope(scopePtr).parent;
			}
			sprintf(temp, "%d", -1 * varSize - 4);	// access link
			mips += "sw $t0, " + string(temp) + "($sp)\n";

			sprintf(temp, "%d", -1 * funcSize);	// 开栈（参数、access link、$ra）
			mips += "addi $sp, $sp, " + string(temp) + "\n";
			mips += "add $t7, $zero, $fp\n";		// 保存 $fp for control link
			mips += "add $fp, $zero, $sp\n";

			sprintf(temp, "%d", -4);	// 开栈（control link）
			mips += "addi $sp, $sp, " + string(temp) + "\n";
			mips += "sw $t7, 0($sp)\n";

			// 为数组参数申请空间
			assert(targetFunc->type == pprocedure);	// procedure_decl
			assert(targetFunc->value.proc->child2->child1->child4 != NULL);	// var_part
			var_decl_list * vars = targetFunc->value.proc->child2->child1->child4->child1;
			// procedure scope
			int pscope = gtable.lookUpScope(targetFunc->value.proc);
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
						sprintf(temp, "%d", -1 * arraySize * 4);
						mips += "addi $sp, $sp, " + string(temp) + "\n";
						mips += "sw $sp, " + to_string(newvariden->offset) + "($fp)\n";

						varName = varName->child1;
					}
				}
				else {}

				vars = vars->next;
			}

			// 找 function 地址
			int funcScope = gtable.lookUp(child1.choice1, currentScope)->scope;
			sprintf(temp, "%d", funcScope);

			// 跳转
			mips += "jal " + child1.choice1->name + string(temp) + "\n";

			// 返回的时候清栈
			mips += "lw $fp, -4($fp)\n";
			sprintf(temp, "%d", 4);	// 收栈（control link）
			mips += "addi $sp, $sp, " + string(temp) + "\n";
			sprintf(temp, "%d", funcSize);	// 收栈（参数、access link、$ra）
			mips += "addi $sp, $sp, " + string(temp) + "\n";

			break;
		}
		case  2:	// 有参数的 Procedure
		{
			// 参数压栈
			identifier * targetFunc = gtable.lookUp(child1.choice1, currentScope);

			assert(targetFunc->type == pprocedure);
			funcSize = targetFunc->offset;
			int argsCounter = 0;
			assert(targetFunc->paras!=NULL);
			argsCounter = targetFunc->paras->size();
			int varSize = funcSize - 8 - argsCounter * 4;
			int childscope = -1;
			switch (targetFunc->type) {
				case ffunction:
					childscope = gtable.lookUpScope(targetFunc->value.func);
					break;
				case pprocedure:
					childscope = gtable.lookUpScope(targetFunc->value.proc);
					break;
				default: break;		// Error
			}
			//cout << "CURRENT SCOPE: |" << currentScope << "|" << endl;
			//cout << "PROC CHILDSCOPE: |" << childscope << "|" << endl;
			assert(childscope != -1);

			args_list * ptr = child1.choice2.child2;
			string recover = "";	// restore for references
			for (int i = 0; i < argsCounter; i++) {
				// 计算参数值
				mips += ptr->codeGenerate();
				// 压栈
				sprintf(temp, "%d", -1 * funcSize + 8 + i * 4);
				mips += "sw $v0, " + string(temp) + "($sp)\n";

				//cout << "finding " << targetFunc->paras->at(i) << endl;
				identifier * argID = gtable.lookUp(targetFunc->paras->at(i), childscope);
				assert(argID != NULL);
				// 参数为引用则生成赋值语句
				//cout << argID->passByReference << endl;
				if (argID->passByReference) {
					//sprintf(temp, "%d", 8 + i * 4);
					recover += "lw $t1, " + string(temp) + "($sp)\n";

					int scopePtr = currentScope;
					recover += "add $t0, $zero, $fp\n";
					assert(ptr->child3->child3->child3->choice1 == 1);
					identifier *sbid = gtable.lookUp(ptr->child3->child3->child3->child1.choice1, currentScope);
					assert(sbid != NULL);

					while (scopePtr != sbid->scope) {
						recover += "lw $t0, 4($t0)\n";
						scopePtr = gtable.getScope(scopePtr).parent;
					}
					sprintf(temp, "%d", sbid->offset);

					recover += "sw $t1, " + string(temp) + "($t0)\n";
				}

				ptr = ptr->next;
			}

			// 获取 access link -> $t0
			int scopePtr = currentScope;
			mips += "add $t0, $zero, $fp #先找出ID的地址\n";
			while (scopePtr != targetFunc->scope) {
				mips += "lw $t0, 4($t0)\n";
				scopePtr = gtable.getScope(scopePtr).parent;
			}
			sprintf(temp, "%d", -1 * funcSize + 4);	// access link
			mips += "sw $t0, " + string(temp) + "($sp)\n";


			sprintf(temp, "%d", -1 * funcSize);		// 开栈（参数、access link、$ra）
			mips += "addi $sp, $sp, " + string(temp) + "\n";
			mips += "add $t7, $zero, $fp\n";		// 保存 $fp for control link
			mips += "add $fp, $zero, $sp\n";

			sprintf(temp, "%d", -4);	// 开栈（control link）
			mips += "addi $sp, $sp, " + string(temp) + "\n";
			mips += "sw $t7, 0($sp)\n";

			// 为数组参数申请空间
			assert(targetFunc->type == pprocedure);	// procedure_decl
			assert(targetFunc->value.proc->child2->child1->child4 != NULL);	// var_part
			var_decl_list * vars = targetFunc->value.proc->child2->child1->child4->child1;
			// procedure scope
			int pscope = gtable.lookUpScope(targetFunc->value.proc);
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
						sprintf(temp, "%d", -1 * arraySize * 4);
						mips += "addi $sp, $sp, " + string(temp) + "\n";
						mips += "sw $sp, " + to_string(newvariden->offset) + "($fp)\n";

						varName = varName->child1;
					}
				}
				else {}

				vars = vars->next;
			}

			// 找 function 地址
			int funcScope = gtable.lookUp(child1.choice2.child1, currentScope)->scope;
			sprintf(temp, "%d", funcScope);

			// 跳转
			mips += "jal " + child1.choice2.child1->name + string(temp) + "\n";

			// 清栈
			mips += "lw $fp, -4($fp)\n";
			sprintf(temp, "%d", 4);	// 收栈（control link）
			mips += "addi $sp, $sp, " + string(temp) + "\n";
			sprintf(temp, "%d", funcSize);	// 收栈（参数、access link、$ra）
			mips += "addi $sp, $sp, " + string(temp) + "\n";

			// 返回的时候对引用进行赋值
			mips += recover;
			
			break;
		}
		case  3: {		// 有参数的 Sys_proc
			break;}
		case  4: {		// 有参数的 Sys_proc
			if (child1.choice4.child2->child3->child3->child3->choice1 == 5 &&
				child1.choice4.child2->child3->child3->child3->child1.choice5->choice1 == 4) {
				// 输出字符串
				assert(child1.choice4.child2->child3->child3->child3->child1.choice5->child1.choice4 != NULL);
				//mips += "print_str(\"" + *(child1.choice4.child2->child3->child3->child3->child1.choice5->child1.choice4) + "\")\n";
				uniqueNum++;
				ss << "string";
				ss << uniqueNum;
				ss >> tmp;
				ss.clear();
				mips += ".data\n";
				mips += tmp + ":\n";
				mips += ".asciiz \"" + *(child1.choice4.child2->child3->child3->child3->child1.choice5->child1.choice4) + "\"\n";
				mips += ".text\n";
				mips += "addi $v0, $zero, 4\n";
				mips += "la $a0, " + tmp + "\n";
				mips += "syscall\n";
			}
			else {
				// 输出 expression
				mips += child1.choice4.child2->codeGenerate();	// expression
				mips += child1.choice4.child1->codeGenerate(child1.choice4.child2->value.stype);	// sys_proc
			}

			switch (child1.choice4.child1->child1) {
				case wwrite: break;
				case wwriteln: 
					//mips += "addi $t0, $zero, 10\n";
					//mips += "print_char($t0)\n";
					mips += "addi $v0, $zero, 11\n";
					mips += "addi $a0, $zero, 10\n";
					mips += "syscall\n";
					break;
			}
			break;}
		case  5: {		// Read
			SimpleType tarSType;

			switch (child1.choice5->choice1) {
				case  1:{	// ID
					identifier * targetFunc = gtable.lookUp(child1.choice5->child1.choice1, currentScope);
					assert(targetFunc != NULL);
					assert(targetFunc->type == ssimple_type);
					tarSType = targetFunc->stype;		// Target simple type
					// 查找 ID 地址 -> $t0
					int scopePtr = currentScope;
					mips += "add $t0, $zero, $fp #先找出ID的地址\n";
					while (scopePtr != targetFunc->scope) {
						mips += "lw $t0, 4($t0)\n";
						scopePtr = gtable.getScope(scopePtr).parent;
					}
					char temp[30];
					sprintf(temp, "%d", targetFunc->offset);
					mips += "addi $t0, $t0, " + string(temp) + "\n";

					break;
					}
				case  9:{	// Array
					identifier *targetFunc = gtable.lookUp(child1.choice5->child1.choice9.child1, currentScope);
					assert(targetFunc != NULL);
					assert(targetFunc->type == aarray_type);
					tarSType = targetFunc->stype;
					int scopePtr = currentScope;
					mips += child1.choice5->child1.choice9.child2->codeGenerate();
					mips += "add $t0, $zero, $fp #先找出ID的地址\n";
					while(scopePtr != targetFunc->scope) {
						mips += "lw $t0, 4($t0)\n";
						scopePtr = gtable.getScope(scopePtr).parent;
					}				
					char temp[30];
					sprintf(temp, "%d", targetFunc->offset);
					mips += "addi $t0, $t0, " + string(temp) + "\n";
					mips += "lw $t1, 0($t0)\n";
					mips += "sll $v0, $v0, 2\n";
					mips += "add $t0, $t1, $v0\n";

					break;
				}
				default: break;		// Error
			}

			switch (tarSType) {
				case iinteger:
					mips += "li $v0, 5\n";
					mips += "syscall\n";

					mips += "sw $v0, 0($t0)\n";
					break;
				case rreal:
					mips += "li $v0, 6\n";
					mips += "syscall\n";
					
					mips += "swc1 $f0, 0($t0)\n";
					break;
				case cchar:
					break;
				default: break;		// Error
			}

			break;}
		default: {break;}		// Error
	}

	return mips;
}



string if_stmt::codeGenerate(){
	string mips, label1, label2, label3;
	stringstream ss;

	uniqueNum++;
	ss << "if";
	ss << uniqueNum;
	ss >> label1;
	ss.clear();
	ss << "else";
	ss << uniqueNum;
	ss >> label2;
	ss.clear();
	ss << "go_on";
	ss << uniqueNum;
	ss >> label3;
	ss.clear();

	mips += label1 + ":\n";
	mips += child1->codeGenerate();
	mips += "beq $v0, $zero, " + label2 + " #若if条件不满足，跳到else子句\n";
	if(child2)
		mips += child2->codeGenerate();
	mips += "j " + label3 +" #if子句结束\n";
	mips += label2 + ":\n";
	if(child3) 
		mips += child3->codeGenerate();
	mips += label3 + ":\n";

	return mips;
}
string else_clause::codeGenerate(){
	string mips;

	if(choice1 != 0 && child1){
		mips += child1->codeGenerate();
	}

	return mips;
}

string goto_stmt::codeGenerate(){
	string mips, label;
	stringstream ss;

	ss << "mipsLabel";
	ss << child1;
	ss >> label;
	mips += "j " + label + " #goto语句\n";

	return mips;
}

string case_stmt::codeGenerate(){
	string mips, label1, label2, caseLabel;
	stringstream ss;

	uniqueNum++;
	ss << "switch";
	ss << uniqueNum;
	ss >> label1;
	ss.clear();
	ss << "go_on";
	ss << uniqueNum;
	ss >> label2;
	ss.clear();

	mips += label1 + ":\n";
	mips += child1->codeGenerate();
	mips += "#switch中的值存在了$v0中\n";

	case_expr* c_list = child2;
	while (c_list != NULL){
		uniqueNum++;
		ss << "case";
		ss << uniqueNum;
		ss >> caseLabel;
		ss.clear();

		mips += c_list->codeGenerate();
		mips += "j " + label2 + "\n";
		mips += caseLabel + ":\n";
		c_list = c_list->next;
	}

	mips += label2 + ":\n";
	return mips;
}

string case_expr::codeGenerate(){
	string mips, caseLabel, tmp;
	stringstream ss;

	ss << "case";
	ss << uniqueNum;
	ss >> caseLabel;
	ss.clear();

	switch (choice1){
		case 1:
		{
			switch (child1.choice1->choice1) {
				case 1:
				{
					ss << child1.choice1->child1.choice1;
					ss >> tmp;
					ss.clear();
					mips += "addi $v1, $zero, " + tmp + "\n";
					break;
				}
				case  2:
					break;
				case  3:
				{
					ss << child1.choice1->child1.choice3;
					ss >> tmp;
					ss.clear();
					mips += "addi $v1, $zero, '" + tmp + "'\n";
					break;
				}
				default:
					break;
			}
			break;
		}
		case  2:
			// TODO
			break;
		default:
			break;
	}
	mips += "bne $v0, $v1 " + caseLabel + "\n";
	mips += child2->codeGenerate();

	return mips;
}


