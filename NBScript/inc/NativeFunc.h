#pragma once
#include "Value.h"

namespace NBE
{
	int Value::g_string_in ;
	int Value::g_string_de ;
	int Value::g_vector_in ;
	int Value::g_vector_de ;

	void printValue(Value& a)
	{
		switch(a.type)
		{
		case STRING:
			cout<<a.sval->str;
			break;
		case INT:
			cout<<a.ival;
			break;
		case FLOAT:
			cout<<a.fval;
			break;
		case BOOL:
			cout<< (a.ival == 1 ? "true" : "false");
			break;
		case VECTOR:
			cout<<"[";
			for (unsigned int j = 0;j<a.vval->vec->size();++j)
			{ 
				cout<<" ";
				printValue( (*a.vval->vec)[j]);
			}
			cout<<" ]";
			break;
		case -1:
			cout<<"null";
			break;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//native funcs
	Value printFunc(vector<Value>& stack,int paraNum)
	{
		unsigned int sizeOfStack = stack.size();
		for (unsigned int i = sizeOfStack - paraNum; i<sizeOfStack;++i)
		{
			printValue( stack[i] );
		}
		cout<<"\n";
		return Value();//return a useless value
	}

	Value square(vector<Value>& stack, int paraNum)
	{
		Value& para = stack[stack.size()-1];
		Value rt = Value();
		switch(para.type)
		{
		case INT:
			rt.type = INT;
			rt.ival = para.ival * para.ival;
			break;
		case FLOAT:
			rt.type = FLOAT;
			rt.fval = para.fval * para.fval;
			break;
		default:
			cout<<"parameter needs to be INT or FLOAT\n";
		}
		return rt;
	}

	Value IntToStr(vector<Value>& stack, int paraNum)
	{
		Value& para = stack[stack.size()-1];
		Value rt = Value(STRING,new Value::ValStr(""));
		rt.sval->str = new char[16];
		sprintf_s(rt.sval->str,16,"%d", para.ival);
	//	itoa( para.ival,rt.sval->str,10);
		return rt;
	}


	//////////////////////////////////////////////////////////////////////////
}