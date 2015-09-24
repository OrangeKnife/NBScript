#pragma once
#include <string>
using namespace std;
namespace NBE
{
	enum TOKEN
	{
		TOKEN_EOF = 0,
		TOKEN_ROOT,
		TOKEN_NAME,
		TOKEN_INT,
		TOKEN_FLOAT,
		TOKEN_OP,
		TOKEN_UNARY,//"!"
		TOKEN_STRING,
		TOKEN_CALL,
		TOKEN_VECTOR,

		//TOKEN_NULL,
		TOKEN_TRUE,
		TOKEN_FALSE,
		TOKEN_NOT,
		//TOKEN_VECTOR,
		TOKEN_IF,
		TOKEN_THEN,
		TOKEN_ELSEIF,
		TOKEN_ELSE,
		TOKEN_END,
		TOKEN_WHILE,
		TOKEN_DO,
		TOKEN_BREAK,
		TOKEN_RETURN,
		TOKEN_FUNC,
		TOKEN_PARA,//parameters
		TOKEN_VAR,
		TOKEN_SEMICOLON,
		TOKEN_COMMA,
		TOKEN_LEFTPAR,
		TOKEN_RIGHTPAR,
		TOKEN_LEFTBRACKET,
		TOKEN_RIGHTBRACKET,
		//TOKEN_LEFTBRACE,
		//TOKEN_RIGHTBRACE,
		TOKEN_ASSIGN,
		TOKEN_PLUS,
		TOKEN_MINUS,
		TOKEN_MULTIPLY,
		TOKEN_DIVIDE,
		TOKEN_LESS,
		TOKEN_LESSEQ,
		TOKEN_GREATER,
		TOKEN_GREATEREQ,
		TOKEN_EQ,
		TOKEN_NOTEQ,
		TOKEN_AND,
		TOKEN_OR,
		TOKEN_POP,
		TOKEN_NATIVECALL,
		TOKENSIZE
	};

	std::string TokenStrList[];

	class Node
	{
	public:
		union VALUE
		{
			Node* child;
			int i;
			float f;
			string* s;
			bool b;
		};
		Node(VALUE v,int t,Node* child, Node* sib):
		val(v),type(t),hasChild(false),sibling(NULL)
		{
			if(child != NULL)
			{
				val.child = child;
				child->sibling = sib;
				hasChild = true;
			}
		}
		~Node()
		{
			if(sibling)
				delete sibling;

			if(hasChild)
			{delete val.child;}
			if(type == TOKEN_STRING || type == TOKEN_NAME )
			{delete val.s;}
		}
		int type;
		Node* sibling;
		VALUE val;
		bool hasChild;
	};
}