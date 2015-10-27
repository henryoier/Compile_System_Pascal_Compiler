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


string repeat_stmt::codeGenerate() {
	string mips, label;
	stringstream ss;

	uniqueNum++;
	ss << "repeat"; //这里的ss可能会错 wtfwtf 可能要用to_string 可能两次<<会出错
	ss << uniqueNum;
	ss >> label;
	mips += label + ":\n";

	stmt_list* s_list = child1;
	while (s_list != NULL){
		mips += s_list->codeGenerate();
		s_list = s_list->next;
	}

	mips += child2->codeGenerate();
	mips += "beq $v0, $zero, " + label + " #条件满足时跳出循环\n";

	return mips;
}

string while_stmt::codeGenerate(){
	string mips, label1, label2;
	stringstream ss;

	uniqueNum++;
	ss << "while";
	ss << uniqueNum;
	ss >> label1;
	ss.clear();
	ss << "go_on";
	ss << uniqueNum;
	ss >> label2;

	mips += label1 + ":\n";
	mips += child1->codeGenerate();
	mips += "beq $v0, $zero, " + label2 + " #条件不再满足时跳出循环\n";

	mips += child2->codeGenerate();
	mips += "j " + label1 + " #循环\n\n";
	mips += label2 + ":\n";

	return mips;
}

string for_stmt::codeGenerate() {
	string mips, label1, label2, offset;
	stringstream ss;

	uniqueNum++;
	ss << "for";
	ss << uniqueNum;
	ss >> label1;
	ss.clear();
	ss << "go_on";
	ss << uniqueNum;
	ss >> label2;
	ss.clear();

	identifier* targetID = gtable.lookUp(child1, currentScope);
	if (targetID->type != ssimple_type || targetID->stype != iinteger)
		abort();
	int scopePtr = currentScope;
	mips += "add $t0, $zero, $fp #先找出ID的地址\n";
	while (scopePtr != targetID->scope) {
		mips += "lw $t0, 4($t0)\n";
		scopePtr = gtable.getScope(scopePtr).parent;
	}
	mips += "addi $sp, $sp, -4\n";
	mips += "sw $t0, 0($sp) #把id的地址压入栈\n";

	mips += child4->codeGenerate();
	mips += "addi $sp, $sp, -4\n";
	mips += "sw $v0, 0($sp) #把终止条件压入栈\n";
	
	mips += child2->codeGenerate();	// initial condition
	mips += "lw $t0, 4($sp)\n";
	ss << targetID->offset;
	ss >> offset;
	ss.clear();
	mips += "sw $v0, " + offset + "($t0) #把初始条件写到id中\n";

	mips += label1 + ":\n";
	switch (child3){	// 1: to  2: downto
		case 1:
		{
			mips += "lw $v1, 0($sp) #取出终止条件到$v1中\n";
			mips += "sle $t0, $v0, $v1\n";
			mips += "beq $t0, $zero, " + label2 + "\n";
			break;
		}
		case 2:
		{
			mips += "lw $v1, 0($sp) #取出终止条件到$v1中\n";
			mips += "sge $t0, $v0, $v1\n";
			mips += "beq $t0, $zero, " + label2 + "\n";
			break;
		}
		default:
			break;	// Error
	}

	mips += child5->codeGenerate();
	mips += "lw $t0, 4($sp) #取出id地址\n";
	mips += "lw $v0, " + offset + "($t0) #取出id当前值到$v0中\n";
	switch (child3){	// 1: to  2: downto
		case  1:
		{
			mips += "addi $v0, $v0, 1\n";
			break;
		}
		case  2:
		{
			mips += "addi $v0, $v0, -1\n";
			break;
		}
		default:
			break;	// Error
	}
	mips += "sw $v0, " + offset + "($t0)\n";

	mips += "j " + label1 + " #循环\n\n";
	mips += label2 + ":\n";
	// restore var
	mips += "lw $t0, 4($sp) #取出id地址\n";
	mips += "lw $v0, " + offset + "($t0) #取出id当前值到$v0中\n";
	switch (child3){	// 1: to  2: downto
		case  1:
		{
			mips += "addi $v0, $v0, -1 #恢复id的值\n";
			mips += "#因为实际代码不会让id超出for范围\n";
			break;
		}
		case  2:
		{
			mips += "addi $v0, $v0, 1 #恢复id的值\n";
			mips += "#因为实际代码不会让id超出for范围\n";
			break;
		}
		default:
			break;	// Error
	}
	mips += "sw $v0, " + offset + "($t0) #写入id的最终值\n";
	mips += "addi $sp, $sp, 8 #弹出终止条件和id地址\n\n";

	return mips;
}
