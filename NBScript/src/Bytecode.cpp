#include "Node.h"
#include <stack>
#include <vector>


#include "Bytecode.h"
#include <iostream>
#include "Log.h"

//#define DATASIZE 2048
//#define CODESTART 2048
namespace NBE
{
	string commandNameStrList[C_TOTALCOMNUM] = 
	{
		string("push float"),string("push int"),string("push boolean"),string("push string"),string("push name"),
		string("!"),string("||"),string("&&"),string("=="),string("!="),string(">"),string("<"),string(">="),string("<="),string("+"),
		string("-"),string("*"),string("/"),string("="),string("[]="),string("var"),string("["),string("vector"),string("call"),string("native call"),string("func"),string("para"),string("local"),
		string("break"),string("return"),string("return_noval"),string("ifne"),string("no command"),string("jump"),string("pop"),string("exit")
	};

	void ByteCode::generateFromNode(Node* nd,funcInfo* finfo)
	{
		if (nd)
		{
			byte cType = 255;
			byte* dataArray = NULL;
			int lenOfArray = 0;

			int tempInt = 0;
			int varCount = 0;//in a func, need to know how many variables it has

			switch(nd->type)
			{
			case TOKEN_UNARY:
			case TOKEN_NOT:
				cType = C_NOT;
				break;

			case TOKEN_OR: cType = C_OR; break;
			case TOKEN_AND: cType = C_AND; break;
			case TOKEN_EQ: cType = C_EQ; break;
			case TOKEN_NOTEQ: cType = C_NOTEQ; break;
			case TOKEN_GREATER: cType = C_GREATER; break;
			case TOKEN_LESS: cType = C_LESS; break;
			case TOKEN_GREATEREQ: cType = C_GREATEREQ; break;
			case TOKEN_LESSEQ: cType = C_LESSEQ; break;
			case TOKEN_PLUS: cType = C_PLUS; break;
			case TOKEN_MINUS: cType = C_MINUS; break;
			case TOKEN_MULTIPLY: cType = C_MULTIPLY; break;
			case TOKEN_DIVIDE: cType = C_DIVIDE; break;
			case TOKEN_ASSIGN:
				if (nd->val.child->type == TOKEN_LEFTBRACKET)
				{
					cType = C_ASSIDX; 
					generateFromNode(nd->val.child->val.child->sibling ,finfo);//get the index num
					generateFromNode(nd->val.child->sibling,finfo);//get the value
					lenOfArray = sizeof(int);
					dataArray = new byte[lenOfArray];
					memcpy_s(dataArray,lenOfArray,&finfo->varMap[*nd->val.child->val.child->val.s],lenOfArray);
					pushData(&cType,dataArray,lenOfArray);
					delete[] dataArray;
					cType = C_NONE;
				}
				else
				{
					cType = C_ASSIGN; 
					generateFromNode(nd->val.child->sibling,finfo);
					lenOfArray = sizeof(int);
					dataArray = new byte[lenOfArray];
					memcpy_s(dataArray,lenOfArray,&finfo->varMap[*nd->val.child->val.s],lenOfArray);
					pushData(&cType,dataArray,lenOfArray);
					delete[] dataArray;
					cType = C_NONE;
				}
				break;

			case TOKEN_INT: 
				cType = C_PUSH_INT; 
				lenOfArray = sizeof(int);
				dataArray = new byte[lenOfArray];
				memcpy_s(dataArray,lenOfArray,&nd->val.i,lenOfArray);
				break;
			case TOKEN_FLOAT:
				cType = C_PUSH_FLOAT; 
				lenOfArray = sizeof(float);
				dataArray = new byte[lenOfArray];
				memcpy_s(dataArray,lenOfArray,&nd->val.f,lenOfArray);
				break;
			case TOKEN_TRUE:
				lenOfArray = sizeof(int);
				cType = C_PUSH_BOOLEAN;
				dataArray = new byte[lenOfArray];
				tempInt = 0;
				memcpy_s(dataArray,lenOfArray,&tempInt,lenOfArray);
				break;
			case TOKEN_FALSE: 
				lenOfArray = sizeof(int);
				cType = C_PUSH_BOOLEAN;
				dataArray = new byte[lenOfArray];
				tempInt = 1;
				memcpy_s(dataArray,lenOfArray,&tempInt,lenOfArray);
				break;
			case TOKEN_VECTOR://not indexing
				cType = C_VECTOR;
				lenOfArray = sizeof(unsigned int);
				dataArray = new byte[lenOfArray];
				tempInt = getVecLen(nd);
				memcpy_s(dataArray,lenOfArray,&tempInt,lenOfArray);//get the node's children & sibling number
				 
				break;
			case TOKEN_STRING:
				lenOfArray = nd->val.s->size() + 1;
				cType = C_PUSH_STRING;
				dataArray = new byte[lenOfArray];
				memcpy_s(dataArray,lenOfArray,nd->val.s->c_str(),lenOfArray);
				dataArray[lenOfArray - 1] = 0;
				break;
			case TOKEN_VAR:
				cType = C_NONE;
				break;//variable definition
			case TOKEN_NAME:
				cType = C_PUSH_NAME;
					lenOfArray = sizeof(int);
					dataArray = new byte[lenOfArray];
					memcpy_s(dataArray,lenOfArray,&finfo->varMap[*nd->val.s],lenOfArray);
				break;//variable

			case TOKEN_LEFTBRACKET:
				cType = C_INDEXING;
				break;
			case TOKEN_CALL:
				cType = C_CALL;

				if (nd->hasChild)
				{
					auto funcMap = ps->getFuncInfoMap();
					auto fi = funcMap.find(*nd->val.child->val.s);
					if(fi == funcMap.end())
					{
						cout<<"-- error: unknown function call --\n";
						Log::writeLog("-- error: unknown function call --\n");
						system("pause");
						exit(0);
					}


					
					Node* paraNd = nd->val.child->sibling;//first parameters
					
		
					generateFromNode(paraNd,finfo);//push all the parameters
					
					int paraCount = 0;
					while(paraNd)
					{
						paraCount++;
						paraNd = paraNd ->sibling;
					}

					if (paraCount != (*fi).second->numOfPara)
					{
						cout<<"-- error: func parameters doesn't match\n";
						Log::writeLog("-- error: func parameters doesn't match\n");
						return;
					}


					//cout<<"<"<<dec<<m_byteCode.size()<<">";
					genCommandCode(&cType,1);
					
					lenOfArray = sizeof(unsigned int);
					dataArray = new byte[lenOfArray];

					funcInfo* callFuncInfo = ps->getFuncInfoMap()[*nd->val.child->val.s];
					if(callFuncInfo->defined)
					{
						memcpy_s(dataArray,lenOfArray,&callFuncInfo->funcAddress,lenOfArray);
					}
					else
					{
						m_funcCallRequests.push_back(funcReq(*nd->val.child->val.s,m_byteCode.size()));
						memset(dataArray,0,lenOfArray);
					}
					

					
					//cout<<"[";
					genDataCode(dataArray,lenOfArray);// address of func
					
					memcpy_s(dataArray,lenOfArray,&(callFuncInfo->numOfVar),lenOfArray);
					genDataCode(dataArray,lenOfArray);// num of variable in func
					delete[] dataArray;
					//cout<<"]\n";

				}
				cType = C_NONE;
				break;
			case TOKEN_NATIVECALL:
				cType = C_NATIVECALL;

				if (nd->hasChild)
				{
					Node* paraNd = nd->val.child->sibling;//first parameters


					generateFromNode(paraNd,finfo);//push all the parameters
					int paraCount = 0;
					while(paraNd)
					{
						paraCount++;
						paraNd = paraNd ->sibling;
					}

					lenOfArray = sizeof(unsigned int);
					dataArray = new byte[lenOfArray];
					memcpy_s(dataArray,lenOfArray,&nativeFuncMap[*nd->val.child->val.s]->id,lenOfArray);
					genCommandCode(&cType,1);//NATIVE CALL
					genDataCode(dataArray,lenOfArray);//native function id
					memcpy_s(dataArray,lenOfArray,&paraCount,lenOfArray);
					genDataCode(dataArray,lenOfArray);//how many parameters pushed
					delete[] dataArray;
				}
				cType = C_NONE;
				break;
			case TOKEN_FUNC:
				cType = C_NONE;
				generateFunc(nd->val.child);

				break;
			case TOKEN_BREAK:
				{
				cType = C_JUMP;
				//cout<<"<"<<dec<<m_byteCode.size()<<">";
				genCommandCode(&cType,1);
				m_breakRequests.push_back( m_byteCode.size() );//this is jumping to end while
				lenOfArray = sizeof(unsigned int);
				dataArray = new byte[lenOfArray];
				memset(dataArray,0,lenOfArray);
				//cout<<"[";
				genDataCode(dataArray,lenOfArray);
				delete[] dataArray;
				//cout<<"]\n";
				cType = C_BREAK;
				}
				break;
			case TOKEN_RETURN:
				finfo->hasReturn = true;
				if(nd->hasChild)
				{
					cType = C_RETURN;
					//finfo->returnNull = false;//means return a real value
					generateFromNode(nd->val.child,finfo);
					//cout<<"<"<<dec<<m_byteCode.size()<<">";
					genCommandCode(&cType,1);

				}
				else
				{
					cType = C_RETURN_NO_VALUE;
					//cout<<"<"<<dec<<m_byteCode.size()<<">";
					genCommandCode(&cType,1);
				}
				

				lenOfArray = sizeof(unsigned int);
				dataArray = new byte[lenOfArray];
				memcpy_s(dataArray,lenOfArray,&finfo->numOfPara,lenOfArray);
				//cout<<"[";
				genDataCode(dataArray,lenOfArray);// num of para in func
				delete[] dataArray;
				//cout<<"]\n";

				cType = C_NONE;
				break;
			case TOKEN_IF:
				cType = C_IFNE;


				if(nd->hasChild)
				{
					generateFromNode(nd->val.child,finfo); //condition
				}



				cType = C_NONE;
				break;
			case TOKEN_THEN:
				{
				//cout<<"<"<<dec<<m_byteCode.size()<<">";
				cType = C_IFNE;
				genCommandCode(&cType,1);

				unsigned int loc1 = m_byteCode.size();//this is jumping to else statements
				lenOfArray = sizeof(unsigned int);
				dataArray = new byte[lenOfArray];
				memset(dataArray,0,lenOfArray);
				//cout<<"[";
				genDataCode(dataArray,lenOfArray);// num of para in func
				delete[] dataArray;
				//cout<<"]\n";

				
				generateFromNode(nd->val.child,finfo);
				cType = C_JUMP;
				//cout<<"<"<<dec<<m_byteCode.size()<<">";
				genCommandCode(&cType,1);
				unsigned int loc2 = m_byteCode.size();//this is jumping to end of else
				lenOfArray = sizeof(unsigned int);
				dataArray = new byte[lenOfArray];
				memset(dataArray,0,lenOfArray);
				//cout<<"[";
				genDataCode(dataArray,lenOfArray);//
				delete[] dataArray;
				//cout<<"]\n";

				unsigned int currentAddress = m_byteCode.size();
				memcpy_s(&m_byteCode[loc1],4,&currentAddress,4);
				//cout<<"re-locate "<<dec<<loc1<<" the ifne address"<<currentAddress<<"\n";
				
				if (nd->sibling)
				{
					generateFromNode(nd->sibling,finfo);
				}

				currentAddress = m_byteCode.size();
				memcpy_s(&m_byteCode[loc2],4,&currentAddress,4);
				 
				//cout<<"re-locate"<<dec<<loc2<<" the then end jump address"<<currentAddress<<"\n";
				
				cType = C_NONE;
				return;
				}
			case TOKEN_WHILE:
				
				if(nd->hasChild)
				{
					unsigned int loc_whileStart = m_byteCode.size();
					Node* doNode = nd->val.child->sibling;
					nd->val.child->sibling = NULL;
					generateFromNode(nd->val.child,finfo); //loop condition

					//cout<<"<"<<dec<<m_byteCode.size()<<">";
					cType = C_IFNE;
					genCommandCode(&cType,1);

					unsigned int loc1 = m_byteCode.size();//this is jumping out of the loop
					lenOfArray = sizeof(unsigned int);
					dataArray = new byte[lenOfArray];
					memset(dataArray,0,lenOfArray);
					//cout<<"[";
					genDataCode(dataArray,lenOfArray);// num of para in func
					delete[] dataArray;
					//cout<<"]\n";

					if(doNode)
					{
						generateFromNode(doNode,finfo);	
						nd->val.child->sibling = doNode;
					}
					cType = C_JUMP;
					//cout<<"<"<<dec<<m_byteCode.size()<<">";
					genCommandCode(&cType,1);
					lenOfArray = sizeof(unsigned int);
					dataArray = new byte[lenOfArray];
					memcpy_s(dataArray,lenOfArray,&loc_whileStart,lenOfArray);
					//cout<<"[";
					genDataCode(dataArray,lenOfArray);// num of para in func
					delete[] dataArray;
					//cout<<"]\n";
					
					unsigned int currentAddress = m_byteCode.size();
					memcpy_s(&m_byteCode[loc1],4,&currentAddress,4);
					//cout<<"re-locate"<<dec<<loc1<<" the end of while address "<<currentAddress<<"\n";
					
					//re locate all the 'break'
					for (unsigned int i = 0; i< m_breakRequests.size(); ++i)
					{
						memcpy_s(&m_byteCode[ m_breakRequests[i] ],4,&currentAddress,4);
						//cout<<"re-locate"<<dec<<m_breakRequests[i]<<" break -> jump address "<<currentAddress<<"\n";

					}
					
					m_breakRequests.clear();

				}
				cType = C_NONE;
				break;

			case TOKEN_POP:
				cType = C_POP;
				break;
			}
			
			if (cType == 255)
			{
				cout << "-- error: command type --\n";
				Log::writeLog("-- error: command type --\n");
			}

			if(cType >= C_NOT )
			{
				// not data
				if(nd->hasChild && cType != C_FUNC && cType != C_NONE) // function call has special solution
				{
					generateFromNode(nd->val.child,finfo);
				}



				//command
				if(cType != C_NONE && cType != C_THEN)
				{
					//cout<<"<"<<dec<<m_byteCode.size()<<">";
					genCommandCode(&cType,1);


					if(cType == C_VECTOR)
					{
						//push the length of vector or length of the call parameters, they have similar logic
						//length
						//cout<<"[";
						genDataCode(dataArray,lenOfArray);
						delete[] dataArray;
						//cout<<"]";

					}
					//cout<<"\n";
				}
				
				if (nd->sibling)
				{
					generateFromNode(nd->sibling,finfo);
				}


			}
			else //if(cType <= C_PUSH_NAME)
			{
				pushData(&cType,dataArray,lenOfArray);
				delete[] dataArray;

 				if (nd->sibling)
 				{
 					generateFromNode(nd->sibling,finfo);
 				}

			}


			
		}
	}


	void ByteCode::generateFunc(Node* nd)
	{
		if (nd)
		{
			byte* dataArray = NULL;
			int lenOfArray = 0;

			auto finfomap = ps->getFuncInfoMap();
			//name of this func
			
			unsigned int address = m_byteCode.size();
			finfomap[*nd->val.s]->defined = true;
			finfomap[*nd->val.s]->funcAddress = address;

			auto it = m_funcCallRequests.begin();
			for (;it!= m_funcCallRequests.end(); )
			{
				if(it->name == *nd->val.s)
				{
					//cout<<"re-locate"<<dec<<it->loc<<" the call func address\n";
					memcpy_s(&m_byteCode[it->loc],sizeof(unsigned int), &address,sizeof(unsigned int));
				}
				 
				++it;
				 
			}
				
			 

			Node* bodyNode = nd->sibling->sibling;
				 
			funcInfo* fi = finfomap[*nd->val.s];
			generateFromNode(bodyNode,fi);
			 
			if(!fi->hasReturn)
			{
				unsigned char cType = C_RETURN_NO_VALUE;
				//cout<<"<"<<dec<<m_byteCode.size()<<">";
				genCommandCode(&cType,1);
				
				lenOfArray = sizeof(unsigned int);
				dataArray = new byte[lenOfArray];
				memcpy_s(dataArray,lenOfArray,&fi->numOfPara,lenOfArray);
				//cout<<"[";
				genDataCode(dataArray,lenOfArray);// num of para in func
				delete[] dataArray;
				//cout<<"]\n";

				//cout<<"automatically add return\n";
				if(*nd->val.s != "main")
				{
					cout<<"warning: function "<<*nd->val.s<<" doesn't have a return value, system will return null\n";
					Log::writeLog(string("warning: function ") + *nd->val.s + string(" doesn't have a return value, system will return null\n"));
					system("pause");
				}
			}
			
			
		}
	}

	int ByteCode::initializeFuncParaOffset(Node* nd,unordered_map<string,int>* pFuncParaMap)
	{
		int lenOfArray = sizeof(unsigned int);
		byte* dataArray = new byte[lenOfArray];
		int paraCount = getVecLen(nd->sibling);
		memcpy_s(dataArray,lenOfArray,&paraCount,lenOfArray);//get the node's children & sibling number

		unsigned char cmdType = C_PARA;
		pushData(&cmdType,dataArray,lenOfArray);
		delete[] dataArray;

		if(paraCount > 0)//to cal the num of parameters
		{
			//have at least 1 parameter
			Node*paraNode = nd->sibling;
			if (paraNode->hasChild) 
			{
				paraNode = paraNode->val.child;
				(*pFuncParaMap)[*paraNode->val.s] = -paraCount - 1; //this is the parameter offset
				paraCount--;
			}
			for (;;)
			{
				if (paraNode->sibling)
				{
					paraNode = paraNode->sibling;
					(*pFuncParaMap)[*paraNode->val.s] = -paraCount - 1;
					paraCount--;
				}
				else
				{
					break;
				}
			}
		}
		return paraCount;
	}

	int ByteCode::initializeFuncVarOffset(Node* bodyNode,unordered_map<string,int>* pFuncParaMap)
	{
		int varCount = 0;
		if (bodyNode)
		{
			for (;;)
			{
				if(bodyNode->type == TOKEN_VAR)
				{
					(*pFuncParaMap)[string(*bodyNode->val.child->val.s)] = ++varCount ;
				}
				if (bodyNode->sibling)
				{
					bodyNode = bodyNode->sibling;
				}
				else
				{
					break;
				}
			}	
		}
		int lenOfArray = sizeof(unsigned int);
		byte* dataArray = new byte[lenOfArray];
		memcpy_s(dataArray,lenOfArray,&varCount,lenOfArray);//get the node's children & sibling number

		unsigned char cmdType = C_LOCALVAR;
		pushData(&cmdType,dataArray,lenOfArray);
		delete[] dataArray;

		return varCount;
	}

	void ByteCode::pushData(byte* commandT, byte* arr, int len)
	{
		//this is an output
		//cout<<"<"<<dec<<m_byteCode.size()<<">";
		genCommandCode(commandT,1);

		//data
		//cout<<"[";
		genDataCode(arr,len);
		//cout<<"]\n";
	}

	unsigned int ByteCode::getVecLen(Node* nd)
	{
		unsigned int numOfChildren = 0;
		if (nd->hasChild) 
		{
			numOfChildren++;
			nd = nd->val.child;
		
			for (;;)
			{
				if (nd->sibling)
				{
					numOfChildren++;
					nd = nd->sibling;
				}
				else
				{
					break;
				}
			}
		}
		return numOfChildren;
	}

	void ByteCode::genCommandCode(byte* btarray, int len)
	{
		for (int i =0 ; i< len; ++i)
		{
			m_byteCode.push_back(btarray[i]);
			//cout<<commandNameStrList[btarray[i]];
		}
	}

	void ByteCode::genDataCode(byte* btarray, int len)
	{
		for (int i =0 ; i< len; ++i)
		{
			m_byteCode.push_back(btarray[i]);
			//cout<<hex<<(int)btarray[i];
		}
	}
 

	ByteCode::ByteCode(Parser* paser, NativeFuncMap& nfmap):ps(paser),nativeFuncMap(nfmap)
	{
		m_funcCallRequests.push_back(funcReq("main",1));
	}

	ByteCode::~ByteCode()
	{
		m_byteCode.clear();
	}


	void ByteCode::output()
	{
		for(int i =0; i<(int)m_byteCode.size(); ++i)
		{
			int dataLen = 0;
			cout<<dec<<i<<": "<<commandNameStrList[m_byteCode[i]]<<" ";
			Log::writeLog(to_string(i) + string(": ") + commandNameStrList[m_byteCode[i]] + string(" "));
			switch(m_byteCode[i])
			{
			case C_PUSH_FLOAT:
				cout<< *(float*)&m_byteCode[++i]<<"\n";
				Log::writeLog(to_string(*(float*)&m_byteCode[++i]) + "\n");
				i+= 3;
				continue;
			case C_PUSH_INT:
			case C_PUSH_BOOLEAN:dataLen = 4;break;
			case C_PUSH_STRING:
				for (;m_byteCode[++i] != 0;)
				{
					cout<<m_byteCode[i];
					string str; str.push_back(m_byteCode[i]);
					Log::writeLog(str);
				}
				cout<<"\n";
				Log::writeLog("\n");
				continue;//until read a 0
			case C_PUSH_NAME:dataLen = 4;break;//variable
			case C_NOT:break;// not !
			case C_OR:break;// ||
			case C_AND:break;// &&
			case C_EQ:break;// ==
			case C_NOTEQ:break;// !=
			case C_GREATER:break;// >
			case C_LESS:break;// <
			case C_GREATEREQ:break;// >=
			case C_LESSEQ:break;// <=
			case C_PLUS:break;// +
			case C_MINUS:break;// -
			case C_MULTIPLY:break;// *
			case C_DIVIDE:break;// /
			case C_ASSIGN://dataLen = 4;break;// =
			case C_ASSIDX:dataLen = 4;break;// assign to a[0]
			case C_VAR:break;//var
			case C_INDEXING:break;// [
			case C_VECTOR:dataLen = 4;break;
			case C_CALL:dataLen = 8;break;// function call
			case C_NATIVECALL:dataLen = 8;break;//native function numOfPara, function id
			case C_FUNC:break;//function definition
			case C_PARA:break;//indicate the parameters
			case C_LOCALVAR:break;//var number in func
			case C_RETURN_NO_VALUE:
			case C_RETURN:dataLen = 4;break;//function return num
			case C_IFNE:dataLen = 4;break;// if not equal
			case C_THEN:break;//no command
			case C_JUMP:dataLen = 4;break;//jump command
			case C_POP:break;
			case C_EXIT:break;
			}
			
			if (dataLen > 0)
			{
				++i;
				for (int j = 0; j < dataLen/4; ++j)
				{
					cout<< *(int*)&m_byteCode[i + j*4]<<"  ";
					Log::writeLog(to_string(*(int*)&m_byteCode[i + j * 4]) + "  ");
				}
				i+= dataLen - 1;
			}
			cout<<"\n";
			Log::writeLog("\n");
		}
	}

	void ByteCode::generate(Node* nd)
	{
		//start generating

		//always call main() at the beginning
		auto fimap =  ps->getFuncInfoMap();
		if(fimap.find("main") == fimap.end())
		{
			cout<<"need a main function\n";
			Log::writeLog("need a main function\n");
			return;
		}

		unsigned char commandType = C_CALL; 
		int loc_main_address[2] = {0};
		loc_main_address[1] = ps->getFuncInfoMap()["main"]->numOfVar;
		pushData(&commandType,(unsigned char*)loc_main_address,sizeof(unsigned int)*2);

		commandType = C_EXIT;//byebye
		//cout<<"<"<<dec<<m_byteCode.size()<<">";
		genCommandCode(&commandType,1);
		//cout<<"\n";
		//traverse tree 
		generateFromNode(nd,NULL);
	}

}