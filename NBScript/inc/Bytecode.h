#pragma once
#define byte unsigned char
#include <unordered_map>
#include "Lex.h"
#include "Parser.h"
namespace NBE
{
	enum COMMANDTYPE
	{
		C_PUSH_FLOAT = 0,
		C_PUSH_INT,
		C_PUSH_BOOLEAN,
		C_PUSH_STRING,
		C_PUSH_NAME,//variable
		C_NOT,// not !
		C_OR,// ||
		C_AND,// &&
		C_EQ,// ==
		C_NOTEQ,// !=
		C_GREATER,// >
		C_LESS,// <
		C_GREATEREQ,// >=
		C_LESSEQ,// <=
		C_PLUS,// +
		C_MINUS,// -
		C_MULTIPLY,// *
		C_DIVIDE,// /
		C_ASSIGN,// =
		C_ASSIDX,// assign to a value in a vector
		C_VAR,//var
		C_INDEXING,// [
		C_VECTOR,
		C_CALL,// function call
		C_NATIVECALL,//native function call
		C_FUNC,//function definition
		C_PARA,//indicate the parameters
		C_LOCALVAR,//var number in func
		C_BREAK,//break a while
		C_RETURN,//function return
		C_RETURN_NO_VALUE,//return without value
		C_IFNE,// if not equal
		C_THEN,//no command
		C_JUMP,//jump command
		C_POP,
		C_EXIT,//end of execution

		C_NONE,// do not recurse the node , eg. var in func
		C_TOTALCOMNUM
	};

	std::string commandNameStrList[];

	class ByteCode
	{
	public:
		struct data
		{
			int type;
			byte bt[4];
			data()
			{
				memset(bt,0,4);
			}
		};

		struct funcReq
		{
			funcReq(string n,unsigned int l):name(n),loc(l)
			{}
			string name;
			unsigned int loc;
		};

		ByteCode(Parser* paser, NativeFuncMap& nfmap);
		~ByteCode();

		unsigned int getVecLen(Node* nd);
		
		void generate(Node* nd);
		void generateFromNode(Node* nd,funcInfo* finfo);
		void generateFunc(Node* nd);
		int initializeFuncParaOffset(Node* nd,unordered_map<string,int>* pFuncParaMap);
		int initializeFuncVarOffset(Node* bodyNode,unordered_map<string,int>* pFuncParaMap);//save the variable map into the para map

		void pushData(byte* commandT, byte* arr, int len);
		void genCommandCode(byte* command, int len);
		void genDataCode(byte* btarray, int len);

		void output();
		std::vector<byte>& getByteCode(){return m_byteCode;}
	private:
		std::vector<byte> m_byteCode;
		std::vector<funcReq> m_funcCallRequests;
		Parser* ps;

		std::vector<unsigned int> m_breakRequests;

		NativeFuncMap& nativeFuncMap;
	};
}