/*
file:   CodegenTests.cpp

author:	Aleksey Yakovlev
data:	July 10, 2022

Unit test and check module for a task on the topic of code generation.
*/

#include "pch.h"
#include "CppUnitTest.h"

#include "../CodegenAPI/CodegenAPI.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace CodegenAPI;
using namespace std;

namespace CodegenAPITests
{
	TEST_CLASS(HeaderGeneratorTests)
	{
	public:

        static void Report(bool testresult, const string &emsg)
        {
            int ch = MultiByteToWideChar(CP_ACP,0,emsg.c_str(),-1,nullptr,0);
            wchar_t *mem = new wchar_t[ch];
            try
            {
                if(mem)MultiByteToWideChar(CP_ACP,0,emsg.c_str(),-1,mem,ch);
                Assert::IsTrue(testresult,mem); if(mem)delete[] mem;
            } catch(...) { if(mem)delete[] mem; throw; }
        }

        TEST_METHOD(moduleNameClass)
        {
            bool testresult; string emsg;
            try
            {
                string m_string {"<string>"};
                Assert::AreEqual(ModuleName(m_string).view(), m_string);
                string m_user1 {"user1.h"};
                Assert::AreEqual(ModuleName(m_user1).view(), "\""+m_user1+"\"");
                string m_user2 {"\"user2.h\""};
                Assert::AreEqual(ModuleName(m_user2).view(), m_user2);
                testresult=true;
            }
            catch(const exception &ex) { testresult=false; emsg=ex.what(); }
            catch(...) { testresult=false; emsg="Unknown error"; }

            Report(testresult,emsg);
        }
		
		TEST_METHOD(sample1)
		{
            bool testresult; string emsg;
            try
            {
                Codegen hg {
                    {"std::string",ClassTypeInfo::make("<string>")},
                    {"lib::func1",FunctionTypeInfo::make("funcs.h",
                        {{"void",false,1},{"std::string"},{"lib::func1"},{"lib::inn::st1",true,1}})},
                    {"lib::func2",FunctionTypeInfo::make("funcs.h",
                        {{"void",false,1},{"std::string"},{"lib::func2"},{"lib::inn::st2",true,1}})},
                    {"lib::inn::st3",StructTypeInfo::make("lib.h")},
                    {"lib::inn::st1",StructTypeInfo::make("lib.h")},
                    {"lib::inn::st2",StructTypeInfo::make("lib.h")},
                };

                testresult = hg.test({"std::string"}, { "lib::func1", "lib::func2", "lib::inn::st3", "lib::inn::st1"});
            }
            catch(const exception &ex) { testresult=false; emsg=ex.what(); }
            catch(...) { testresult=false; emsg="Unknown error"; }

            Report(testresult,emsg);
		}

		TEST_METHOD(sample2)
		{
            bool testresult; string emsg;
            try
            {
                Codegen hg 
                {
                   {"func_a",FunctionTypeInfo::make("",{{"func_b"}})},
                   {"func_b",FunctionTypeInfo::make("",{{"func_c"}})},
                   {"func_c",FunctionTypeInfo::make("",{{"func_a"}})},
                };
                testresult = hg.test({}, {"func_a"}), false;
            }
            catch(const LoopForwardError &ex) { testresult=true; emsg=ex.what(); }
            catch(const exception &ex) { testresult=false; emsg=ex.what(); }
            catch(...) { testresult=false; emsg="Unknown error"; }

            Report(testresult,emsg);
		}

	};

}