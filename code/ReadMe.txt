1. 把“源代码”文件夹内所有文件拷贝到linux系统下（开发的系统是ubuntu14.04 LTS），假设拷贝到了目录D

2. 在目录D下，使用命令行输入flex pascal.l （需要先安装flex，ubuntu下可使用命令sudo apt-get install flex 安装flex；当然也可以直接到第4步，因为源代码中提供了lex.yy.c文件）

3. 命令行输入yacc -d pascal.y （需要安装bison，ubuntu下可使用命令sudo apt-get install bison 安装bison；当然也可以直接到第4步，因为源代码中提供了y.tab.c和y.tab.h文件）

4. 命令行输入：
g++ semanticANLS.cpp lex.yy.c main.cpp symbolTable.cpp util.cpp y.tab.c codeGenerate.cpp basicToken.cpp loopStmt.cpp -std=c++0x
会得到a.out可执行文件，这就是我们的pascal编译器了 (a.out已提供，也可以直接使用)

5. 假如要编译test.pas代码，在命令行输入 ./a.out <test.pas> test.asm 即可得到目标代码文件test.asm