#pragma once
#include <node.h>
#include <string>
#include <tchar.h>
#define KEYWORDSIZE 15
#define OPERATORSIZE 14

namespace NBE
{
	class  LexicalAnalyzer
	{
	public:
		LexicalAnalyzer():token(-1),lineNum(1),c(-1),current(0),attrint(-1)
		{}
		~LexicalAnalyzer(){delete[] buffer;}

		

		int load(TCHAR* fileName);
		void next();
		std::string* getStr(char*& current);
		void error(char* errmsg);
		void outputToken(const char* tk);
		bool isAToken(std::string& str, int& tk);
		void nextLine();
		void nextStarSlash();
		
		bool isLastTokenOfLine();
		//static std::string Keyword[KEYWORDSIZE];
		//static std::string Op[OPERATORSIZE];
		

		int  lineNum,token;
		union Node::VALUE val;
	private:
		int c;
		
		char* current;
		int attrint;
		float attrfloat;

		int bufferSize;
		char* buffer;

	};

}