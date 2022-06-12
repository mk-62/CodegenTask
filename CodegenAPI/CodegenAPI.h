/*
file:   CodegenAPI.h

author:	Aleksey Yakovlev
data:	July 10, 2022

The main interface module header for a task on the topic of code generation.

The code generator API provides the ability to automatically output preliminary
announcements and a list of libraries to include in C++ based on the meta
information received. This API has a flexible extensible architecture.

The API provides the 'CodegenAPI::Codegen' class for loading meta information with
the possibility of its subsequent multiple translation into the intermediate code
'CodegenAPI::IntermediateCode' class with various parameters.
The API provides the 'CodegenAPI::ClassTypeInfo', 'CodegenAPI::StructTypeInfo',
and 'CodegenAPI::FunctionTypeInfo' classes to describe the original meta information
and can be extended with new constructs by inheriting from the 'CodegenAPI::TypeInfo' class.
When creating intermediate code, the correct and effective grouping and sorting of
language constructs by the namespaces of the generated code is performed.
A special greedy type algorithm is used for this. The required included libraries
will be automatically connected. The contents of the intermediate code 
'CodegenAPI::IntermediateCode' can then be checked using the 'verify' class method
and converted to a text form using the 'translate' class method.

The greedy algorithm used for translation is not optimal in terms
of code generation quality and performance. This can be improved.
Separate copies of 'std::string' are used to store meta-information keys,
which requires a lot of memory. It is possible to create a hash table for storing
parameters and store only pointers to its elements in the meta information.
*/

#ifndef CODEGEN_API_H
#define CODEGEN_API_H

#include <memory>
#include <string>
#include <map>
#include <algorithm>

#include "TypeInfo.h"

namespace CodegenAPI
{
    class IntermediateCode
    {
    protected:
        enum class Command { IncludeModule, OpenNamespace, ForwardDeclaration, CloseNamespace };
        std::vector<std::pair<Command,std::string>> m_code;
    public:
        IntermediateCode() = default;

        void includeModule(const ModuleName &mname);
        void openNamespace(const std::string &name);
        void declareForward(const std::string &name);
        void closeNamespace();

        std::string translate(const std::map<LongName,std::shared_ptr<TypeInfo>> &scheme) const;
        bool verify(
            const std::map<LongName,std::shared_ptr<TypeInfo>> &scheme,
            const std::vector<LongName> &include_names, const std::vector<LongName> &declare_names,
            const std::set<LongName> &applied) const;
    };

	class Codegen
	{
	protected:
        struct NamespaceModelNode;

        std::map<LongName,std::shared_ptr<TypeInfo>> m_scheme;
        std::set<LongName> m_some_fundamental = 
            {"void", "char", "int", "long", "long long", "unsigned", "size_t", "float", "double"};

        template <class Iter> Codegen(Iter first, Iter last) 
        {
            std::for_each(first,last,[this](const std::pair<LongName,std::shared_ptr<TypeInfo>>& i)
            {
                const auto & [keyname, info] = i;
                if(!m_scheme.try_emplace(keyname,info).second)throw DuplicateKeyError(keyname);
            });
        }
	public:
		Codegen(const std::map<LongName,std::shared_ptr<TypeInfo>> &scheme)
            : m_scheme(scheme) { }
		Codegen(std::map<LongName,std::shared_ptr<TypeInfo>> &&scheme)
            : m_scheme(std::move(scheme)) { }
        Codegen(std::initializer_list<std::pair<LongName,std::shared_ptr<TypeInfo>>> scheme)
            : Codegen(std::begin(scheme),std::end(scheme)) { }
		Codegen(const std::vector<std::pair<LongName,std::shared_ptr<TypeInfo>>> &scheme)
            : Codegen(std::begin(scheme),std::end(scheme)) { }

		IntermediateCode code(
			const std::vector<LongName> &include_names,
			const std::vector<LongName> &declare_names) const;
		std::string source(
			const std::vector<LongName> &include_names,
			const std::vector<LongName> &declare_names) const
            { return code(include_names,declare_names).translate(m_scheme); }
		bool test(
			const std::vector<LongName> &include_names,
			const std::vector<LongName> &declare_names) const
            { return code(include_names,declare_names)
                    .verify(m_scheme,include_names,declare_names,m_some_fundamental); }

        const std::map<LongName,std::shared_ptr<TypeInfo>>& getSheme() const { return m_scheme; }
	};
}

#endif