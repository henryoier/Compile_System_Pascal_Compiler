1. �ѡ�Դ���롱�ļ����������ļ�������linuxϵͳ�£�������ϵͳ��ubuntu14.04 LTS�������追������Ŀ¼D

2. ��Ŀ¼D�£�ʹ������������flex pascal.l ����Ҫ�Ȱ�װflex��ubuntu�¿�ʹ������sudo apt-get install flex ��װflex����ȻҲ����ֱ�ӵ���4������ΪԴ�������ṩ��lex.yy.c�ļ���

3. ����������yacc -d pascal.y ����Ҫ��װbison��ubuntu�¿�ʹ������sudo apt-get install bison ��װbison����ȻҲ����ֱ�ӵ���4������ΪԴ�������ṩ��y.tab.c��y.tab.h�ļ���

4. ���������룺
g++ semanticANLS.cpp lex.yy.c main.cpp symbolTable.cpp util.cpp y.tab.c codeGenerate.cpp basicToken.cpp loopStmt.cpp -std=c++0x
��õ�a.out��ִ���ļ�����������ǵ�pascal�������� (a.out���ṩ��Ҳ����ֱ��ʹ��)

5. ����Ҫ����test.pas���룬������������ ./a.out <test.pas> test.asm ���ɵõ�Ŀ������ļ�test.asm