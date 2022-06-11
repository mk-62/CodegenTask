/*
file:   CodegenRun.cpp

author:	Aleksey Yakovlev
data:	July 10, 2022

Main program sample for a task on the topic of code generation.
*/

#include "pch.h"
#include "../CodegenAPI/CodegenAPI.h"
#include <iostream>

using namespace CodegenAPI;
using namespace std;

int main()
{
    int retcode;

    try
    {
        Codegen hg {
            {"std::string",ClassTypeInfo::make("<string>")},
            {"my_library::awesome",ClassTypeInfo::make("")},
            {"my_library::func1",FunctionTypeInfo::make("",
                {{"void",false,1},{"std::string"},{"my_library::func1"},{"my_library::awesome",true},{"my_library::inn::inn::struct1",true,1}})},
            {"my_library::func2",FunctionTypeInfo::make("",
                {{"void",false,1},{"std::string"},{"my_library::func2"},{"my_library::awesome",true},{"my_library::inn::inn::struct2",true,1}})},
            {"astra::loss",FunctionTypeInfo::make("",{{"void",true,1},{"my_library::awesome"}/*,{"my_library::struct1"}*/})},
            {"my_library::quick",StructTypeInfo::make("my_library.h",{"T1","T2","T3"})},
            {"my_library::inn::inn::struct3",StructTypeInfo::make("my_library.h")},
            {"my_library::inn::inn::struct1",StructTypeInfo::make("my_library.h")},
            {"my_library::inn::inn::struct2",StructTypeInfo::make("my_library.h")},
            {"astra::bar",ClassTypeInfo::make("")},
        };

        cout << hg.source(
            {"std::string"},
            {
                "my_library::quick", "astra::loss", "my_library::func1", "my_library::func2",
                "my_library::inn::inn::struct3", "my_library::inn::inn::struct1",
                "astra::bar"
            });

        retcode = 0;
    }
    catch(const exception &ex) { cerr << ex.what(); retcode=-1; }
    catch(...) { cerr << "Unknown error"; retcode=-1; }

    return retcode;
}
