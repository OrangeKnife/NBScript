#include "Node.h"
#include "Lex.h"
#include "Parser.h"
#include <iostream>
#include "MyException.h"
namespace NBE
{
	Parser::Parser(LexicalAnalyzer* _lex, NativeFuncMap& nfmap ):m_lex(_lex),errorNum(0),checkName(true),
		nativeFuncMap(nfmap)
	{
	}



	Parser::~Parser()
	{
		for (auto it = funcInfoMap.begin();it!=funcInfoMap.end();++it)
		{
			delete (*it).second;
		}
		funcInfoMap.clear();

		for (auto it = nativeFuncMap.begin(); it!=nativeFuncMap.end();++it)
		{
			delete (*it).second;
		}
		nativeFuncMap.clear();
	}

	



	

	Node* Parser::ParseExpression()
	{
		Node* rt = NULL;	
		Node* e = NULL;
 		switch(m_lex->token)
		{
			case TOKEN_NAME:
				e = ParseOrExp();

				if (checkName
					&& e->type == TOKEN_NAME 
					&& finfo->varMap.find(*e->val.s) == finfo->varMap.end())
				{
					error("unknown variable name");
				}

				if(m_lex->token == TOKEN_ASSIGN)
				{
					m_lex->next();
					int theLine = m_lex->lineNum;
					Node* rightSideExp = ParseExpression();
					if (rightSideExp == NULL)
					{
						char str[256];
						sprintf_s(str,256,"This token %s is not supposed to be here",TokenStrList[m_lex->token].c_str());
						error(str);
					}
					rt = new Node(m_lex->val,TOKEN_ASSIGN,e,rightSideExp);
					if(m_lex->lineNum != theLine)
						Expect(TOKEN_SEMICOLON);//same line, need a ;
				}
				else
				{
					rt = e;//only expression
				}
				break;
			case TOKEN_END:
				break;//end of a 'if else' or func or while
			case TOKEN_SEMICOLON:
				break;
			default :
				rt = ParseOrExp();
		}

		return rt;
	}
	Node* Parser::ParseStatement()
	{
		m_lex->next();
		Node* rt = NULL;
		Node* e = NULL;
		Node::VALUE v;
		switch(m_lex->token)
		{
		case TOKEN_VAR:
			//var name
			rt = new Node(m_lex->val,TOKEN_VAR,NULL,NULL);
			m_lex->next();
			if(Expect(TOKEN_NAME))
			{
				e = new Node(m_lex->val,TOKEN_NAME,NULL,NULL); 
				rt->val.child = e;
				rt->hasChild = true;

				m_lex->next();
				Expect(TOKEN_SEMICOLON);
			}
				finfo->numOfVar++;
				finfo->varMap[*e->val.s] = finfo->numOfVar; // offset
			 

			break;
		case TOKEN_IF:
			m_lex->next();
			e = ParseExpression();
			if(Expect(TOKEN_THEN))
			{
				//m_lex->next();
				Node* thenNode = new Node(m_lex->val,TOKEN_THEN,ParseStatement(),NULL); 
				rt = new Node(m_lex->val,TOKEN_IF,e,thenNode); // if e then thenNode->child
				thenNode = thenNode->val.child;

				for(;;)
				{
					


					//m_lex->next();
					thenNode->sibling =  ParseStatement();//multiple statements
					if(thenNode->sibling != NULL)
					{
						thenNode = thenNode->sibling;
					}
					if(  m_lex->token == TOKEN_ELSE || m_lex->token == TOKEN_END) 
						break;
	
				}

			}
			
			

			if(m_lex->token == TOKEN_ELSE)
			{
				e = e->sibling;// e = thenNode
				for(;;)
				{
					if(m_lex->token == TOKEN_END) 
						break;
					//m_lex->next();
					e->sibling =  ParseStatement(); // any else statements are the sibling of 'then'
					e = e->sibling; // this is one of statement list
					if(e == NULL) 
						break;
					//make sure all the statements are sibling of the X in the 'if(X)then Y1 Y2'
				}
			}
			Expect(TOKEN_END);
			break;
		case TOKEN_WHILE: 
			m_lex->next();
			e = ParseExpression();
			if(Expect(TOKEN_DO))
			{
				rt = new Node(m_lex->val,TOKEN_WHILE,e,NULL); // if e then thenNode->child
				//Node* doNode = e->sibling;

				for(;;)
				{
					if(e == NULL || e->type == TOKEN_END) 
						break;
					//m_lex->next();
					e->sibling =  ParseStatement();//multiple statements
					e = e->sibling;
				}

			}
			Expect(TOKEN_END);
			break;
		case TOKEN_BREAK: 
			m_lex->next();
			rt = new Node(m_lex->val,TOKEN_BREAK,NULL,NULL);
			Expect(TOKEN_SEMICOLON);
			break;
		case TOKEN_RETURN:
			m_lex->next();
			e = ParseExpression();
			rt = new Node(m_lex->val,TOKEN_RETURN,e,NULL);
			Expect(TOKEN_SEMICOLON);
			break;
		case TOKEN_FUNC: 
			m_lex->next();
			if(Expect(TOKEN_NAME))
			{
				finfo = new funcInfo();
				v = m_lex->val;//name of func
				
				m_lex->next();
				Expect(TOKEN_LEFTPAR);
				m_lex->next();
				checkName = false;//dont check name for the parameters
				e = ParseExpression();// the 1st parameter
				Node* paraNode = new Node(m_lex->val,TOKEN_PARA,e,NULL);
				rt = new Node(m_lex->val,TOKEN_FUNC,new Node(v,TOKEN_NAME,NULL,NULL),paraNode);

				
				if( e == NULL )
				{
					e = rt->val.child ;
				}
				else
				{
					finfo->numOfPara++;
					finfo->varMap[*e->val.s] = finfo->numOfPara; // offset
				}
				
				if (m_lex->token == TOKEN_COMMA)
				{
					m_lex->next();
				}

				for(;m_lex->token != TOKEN_RIGHTPAR;)
				{				
					Expect(TOKEN_NAME);
					e->sibling = ParseExpression();// other parameters
					e = e->sibling;
					finfo->numOfPara++;
					finfo->varMap[*e->val.s] = finfo->numOfPara; // offset

					if(m_lex->token == TOKEN_COMMA)	m_lex->next(); // for next name
				}

				for(auto it = finfo->varMap.begin();it!=finfo->varMap.end();++it)
				{
					(*it).second += -finfo->numOfPara - 2; //offset !!! -4 -3 -2
				}

				Expect(TOKEN_RIGHTPAR);

				checkName = true;//check for statements again

				for(;;)
				{
					//func body
					if(paraNode == NULL || paraNode->type == TOKEN_END) 
						break;
					//m_lex->next();
					paraNode->sibling =  ParseStatement();//multiple statements
					//why need finfo? because in statements, need all the variables have been declared/ or arguments
					paraNode = paraNode->sibling;

				}
				Expect(TOKEN_END);
				funcInfoMap[*v.s] = finfo;
			}

			
			break;
		//case TOKEN_NAME:
		//	rt = ParseExpression();
		//	break;
		default:
			//Exp
			rt = ParseExpression();
			if(rt)
				rt = new Node(m_lex->val,TOKEN_POP,rt,NULL);
		}
		//Expect(TOKEN_SEMICOLON);
		return rt;
	}




	Node* Parser::ParseOrExp()
	{
		auto e = ParseAndExp();
		while(m_lex->token == TOKEN_OR)
		{
			m_lex->next();
			e = new Node(m_lex->val,TOKEN_OR,e,ParseAndExp());
		}
		return e;
	}

	Node* Parser::ParseAndExp()
	{
		auto e = ParseEqExp();
		while(m_lex->token == TOKEN_AND)
		{
			m_lex->next();
			e = new Node(m_lex->val,TOKEN_AND,e,ParseEqExp());
		}
		return e;
	}



	Node* Parser::ParseEqExp()
	{
		auto e =ParseCompExp();
		while(m_lex->token == TOKEN_EQ || m_lex->token == TOKEN_NOTEQ)
		{
			int tempTk = m_lex->token;
			m_lex->next();
			e = new Node(m_lex->val,tempTk,e,ParseCompExp());
		}
		return e;
	}

	Node* Parser::ParseCompExp()
	{
		auto e = ParsePlusExp();
		while(m_lex->token == TOKEN_GREATER ||
			m_lex->token == TOKEN_LESS ||
			m_lex->token == TOKEN_GREATEREQ ||
			m_lex->token == TOKEN_LESSEQ)
		{
			int tempTK = m_lex->token;
			m_lex->next();
			e = new Node(m_lex->val,tempTK,e,ParsePlusExp());
		}
		return e;
	}

	//Node* Parser::ParsePlusExp() //Right Associative a + ( b + c ), Recursive
	//{
	//	auto e = ParseMulExp();

	//	if(m_lex->token == TOKEN_PLUS || m_lex->token == TOKEN_MINUS )
	//	{
	//		int tempTk = m_lex->token;
	//		m_lex->next();
	//		return new Node(m_lex->val,tempTk,e,ParsePlusExp());
	//	}
	//	return e;
	//}
	 
	Node* Parser::ParsePlusExp()//Left Associative (a + b) + c, loop
	{
		auto e =ParseMulExp();
		while(m_lex->token == TOKEN_PLUS || m_lex->token == TOKEN_MINUS)
		{
			int tempTk = m_lex->token;
			m_lex->next();
			e = new Node(m_lex->val,tempTk,e,ParseMulExp());
		}
		return e;
	}
	 
	Node* Parser::ParseMulExp()
	{
		auto e = ParseUnaryExp();
		while(m_lex->token == TOKEN_MULTIPLY || m_lex->token == TOKEN_DIVIDE)
		{
			int tempTk = m_lex->token;
			m_lex->next();
			e = new Node(m_lex->val,tempTk,e,ParseUnaryExp());
		}
		return e;
	}

	Node* Parser::ParseUnaryExp()
	{
		auto e = ParseFactor();
		while(m_lex->token == TOKEN_UNARY || m_lex->token == TOKEN_NOT)
		{
			int tempTK = m_lex->token;
			m_lex->next();
			e = new Node(m_lex->val,tempTK,ParseFactor(),NULL);
		}
		return e;
	}

	Node* Parser::ParseFactor()
	{
		Node* e = NULL;
		if(m_lex->token == TOKEN_LEFTPAR)
		{
			m_lex->next();
			e = ParseExpression();
			
			Expect(TOKEN_RIGHTPAR);
			m_lex->next();
		}
		else if(m_lex->token == TOKEN_INT || m_lex->token == TOKEN_FLOAT || 
			m_lex->token == TOKEN_TRUE || m_lex->token == TOKEN_FALSE || m_lex->token == TOKEN_STRING)
		{
			e = new Node(m_lex->val,m_lex->token,NULL,NULL);	
			m_lex->next();
		}
		else if(m_lex->token == TOKEN_NAME)
		{
			e = new Node(m_lex->val,TOKEN_NAME,NULL,NULL);
			m_lex->next();

			if (m_lex->token == TOKEN_LEFTPAR)
			{
				auto nf = nativeFuncMap.find(*m_lex->val.s);
				if ( nf != nativeFuncMap.end())
				{
					//native func call
					e = new Node(m_lex->val,TOKEN_NATIVECALL,e,NULL);
				}
				else
				{
					//func call
					e = new Node(m_lex->val,TOKEN_CALL,e,NULL);
				}
				
				m_lex->next();
				if(m_lex->token != TOKEN_RIGHTPAR)
				{
					Node* para = ParseExpression();
					e->val.child->sibling = para;// the 1st parameter 
					if (m_lex->token == TOKEN_COMMA)
					{
						m_lex->next();
					}
					for(;m_lex->token != TOKEN_RIGHTPAR;)
					{				
						para->sibling = ParseExpression();// other parameters
						para = para->sibling;
						if(m_lex->token == TOKEN_COMMA)	m_lex->next(); // for next para
					}
				}

				Expect(TOKEN_RIGHTPAR);
				int theLine = m_lex->lineNum;
				m_lex->next();
				if(m_lex->lineNum != theLine)
					Expect(TOKEN_SEMICOLON);//if this function call is the end of the line, need ;
			}
			else if (m_lex->token == TOKEN_LEFTBRACKET)
			{
				//[] indexing
				int tempTK= m_lex->token;
				m_lex->next();
				e = new Node(m_lex->val,tempTK,e,ParseExpression());//only store a '[' as a node, means indexing
				Expect(TOKEN_RIGHTBRACKET);
				m_lex->next();
			}
			else
			{
				//means it is a variable
				if (checkName
					&& e->type == TOKEN_NAME 
					&& finfo->varMap.find(*e->val.s) == finfo->varMap.end())
				{
					error("unknown variable name");
				}
			}

		}
		else if (m_lex->token == TOKEN_LEFTBRACKET)
		{
			//this is  [1,2,[3,4,5],b]   a kind of vector initialization
			e = new Node(m_lex->val,TOKEN_VECTOR,e,NULL);
			m_lex->next();
			if(m_lex->token != TOKEN_RIGHTBRACKET)
			{
				Node* element = ParseExpression();
				e->val.child = element;// the 1st element 
				e->hasChild = true;
				if (m_lex->token == TOKEN_COMMA)
				{
					m_lex->next();
				}
				for(;m_lex->token != TOKEN_RIGHTBRACKET;)
				{				
					element->sibling = ParseExpression();// other elements
					element = element->sibling;
					if(m_lex->token == TOKEN_COMMA)	m_lex->next(); // for next para
				}
			}

			Expect(TOKEN_RIGHTBRACKET);
			int theLine = m_lex->lineNum;
			m_lex->next();
			if(m_lex->lineNum != theLine)
				Expect(TOKEN_SEMICOLON);//if this function call is the end of the line, need ;
		}
		else if((!m_lex->isLastTokenOfLine() && m_lex->token == TOKEN_SEMICOLON) )
		{
			char str[256];
			sprintf_s(str,256,"This token %s is not supposed to be here",TokenStrList[m_lex->token].c_str());
			error(str);
		}
		
		return e;
	}

	bool Parser::Expect(int t)
	{
		if(m_lex->token != t)
		{
			char str[256];
			sprintf_s(str,256,"Except a %s",TokenStrList[t].c_str());
			error(str);
			errorNum++;
			system("pause");
			return false;
		}
		return true;
		//m_lex->next();
	}

	void Parser::error(char* errmsg)
	{
		char str[256];
		sprintf_s(str,256,"--- Error: Line: %d. %s ---",m_lex->lineNum,errmsg);
		std::cout<<str<<"\n";
		errorNum++;
		system("pause");
	}
}