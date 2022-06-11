/*
file:   CodegenAPI.h

author:	Aleksey Yakovlev
data:	July 10, 2022

The main interface module header for a task on the topic of code generation.
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

        const std::map<LongName,std::shared_ptr<TypeInfo>>& scheme() const { return m_scheme; }
	};
}

#endif