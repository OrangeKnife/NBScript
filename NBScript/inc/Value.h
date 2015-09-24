#pragma once
#include <vector>
namespace NBE
{
	enum ValueType
	{
		INT = 0,
		FLOAT,
		STRING,
		VECTOR,
		PARENTFP,
		RTADDRESS,
		BOOL,

		TOTALTYPE
	};

	class RefCountable
	{
	public:
		RefCountable():refCount(1){}
		int refCount;
	};



	struct Value
	{
		static int g_string_in ;
		static int g_string_de ;
		static int g_vector_in ;
		static int g_vector_de ;
		struct ValStr:public RefCountable
		{
			ValStr(char* c):str(c)//,refCount(1)
			{}
			~ValStr()
			{
				delete str;
			}
			char* str;
			//int refCount;
		};

		struct ValVec:public RefCountable
		{
			ValVec(std::vector<Value>* v):vec(v)//,refCount(1)
			{}
			~ValVec()
			{
				delete vec;
			}
			std::vector<Value>* vec;
			//int refCount;
		};
		Value():type(-1)
		{}

		Value(ValueType tp, int val):type(tp)
		{ival = val;}
		Value(ValueType tp, float val):type(tp)
		{fval = val;}
		Value(ValueType tp, ValStr* val):type(tp)
		{sval = val;}
		Value(ValueType tp, ValVec* val):type(tp)
		{vval = val;}
		Value(ValueType tp, unsigned int val):type(tp)
		{tp == PARENTFP? parentfp = val : returnaddress = val;}


		int type;
		union
		{
			int ival;//int
			float fval;//float
			ValStr* sval;//string
			ValVec* vval;//vector
			unsigned int parentfp;
			unsigned int returnaddress;
		};

		int inRef(){
			int count = 0;
			switch (type)
			{
			case STRING:
				count = ++sval->refCount;
				g_string_in++;
				break;
			case VECTOR:
				count = ++vval->refCount;
				g_vector_in++;
				break;
			}
			return count;}

		int deRef(){
			int count = 0;
			switch (type)
			{
			case STRING:
				count = --sval->refCount;
				g_string_de++;
				break;
			case VECTOR:
				count = --vval->refCount;
				g_vector_de++;
				break;
			}
			if (count <= 0)//delete it
			{
				switch (type)
				{
				case STRING:
					delete sval;
					break;
				case VECTOR:
					for(unsigned int i = 0; i < vval->vec->size(); ++i)
					{
						(*vval->vec)[i].deRef();
					}
					delete vval;
					break;
				case -1:
					break;//just a empty value

				}
			}
			return count;}

	};


}