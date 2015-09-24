
#include "Lex.h"
#include <fstream>
#include <iostream>
#include "Log.h"
using namespace std;

namespace NBE
{
	//std::string LexicalAnalyzer::Keyword[KEYWORDSIZE] = 
	//		{string("true"),string("false"),string("not"),string("if"),
	//		 string("then"),string("elseif"),string("else"),string("end"),string("while"),
	//		 string("do"),string("break"),string("return"),string("func"),string("var")};

	//std::string LexicalAnalyzer::Op[OPERATORSIZE] = 
	//	{string("+"),string("-"),string("*"),string("/"),string("%"),
	//	 string("<"),string(">"),string("<="),string(">="),string("=="),
	//	 string("!="),string("&&"),string("||"),string("!")};

	std::string TokenStrList[TOKENSIZE] =
	{
			//use to output info
			string(""),
			string("root"),
			string("name"),
			string("integer"),
			string("float"),
			string("operator"),
			string("!"),
			string("string"),
			string("call"),
			string("vector"),
						
			string("true"),
			string("false"),
			string("not"),
			string("if"),
			string("then"),
			string("elseif"),
			string("else"),
			string("end"),
			string("while"),
			string("do"),
			string("break"),
			string("return"),
			string("func"),
			string("para"),
			string("var"),
			string(";"),
			string(","),
			string("("),
			string(")"),
			string("["),
			string("]"),
// 			string("{"),
// 			string("}"),
			string("="),
			string("+"),
			string("-"),
			string("*"),
			string("/"),
			string("<"),
			string("<="),
			string(">"),
			string(">="),
			string("=="),
			string("!="),
			string("&&"),
			string("||"),
			string("pop"),
			string("native call")};

	void LexicalAnalyzer::next()
	{
		//only return 1 token
		token = -1;
		for(;token == -1;)
		{
			
			switch(c = *current++)
			{
			case 0: token = TOKEN_EOF; current--; return;
			case '\n': lineNum++; break;
			case '\r': break;//for windows
			case ' ': break;
			case '	': break;
			case '\"': token = TOKEN_STRING; val.s = getStr(current); outputToken( val.s->c_str()); return; // parse string !
			case '(': token = TOKEN_LEFTPAR; break;
			case ')': token = TOKEN_RIGHTPAR; break;
			case '[': token = TOKEN_LEFTBRACKET; break;
			case ']': token = TOKEN_RIGHTBRACKET; break;
// 			case '{': token = TOKEN_LEFTBRACE; break;
// 			case '}': token = TOKEN_RIGHTBRACE; break;
			case ';': token = TOKEN_SEMICOLON; break;
			case ',': token = TOKEN_COMMA; break;

			case '=': if(*current=='=') { current++; token = TOKEN_EQ; break;}
					  else { token = TOKEN_ASSIGN; break; }
			case '+': token = TOKEN_PLUS; break;
			case '-': token = TOKEN_MINUS; break;
			case '*': token = TOKEN_MULTIPLY; break;
			case '/': if(*current == '/'){nextLine();break;}
					  else if(*current == '*'){nextStarSlash();break;}
					  else{token = TOKEN_DIVIDE; break;}
			case '<': if(*current == '='){current++; token = TOKEN_LESSEQ; break;}
					  else{token = TOKEN_LESS; break;}
			case '>': if(*current == '='){current++; token = TOKEN_GREATEREQ; break;}
					  else{token = TOKEN_GREATER; break;}
			case '!': if(*current == '='){current++; token = TOKEN_NOTEQ; break;}
					  else{token = TOKEN_UNARY; break;}
			case '&': if(*current == '&'){current++; token = TOKEN_AND; break;}
					  else{}
			case '|': if(*current == '|'){current++; token = TOKEN_OR; break;}


			default:

				if(c < 0)
				{
					error("invalid character");
					return ;
				}
				if(isalpha(c)|| c=='_')
				{
					token = TOKEN_NAME;
					char name[256];
					sprintf_s(name,256,"%c",c);
					while(isalpha(*current) || isdigit(*current) || *current == '_' )
					{
						int len = strlen(name);
						name[len] = *current;
						name[len+1] = '\0';
						++current;
						if(*current < 0)
						{
							error("invalid character");
							break ;
						}
					}
					 
					isAToken(string(name),token);
					if(token == TOKEN_NAME)
					{
						val.s = new string(name);
					}
				 

					outputToken(name);
					//token = -1;
					return;//continue; 
				}
				else if(isdigit(c)|| (c == '.' )) 
				{ 
					bool hasDot = false;
					
					char digit[32];
					sprintf_s(digit,32,"%c",c);
					while( isdigit(*current) || (*current == '.' && !hasDot) )
					{
						if(*current == '.') 
							hasDot = true;
						 
						 
						int len = strlen(digit);
						digit[len] = *current;
						digit[len+1] = '\0';
						++current;
						if(*current < 0)
						{
							error("invalid character");
							break ;
						}
					}
			
		
					if(hasDot)
					{
						token = TOKEN_FLOAT;
						val.f = static_cast<float>(atof(digit));
					}
					else
					{
						val.i = atoi(digit);
						token = TOKEN_INT;
					}

					outputToken(digit);
					//token = -1;
					return;//continue;
				}
				else
				{
					//error
					char unknownC[32];
					sprintf_s(unknownC,32,"unknown character: %c",c);
					error(unknownC);
					return;//continue;
				}

			}
			if(token > -1)
			{
				outputToken(TokenStrList[token].c_str());
				//token = -1;
			}
		}
	}

	bool LexicalAnalyzer::isAToken(string& str, int& tk)
	{
		for(int i = 0; i < TOKENSIZE; ++i)
		{
			if(str == TokenStrList[i])
			{
				//outputToken(str.c_str());
				tk = i;
				return true;
				
			}
		}
		return false;
	}

	
	void LexicalAnalyzer::nextLine()
	{
		while(c = *current++)
		{
			if (c == '\n' || *current == 0)
			{
				break;
			}
		}
	}

	void LexicalAnalyzer::nextStarSlash()
	{
		while(c = *current++)
		{
			if (c == '*' && *current == '/')
			{
				current += 2;
				break;
			}
		}
	}

	string* LexicalAnalyzer::getStr(char*& current)
	{
		string* prtv= new string("");
		for(;;)
		{
			if(*current == '\"')
			{
				++current;
				//*prtv += "\"";
				return prtv;
			}
			else
			{
				*prtv += *current;
			}
			++current;
			
		}
		delete prtv;
		error("need another '\"'");
	}

	void LexicalAnalyzer::outputToken(const char* tk)
	{
		char str[256];
		sprintf_s(str,256,"Line: %d, token: %s",lineNum,tk);
		cout<<str<<"\n";
		Log::writeLog(string(str) + "\n");
	}

	int LexicalAnalyzer::load(TCHAR* fileName)
	{
		std::ifstream *ifs = new std::ifstream(fileName,std::ios_base::binary);
		if(!ifs->fail())
		{
			ifs->seekg(0,std::ios::end);
			bufferSize = (int)ifs->tellg();
			ifs->seekg(0,std::ios::beg);
		}
		else
		{
			cout<<"read file error!\n";
			system("pause");
			exit(0);
		}
		if(bufferSize > 0)
		{
			buffer = new char[bufferSize+1];
			ifs->read(buffer,bufferSize);
			buffer[bufferSize] = 0;
			current = buffer;
		}
		ifs->close();
		delete ifs;
		return 0;
	}
	void LexicalAnalyzer::error(char* errmsg)
	{
		char str[256];
		sprintf_s(str,256,"--- Error: Line: %d. %s ---",lineNum,errmsg);
		std::cout<<str<<"\n";
		Log::writeLog(string(str) + "\n");
	}

	bool LexicalAnalyzer::isLastTokenOfLine()
	{
		return *current == '\r' || *current == '\n' || *current == 0;
	}

}