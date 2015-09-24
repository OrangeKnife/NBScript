#pragma once 
#include <vector>
namespace NBE
{

	class VM
	{
	public:
		VM(std::vector<byte> code, NativeFuncMap& nfmap):m_code(code),IP(0),FP(0),
			nativeFuncMap(nfmap)
		{}
		void execute();
		void runTimeError(char* errorStr);
		inline byte* getDataFromByteCode(unsigned int lenOfData);

		inline void push(Value &v);
		inline void pop(unsigned int sizeOfStack);
		inline Value& getLocal();
		
		NativeFunc* getNativeFuncById(unsigned int id);
	private:
		std::vector<byte> m_code;
		std::vector<Value> stack;
		unsigned int IP;
		unsigned int FP;

		NativeFuncMap& nativeFuncMap;
	};



}