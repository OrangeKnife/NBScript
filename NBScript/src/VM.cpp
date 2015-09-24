#include "Bytecode.h"
#include "VM.h"
#include <iostream>
#include "Log.h"

#define OP(OPER) unsigned int sizeOfStack = stack.size();\
	Value &a = stack[sizeOfStack - 2];\
	Value &b = stack[sizeOfStack - 1];\
	if( a.type == INT && b.type == FLOAT){\
		a.type = FLOAT; float tempF = (float)a.ival; a.fval = tempF;\
	}\
	else if( a.type == FLOAT && b.type == INT ){\
		b.type = FLOAT; float tempF = (float)b.ival; b.fval = tempF;\
	}\
	else if(a.type != b.type ){\
	runTimeError("can't operate 2 different types values");\
	return;}\
	if(a.type == INT){\
		a.ival = a.ival OPER b.ival;}\
	else if(a.type == FLOAT){\
		a.fval = a.fval OPER b.fval;}\
	else if(a.type == STRING && memcmp(#OPER,"+",1) == 0){\
		int leftLen = strlen(a.sval->str);\
		int rightLen = strlen(b.sval->str);\
		char* newStr = new char[leftLen + rightLen + 1];\
		memcpy_s(newStr,leftLen,a.sval->str,leftLen);\
		memcpy_s(newStr+leftLen,rightLen,b.sval->str,rightLen);\
		newStr[leftLen + rightLen] = 0;\
		delete a.sval->str; a.sval->str = newStr;\
	}\
	else{\
	runTimeError("operator invalid");\
	}\
	pop(sizeOfStack);

#define LOGICOP(OPER) unsigned int sizeOfStack = stack.size();\
					  Value &a = stack[sizeOfStack - 2];\
					  Value &b = stack[sizeOfStack - 1];\
					  if( a.type == BOOL  && b.type == BOOL ){\
							a.ival = a.ival OPER b.ival ? 1 : 0;\
							pop(sizeOfStack);}\
						else{\
						runTimeError("can't compute logic for these 2 types");\
						return;}
#define CMPOP(OPER) unsigned int sizeOfStack = stack.size();\
					Value &a = stack[sizeOfStack - 2];\
					Value &b = stack[sizeOfStack - 1];\
					if(a.type == INT && b.type == INT){\
						a.type = BOOL; \
						a.ival = a.ival OPER b.ival ? 1 : 0;\
						pop(sizeOfStack);}\
					else if(a.type == FLOAT && b.type == FLOAT){\
						a.type = BOOL; \
						a.ival = a.fval OPER b.fval ? 1 : 0;\
						pop(sizeOfStack);}\
					else if( a.type == INT && b.type == FLOAT ){\
						a.type = BOOL; \
						a.ival = a.ival OPER b.fval ? 1 : 0;\
						pop(sizeOfStack);}\
					else if(a.type == FLOAT && b.type == INT){\
						a.type = BOOL;\
						a.ival = a.fval OPER b.ival ? 1 : 0;\
						pop(sizeOfStack);}\
					else{\
					runTimeError("can't compare these 2 types");\
					return;} 


namespace NBE
{
	void VM::execute()
	{
		std::cout<<"---------------run time---------------\n";
		Log::writeLog("---------------run time---------------\n");
		int lenOfDataFollowed = 0;
		byte* buffer = NULL;
		for(;;buffer = NULL/*,cout<<dec<<IP<<"--"*/)switch(m_code[IP++])
		{
		    case C_PUSH_FLOAT:
				buffer = getDataFromByteCode(4);
				stack.push_back(Value(FLOAT,*(float*)(buffer)));
				break;
			case C_PUSH_INT:
				buffer = getDataFromByteCode(4);
				stack.push_back(Value(INT,*(int*)buffer));
				break;
			case C_PUSH_BOOLEAN:
				buffer = getDataFromByteCode(4);break;
			case C_PUSH_STRING:
				{
				//cout<<dec<<IP<<": "<<commandNameStrList[m_code[IP]]<<" ";
				int startIP = IP;
				for (;m_code[IP] != 0;++IP)
				{
					//cout<<m_code[IP];
				}
				int stringlength = IP - startIP + 1;
				char* str = new char[stringlength];
				memcpy_s(str,stringlength,&m_code[startIP],stringlength);
				//cout<<"\n";
				++IP;// now it is end of str, is a 0


				stack.push_back( Value(STRING,new Value::ValStr(str)) );
				Value::g_string_in ++;
				break;
				}
			case C_PUSH_NAME:
				{
				push( getLocal());
				break;//variable
				}
			case C_NOT:
				{
					Value& boolValue = stack.back();
					if (boolValue.type == BOOL)
					{
						boolValue.ival = boolValue.ival == 0 ? 1 : 0;
					}
				}
				break;// not !
			case C_OR:
				{
					LOGICOP(||)
				}
				break;// ||
			case C_AND:
				{
					 LOGICOP(&&)
 
				}
				break;// &&
			case C_EQ:
				{
					LOGICOP(==)
				}
				break;// ==
			case C_NOTEQ:
				{
					LOGICOP(!=)
				}
				break;// !=
			case C_GREATER:
				{
					CMPOP(>)
				}
				break;// >
			case C_LESS:
				{
					CMPOP(<)
				}
				break;// <
			case C_GREATEREQ:
				{
					CMPOP(>=)
				}
				break;// >=
			case C_LESSEQ:
				{
					CMPOP(<=)
				}
				break;// <=
			case C_PLUS:
				{
					OP(+)
				}
				break;// +
			case C_MINUS:
				{
					OP(-)
				}
				break;// -
			case C_MULTIPLY:
				{
					OP(*)
				}
				break;// *
			case C_DIVIDE:
				{
					OP(/)
				}
				break;// /
			case C_ASSIGN:
				{
					buffer = getDataFromByteCode(4);
					int offset = *(int*)buffer;
					unsigned int sizeOfStack = stack.size();
					Value &a = stack[offset + FP];
 
					Value &b = stack[sizeOfStack - 1];
	 
					a.deRef();
					b.inRef();
					a = b;
					
				}
				break;// =
			case C_ASSIDX:
				{
					//cout<<"assign to vector"<<"\n";
					buffer = getDataFromByteCode(4);
					int offset = *(int*)buffer;
					unsigned int sizeOfStack = stack.size();
					Value &a = ( stack[offset + FP] ) ;
					Value &idx = stack[sizeOfStack - 2];
					Value &b = stack[sizeOfStack - 1];
					
					int index = idx.ival;
					(*a.vval->vec)[index].deRef();
					b.inRef();
					(*a.vval->vec)[index] = b;
					pop(sizeOfStack);//pop the value
					pop(sizeOfStack - 1);// pop the idx
					push((*a.vval->vec)[index]);
					
				}
				break;
			//case C_VAR:break;//var
			case C_INDEXING:
				{
					//cout<<"indexing:";
					unsigned int sizeOfStack = stack.size();
					Value &a = stack[sizeOfStack - 2];
					Value &b = stack[sizeOfStack - 1];
					if(b.ival >= (int)a.vval->vec->size() || b.ival < 0 )
					{
						runTimeError("vector over bound");
						return;
					}
					Value indexV = (*a.vval->vec)[b.ival];
					pop(sizeOfStack);
					pop(sizeOfStack - 1);
					push( indexV );
					//cout<<stack.back().sval<<"\n";
				}
				break;// [
			case C_VECTOR:
				{
					buffer = getDataFromByteCode(4);
					unsigned int sizeVec = *(unsigned int*)buffer;
					
					vector<Value>* pVec = new vector<Value>();
					pVec->resize(sizeVec);
					//memcpy_s(&pVec->front(),sizeof(Value) * sizeVec,&stack[stack.size()-sizeVec],sizeof(Value) * sizeVec);
					for (unsigned int i = 0; i< sizeVec; ++i)
					{
						(*pVec)[sizeVec - i -1] = stack.back();
						(*pVec)[sizeVec - i -1].inRef();
						pop(stack.size());
					}

					stack.push_back( Value(VECTOR,new Value::ValVec(pVec)) );
					Value::g_vector_in++;
				}
				break;
			case C_CALL:
				buffer = getDataFromByteCode(8);// function call, includes main
				{
				stack.push_back(Value(RTADDRESS, IP));
			
				IP = *(unsigned int*)buffer;
				unsigned int numOfVar = *(unsigned int*)(buffer + 4);
				
				stack.push_back(Value (PARENTFP,FP));
				 FP = stack.size() - 1;
				stack.resize( FP + 1 + numOfVar);
				
				}
				break;
			case C_NATIVECALL:
				{
				buffer = getDataFromByteCode(8);
				NativeFunc* nf = getNativeFuncById(*(unsigned int*)buffer);
				unsigned int numOfPara = *(unsigned int*)(buffer + 4);
				if (nf)
				{
					int paraDefCount =  nf->numOfPara ;
					if (paraDefCount>-1 && paraDefCount != numOfPara)
					{
						runTimeError("call c function error: wrong parameters number");
					}
					else
					{
						int sizeOfStack = stack.size();
						Value rt = (*nf->funcPointer)(stack,numOfPara);
						//pop all the prarameters
						for (unsigned int i = 0; i < numOfPara;++i)
						{
							pop(stack.size());
						}
						stack.push_back(rt);
						if(rt.type == STRING )
							rt.g_string_in++;
						else if (rt.type == VECTOR)
							rt.g_vector_in++;
					}

					

				}
				}

				
				break;
			case C_FUNC:break;//function definition
			case C_PARA:break;//indicate the parameters
			case C_LOCALVAR:break;//var number in func
			case C_RETURN_NO_VALUE:
				{
				buffer = getDataFromByteCode(4);
				int numOfPara = *(unsigned int*)buffer;
				 
				IP = stack[FP - 1].returnaddress; 
				unsigned int tempFP = stack[FP].parentfp;
				if(FP < stack.size())
				{
					unsigned int sizeOfStack = stack.size();
					for (unsigned int i = FP - 1 - numOfPara; i < sizeOfStack; i++)
					{
						pop(stack.size());
					}
					//stack.erase(stack.begin()+FP - 1,stack.end());
				}
				//stack.erase(stack.end()-numOfPara,stack.end());
				stack.push_back(Value());
				FP = tempFP;
				}
				break;
			case C_RETURN:
				{
					buffer = getDataFromByteCode(4);
					int numOfPara = *(unsigned int*)buffer;
					 
					
					Value tempRtValue = stack.back(); 
					//pop(stack.size());
					stack.pop_back();

					IP = stack[FP - 1].returnaddress; 
					unsigned int tempFP = stack[FP].parentfp;

					if(FP < stack.size())
					{
						for (unsigned int i = stack.size(); i > FP -1 - numOfPara ; --i)
						{
							pop(i);
						}
						//stack.erase(stack.begin()+FP - 1,stack.end());
					}
					//stack.erase(stack.end()-numOfPara,stack.end());
					 
					stack.push_back(tempRtValue);
					//cout<<"return val: "<<dec<<tempRtValue.ival<<"\n";
					FP = tempFP;
				}
				
				break;//function return
			case C_IFNE:
				{
					unsigned int sizeOfStack = stack.size();
					if(stack[sizeOfStack - 1].ival == 0)
					{
						buffer = getDataFromByteCode(4);
						IP = *(unsigned int*)buffer; 
					}
					else
					{
						IP += 4; //skip the ifne address
					}
					pop(sizeOfStack);
				}
				break;// if not equal
			case C_THEN:break;//no command
			case C_JUMP: //jump command
				buffer = getDataFromByteCode(4);
				IP = *(unsigned int*)buffer;
				break;
			case C_POP:
				pop(stack.size());
				break;
			case C_EXIT:
				stack.clear();
				return;//exit!

			default:
				runTimeError("unknown command");
		}
	}

	NativeFunc* VM::getNativeFuncById(unsigned int id)
	{
		for (auto it = nativeFuncMap.begin();it!=nativeFuncMap.end();++it)
		{
			if( it->second->id == id)
				return it->second;
		}
		return NULL;
	}


	byte* VM::getDataFromByteCode(unsigned int lenOfData)
	{
		byte* dataBuffer = &m_code[IP];
		IP += lenOfData;
		return dataBuffer;
	}

	void VM::push(Value &v)
	{
		v.inRef();
		stack.push_back(v);
	}

	void VM::pop(unsigned int sizeOfStack)
	{
		stack[sizeOfStack-1].deRef();
		stack.pop_back();
	}


	Value& VM::getLocal() 
	{
		byte* buff = getDataFromByteCode(4);
		int offsetOfVariable = *(int*)buff;
		return stack[offsetOfVariable+FP];
	}

	void VM::runTimeError(char* errorStr)
	{
		cout<<"Run time error: code address: "<<dec<<IP-1<<". "<<errorStr<<"\n";
		Log::writeLog(string("Run time error: code address: ") + to_string(IP - 1) + string(errorStr) + "\n");
		return;
	}

}