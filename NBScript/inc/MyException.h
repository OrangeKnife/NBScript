#pragma once
namespace NBE
{
	class MyException
	{
	public:
		MyException(int l,int t):lineNum(l),token(t){}
		int lineNum;
		int token;
	};
}