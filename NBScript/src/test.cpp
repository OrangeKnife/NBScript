#include <stdio.h>
#include <tchar.h>
#include "windows.h"
#include "Node.h"
#include "Lex.h"
#include "Parser.h"
#include <crtdbg.h>
#include <iostream>
#include <vector>
#include "MyException.h"
//////////////////////////////////////////////////////////////////////////
#include "Node.h"
#include <stack>
#include <vector>
#include "Bytecode.h"
#include "VM.h"
#include "NativeFunc.h"
#include <fstream>
#include "Log.h"
using namespace NBE;




class NBSCompiler
{
public:
	NBSCompiler():myLex(new LexicalAnalyzer()),
		myParser(new Parser(myLex,nativeFuncMap)),
		root(new Node(Node::VALUE(),TOKEN_ROOT,NULL,NULL))
	{
	}
	~NBSCompiler()
	{
		delete root;
		delete code;
		delete myLex;
		delete myParser;
	}
	

	void loadScript(TCHAR* fileName)
	{
		myLex->load(fileName);
	}

	void parse()
	{
		try
		{
			Node* tempN;
			cout<<"\n------------analyze tokens------------\n";
			Log::writeLog("\n------------analyze tokens------------\n");
			while(myLex->token != TOKEN_EOF)
			{
				if(!root->val.child)
				{
					root->val.child = myParser->ParseStatement();
					root->hasChild = true;
					tempN = root->val.child;
				}
				else
				{
					while(tempN->sibling)
						tempN = tempN->sibling;
					tempN ->sibling = myParser->ParseStatement();
				}
			}
		}
		catch (MyException& e)
		{
			char str[256];
			sprintf_s(str,"--- Error: Line: %d. token: %s ---",e.lineNum,TokenStrList[e.token].c_str());
			std::cout<<str<<"\n";
			Log::writeLog(string(str) + "\n");
		}

		if (myParser->getErrorNum() > 0)
		{
			std::cout<<"------------------------------------\n"
				<<"Failed: "<<myParser->getErrorNum()<<" errors found\n";

			Log::writeLog("------------------------------------\n");
			Log::writeLog("Failed: " + to_string(myParser->getErrorNum()) + " errors found\n");

			exit(0);
		}
		else
		{
			cout<<"\n-----------------tree------------------\n";
			Log::writeLog("\n-----------------tree------------------\n");
			system("pause");
			outputTree(root);
		}
	}

	ByteCode* generateCodes()
	{
		if (myParser->getErrorNum() == 0)
		{
			cout<<"\n------------generating...------------\n";
			Log::writeLog("\n------------generating...------------\n");
			code= new ByteCode(myParser,nativeFuncMap);
			code->generate(root->val.child);
			cout<<"--------------byte codes---------------\n";
			Log::writeLog("--------------byte codes---------------\n");
			system("pause");
			code->output();
			return code;
		}
		return NULL;
	}

	void outputTree(Node* pNode)
{

	vector<Node*> ndVec;
	static int column = 0;

		switch(pNode->type)
		{
		case TOKEN_INT: std::cout << pNode->val.i; Log::writeLog(to_string(pNode->val.i)); break;
		case TOKEN_FLOAT: std::cout << pNode->val.f; Log::writeLog(to_string(pNode->val.f)); break;
		case TOKEN_NAME: std::cout << pNode->val.s->c_str(); Log::writeLog(*pNode->val.s); break;
		case TOKEN_STRING:std::cout << pNode->val.s->c_str(); Log::writeLog(*pNode->val.s); break;
		default: std::cout << TokenStrList[pNode->type]; Log::writeLog(TokenStrList[pNode->type]); break;
		}
		if(pNode->hasChild)
		{
			cout<<" -> ";
			Log::writeLog(" -> ");
			column++;
			ndVec.push_back(pNode->val.child);
			outputTree(pNode->val.child);
			column--;
		}

		if(pNode->sibling)
		{
			
			cout<<"\n";
			Log::writeLog("\n");
			for (int i = 0; i < column; ++i)
			{
				cout << "     ";
				Log::writeLog("     ");
			}
			cout<<"|\n";
			Log::writeLog("|\n");
			for (int i = 0; i < column; ++i)
			{
				cout << "     ";
				Log::writeLog("     ");
			}
			ndVec.push_back(pNode->sibling);
			
			outputTree(pNode->sibling);
			
		}
}

	std::unordered_map<string,NativeFunc*>& getNativeFuncMap(){return nativeFuncMap;}


	void regNativeFunc(char* funcName,myNativefunc p, int funcParaNum)
	{
		nativeFuncMap[string(funcName)] = new NativeFunc(funcParaNum,p,nativeFuncMap.size());
	}

private:
	LexicalAnalyzer* myLex;
	Parser* myParser;
	Node* root;
	ByteCode* code;
 
	NativeFuncMap nativeFuncMap;
};



int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );

	NBSCompiler cp;
	cp.loadScript(TEXT("testCompiler.nbs"));
	cp.regNativeFunc("print",printFunc,-1);//-1 : support multiple parameters
	cp.regNativeFunc("square",square,1);//-1 : support multiple parameters
	cp.regNativeFunc("IntToStr",IntToStr,1);//-1 : support multiple parameters

	cp.parse();
	ByteCode* d = cp.generateCodes();

	if(d->getByteCode().size() > 0)
	{
		VM myVM (d->getByteCode(),cp.getNativeFuncMap());
		myVM.execute();
	}
	 
	cout<<"-------------------------------\n";

	cout<<"string ref count increase:"<<Value::g_string_in<<"\n";
	cout<<"string ref count decrease:"<<Value::g_string_de<<"\n";
	cout<<"vector ref count increase:"<<Value::g_vector_in<<"\n";
	cout<<"vector ref count decrease:"<<Value::g_vector_de<<"\n";
	std::cout<<"\n";

	system("pause");

	Log::Release();
	return 0;
}


