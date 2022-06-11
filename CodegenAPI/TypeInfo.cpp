/*
file:   TypeInfo.cpp

author:	Aleksey Yakovlev
data:	July 10, 2022

Description of data types for a task on the topic of code generation.
*/

#include "pch.h"
#include "TypeInfo.h"

using namespace CodegenAPI;
using namespace std;



string FunctionParam::view() const 
{
    stringstream ss; ss<<m_keyname;
    for(int i=0;i<m_refpow;++i)ss<<"*"; return ss.str();
}

string FunctionParam::view(const string &deepname) const
{
    stringstream ss;
    if(m_const)ss<<"const ";
    if(deepname.size()>0 && deepname.size()<m_keyname.size() && 
        m_keyname.substr(0,deepname.size())==deepname)
        ss<<m_keyname.substr(deepname.size(),m_keyname.size()-deepname.size());
    else ss<<m_keyname;
    for(int i=0;i<m_refpow;++i)ss<<"*"; return ss.str();
}



ModuleName::ModuleName(string name) : m_name(name), m_system()
{
    if(m_name.size()>=2)
        if(m_name[0]=='<' && m_name[m_name.size()-1]=='>')
            { m_system = true; m_name = m_name.substr(1,m_name.size()-2); }
        else if(m_name[0]=='\"' && m_name[m_name.size()-1]=='\"')
            m_name = m_name.substr(1,m_name.size()-2);
}
string ModuleName::view() const
    { return m_system ? "<"+m_name+">" : "\""+m_name+"\""; }



stringstream& TypeInfo::translateTemplateParams(stringstream &ss) const
{
    if(isTemplate())
    {
        ss<<"template <";
        for(size_t i=0;i+1<m_template_params.size();++i)
            ss<<"typename "<<m_template_params[i]<<", ";
        ss<<"typename "<<m_template_params[m_template_params.size()-1]<<"> ";
    }
    return ss;
};



void ClassTypeInfo::translate(stringstream &ss,
        const string &key, const string &name) const
{ 
    translateTemplateParams(ss)<<"class "<<name<<";"<<endl; 
}



void StructTypeInfo::translate(stringstream &ss,
        const string &key, const string &name) const
{ 
    translateTemplateParams(ss)<<"struct "<<name<<";"<<endl; 
}



void FunctionTypeInfo::check(const LongName &keyname,
    const map<LongName,shared_ptr<TypeInfo>> &scheme) const
{ 
    for(const FunctionParam &param : m_params)
        if(auto param_it = scheme.find(param.getKeyName());
                param_it!=scheme.end() && param_it->second->isTemplate())
            throw runtime_error("template arguments are not supported");
}

vector<LongName> FunctionTypeInfo::dependencies() const
{
    vector<LongName> deps;
    for_each(begin(m_params),end(m_params),
        [&deps](const FunctionParam &param) {deps.push_back(param.getKeyName());});
    return deps;
}

void FunctionTypeInfo::translate(stringstream &ss, 
    const string &key, const string &name) const
{
    ss<<"using "<<name<<" = "<<m_params[0].view(key)<<" (*)(";
    for(size_t i=1; i+1<m_params.size(); ++i)ss<<m_params[i].view(key)<<", ";
    if(m_params.size()>1)ss<<m_params[m_params.size()-1].view(key);
    ss<<")"<<";"<<endl;
}