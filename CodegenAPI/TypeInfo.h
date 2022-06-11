/*
file:   TypeInfo.h

author:	Aleksey Yakovlev
data:	July 10, 2022

Description of data types for a task on the topic of code generation.

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

#ifndef TYPE_INFO_H
#define TYPE_INFO_H

#include <vector>
#include <map>
#include <set>
#include <sstream>

#include "ErrorClasses.h"

namespace CodegenAPI
{
    using LongName = std::string;

    using TemplateParam = std::string;

    class ModuleName
    {
    protected:
        std::string m_name;
        bool m_system;
    public:
        ModuleName(std::string name);
        ModuleName(const char name[]) : ModuleName(static_cast<std::string>(name)) { }

        bool isPerfect() const { return !m_name.empty(); }
        bool isSytem() const { return m_system; }
        std::string view() const;

        bool operator<(const ModuleName &module) const 
            { return m_system && !module.m_system || m_system==module.m_system && m_name.compare(module.m_name)<0; }
    };

    class TypeInfo
    {
    protected:
        ModuleName m_module;
        std::vector<TemplateParam> m_template_params;
        std::stringstream& translateTemplateParams(std::stringstream &ss) const;
        TypeInfo(const char module[], std::initializer_list<TemplateParam> template_params)
            : m_module(module), m_template_params(template_params) {}
    public:
        const ModuleName& getModule() const { return m_module; }

        virtual void check(const LongName &keyname,
            const std::map<LongName,std::shared_ptr<TypeInfo>> &scheme) const { }

        virtual std::vector<LongName> dependencies() const =0;

        virtual void translate(std::stringstream &ss,
            const std::string &key, const std::string &name) const =0;

        bool isTemplate() const { return !m_template_params.empty(); }
        bool isExternal() const { return m_module.isPerfect(); }
    };

    class ClassTypeInfo : public TypeInfo
    {
    public:
        ClassTypeInfo(const char module[],
            std::initializer_list<TemplateParam> template_params = {}) 
            : TypeInfo(module,template_params) {}

        std::vector<LongName> dependencies() const override 
            { return std::vector<LongName>(); }

        void translate(std::stringstream &ss,
            const std::string &key, const std::string &name) const override;

        static std::shared_ptr<TypeInfo> make(const char module[],
            std::initializer_list<TemplateParam> template_params = {})
            { return std::static_pointer_cast<TypeInfo>(std::make_shared<ClassTypeInfo>(module,template_params)); }
    };

    class StructTypeInfo : public TypeInfo
    {
    public:
        StructTypeInfo(const char module[],
            std::initializer_list<TemplateParam> template_params = {}) 
            : TypeInfo(module,template_params) {}

        std::vector<LongName> dependencies() const override 
            { return std::vector<LongName>(); }

        void translate(std::stringstream &ss,
            const std::string &key, const std::string &name) const override;

        static std::shared_ptr<TypeInfo> make(const char module[],
            std::initializer_list<TemplateParam> template_params = {})
            { return std::static_pointer_cast<TypeInfo>(std::make_shared<StructTypeInfo>(module,template_params)); }
    };

    class FunctionParam
    {
    protected:
        LongName m_keyname;
        bool m_const;
        int m_refpow;
    public:
        FunctionParam(const LongName &keyname, bool cnst = false, int refpow = 0)
            : m_keyname(keyname), m_const(cnst), m_refpow(refpow) {}

        const LongName& getKeyName() const { return m_keyname; }
        std::string view() const;
        std::string view(const std::string &deepname) const;
    };

    class FunctionTypeInfo : public TypeInfo
    {
    protected:
        std::vector<FunctionParam> m_params;
    public:
        FunctionTypeInfo(const char module[],
            std::initializer_list<FunctionParam> params = {}) 
            : TypeInfo(module,{}), m_params(params)
            { if(m_params.empty())m_params.push_back({"void"}); }

        void check(const LongName &keyname,
            const std::map<LongName,std::shared_ptr<TypeInfo>> &scheme) const override;

        std::vector<LongName> dependencies() const override;

        void translate(std::stringstream &ss, 
            const std::string &key, const std::string &name) const override;

        static std::shared_ptr<TypeInfo> make(const char module[],
            std::initializer_list<FunctionParam> params = {})
            { return std::static_pointer_cast<TypeInfo>(std::make_shared<FunctionTypeInfo>(module,params)); }
    };
}
#endif

