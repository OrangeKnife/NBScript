#pragma once
#include <unordered_map>
#include "Value.h"
namespace NBE
{
	class funcInfo
	{
	public:
		funcInfo():numOfPara(0),numOfVar(0),defined(false),hasReturn(false)//,returnNull(true)
		{}
		~funcInfo(){}
		int numOfPara;
		int numOfVar;
		bool hasReturn;//check if need to add a "return" at the end of the func
		//bool returnNull;//after check hasReturn, check if return a value or just return;
		std::unordered_map<string,int> varMap;//includes the paras
		bool defined;
		unsigned int funcAddress;
	};

	typedef Value (*myNativefunc)(vector<Value>& stack,int numOfPara);

	struct NativeFunc
	{
		NativeFunc(int npara,myNativefunc fp, unsigned int _id):numOfPara(npara),funcPointer(fp),id(_id)
		{}
		~NativeFunc()
		{
		}
		unsigned int id;
		int numOfPara;
		myNativefunc funcPointer;
	};

	typedef std::unordered_map<std::string,NativeFunc*> NativeFuncMap;

	

	class Parser
	{
	public:
		Parser(LexicalAnalyzer* _lex, NativeFuncMap& nfmap);
		~Parser();
		Node* ParseExpression();
		Node* ParseStatement();

		Node* ParseAndExp();
		Node* ParseOrExp();
		Node* ParseCompExp();
		Node* ParseEqExp();
		Node* ParsePlusExp();
		Node* ParseMulExp();
		Node* ParseUnaryExp();
		Node* ParseFactor();


		bool Expect(int t);
		void error(char* errmsg);

		std::unordered_map<string,funcInfo*>& getFuncInfoMap(){return funcInfoMap;}

		unsigned int getErrorNum(){return errorNum;}

	
	private:
		LexicalAnalyzer* m_lex;
		
		std::unordered_map<string,funcInfo*> funcInfoMap;

		bool checkName;//whether or not check the variable name 
		funcInfo* finfo; //inside the current function, this records the func info

		unsigned int errorNum;

		NativeFuncMap& nativeFuncMap;
	};
}