/*
file:   CodegenAPI.cpp

author:	Aleksey Yakovlev
data:	July 10, 2022

The main interface module source for a task on the topic of code generation.
*/

#include "pch.h"
#include "CodegenAPI.h"

#include <string_view>
#include <queue>

using namespace CodegenAPI;
using namespace std;

void IntermediateCode::includeModule(const ModuleName &mname)
{ 
    if(mname.isPerfect())
        m_code.push_back({Command::IncludeModule,mname.view()}); 
}

void IntermediateCode::openNamespace(const string &name)
    { m_code.push_back({Command::OpenNamespace,name}); }

void IntermediateCode::declareForward(const string &name)
    { m_code.push_back({Command::ForwardDeclaration,name}); }

void IntermediateCode::closeNamespace()
{ 
    if(!m_code.empty() && m_code[m_code.size()-1].first==Command::OpenNamespace)
        m_code.pop_back();
    else m_code.push_back({Command::CloseNamespace,string()}); 
}

string IntermediateCode::translate(const map<LongName,shared_ptr<TypeInfo>> &scheme) const
{
    stringstream ss; bool force_endl = false;
    auto skip = [&ss,&force_endl](size_t indent) -> stringstream&
    { 
        if(force_endl){ ss<<endl; force_endl=false; }
        for(size_t i=0; i<indent; ++i)ss<<"\t"; return ss; 
    };

    vector<string> deepname; deepname.push_back(string());
    for(size_t indent=0, i=0; i<m_code.size(); ++i)switch(m_code[i].first)
    {
    case Command::IncludeModule:
        ss<<"#include "<<m_code[i].second<<endl; force_endl=true;
        break;
    case Command::OpenNamespace:
        skip(indent)<<"namespace "<<m_code[i].second<<endl;
        skip(indent)<<"{"<<endl; 
        deepname.push_back(deepname[indent++]+m_code[i].second+"::"); 
        break;
    case Command::ForwardDeclaration:
        {
            auto forward_it = scheme.find(deepname[indent]+m_code[i].second);
            if(forward_it==scheme.end())throw NotFoundKeyError(deepname[indent]+m_code[i].second);
            const auto & [keyname, info] = *forward_it;
            info->translate(skip(indent),deepname[indent],m_code[i].second);
        } break;
    case Command::CloseNamespace:
        deepname.pop_back(); if(deepname.empty())throw NamespaceNestingError();
        skip(--indent)<<"}"<<endl; 
        break;
    default:
        throw bad_exception();
    }
    return ss.str();
}

bool IntermediateCode::verify(
    const map<LongName,shared_ptr<TypeInfo>> &scheme,
    const vector<LongName> &include_names, const vector<LongName> &declare_names,
    const set<LongName> &applied) const
{
    set<LongName> completed(applied.begin(),applied.end());
    set<LongName> forced_declare(begin(declare_names),end(declare_names));
    set<ModuleName> modules;

    vector<string> deepname; deepname.push_back(string());
    for(size_t indent=0, i=0; i<m_code.size(); ++i)switch(m_code[i].first)
    {
    case Command::IncludeModule:
        modules.insert(m_code[i].second);
        break;
    case Command::OpenNamespace:
        deepname.push_back(deepname[indent++]+m_code[i].second+"::");
        break;
    case Command::ForwardDeclaration:
        {
            LongName keyname = deepname[indent]+m_code[i].second;
            auto forward_it = scheme.find(keyname);
            if(forward_it==scheme.end())throw NotFoundKeyError(keyname);
            shared_ptr<TypeInfo> info = forward_it->second;

            //check for double forward
            if(!completed.insert(keyname).second)throw DuplicateForwardError(keyname);

            //check for module include
            if(info->isExternal() && modules.find(info->getModule())==modules.end())
                throw NotFoundModuleError(info->getModule().view());

            //check dependencies for forward and/or include
            for(const LongName &depname : info->dependencies())
                if(completed.find(depname)==completed.end())
                    if(forced_declare.find(depname)!=forced_declare.end())throw NotFoundForwardError(depname);
                    else
                    {
                        auto depend_it = scheme.find(depname);
                        if(depend_it==scheme.end())throw NotFoundKeyError(depname);
                        shared_ptr<TypeInfo> depinfo = depend_it->second;
                    
                        if(depinfo->isExternal())
                        {
                            if(modules.find(depinfo->getModule())==modules.end())
                                throw NotFoundModuleError(depinfo->getModule().view());
                        }
                        else throw NotFoundForwardError(depname);
                    }
        } break;
    case Command::CloseNamespace:
        deepname.pop_back(); if(deepname.empty())throw NamespaceNestingError();
        --indent;
        break;
    default:
        throw bad_exception();
    }

    //check for include forced names
    for(const LongName &keyname : include_names)
        if(auto forward_it = scheme.find(keyname); forward_it==scheme.end())
            throw NotFoundKeyError(keyname);
        else
        {
            shared_ptr<TypeInfo> info = forward_it->second;
            if(modules.find(info->getModule())==modules.end()) 
                    throw NotFoundModuleError(info->getModule().view());
        }

    //check for forward forced names
    for(const LongName &keyname : declare_names)
        if(completed.find(keyname)==completed.end())throw NotFoundForwardError(keyname);

    return true;
}



struct Codegen::NamespaceModelNode
{
    set<string> forwards;
    map<string,NamespaceModelNode> attachments;
    size_t incompleted_count;
    NamespaceModelNode() : incompleted_count() {}

    bool placeForward(string_view id);
    void renderNode(IntermediateCode &code, 
        set<LongName> &completed, const set<LongName> &forced_declare,
        const map<LongName,shared_ptr<TypeInfo>> &scheme, const string &key = string());
};

bool Codegen::NamespaceModelNode::placeForward(string_view id)
{
    if(id.empty())throw SyntaxError();
    string_view::const_iterator vbeg {begin(id)};
    string_view::const_iterator vend {end(id)};
    while(vbeg!=vend)if(*vbeg==':')
    {
        string space {begin(id),vbeg};
        ++vbeg; if(vbeg==vend || *vbeg!=':')throw SyntaxError();
        ++vbeg;
        NamespaceModelNode &child = attachments.try_emplace(space,NamespaceModelNode()).first->second;
        if(child.placeForward({id.data()+(vbeg-begin(id)),size_t(vend-vbeg)}))
            { ++incompleted_count; return true; }
        else return false;
    } else ++vbeg;
    if(forwards.insert(static_cast<string>(id)).second){ ++incompleted_count; return true; }
    else return false;
}

void Codegen::NamespaceModelNode::renderNode(IntermediateCode &code,
    set<LongName> &completed, const set<LongName> &forced_declare,
    const map<LongName,shared_ptr<TypeInfo>> &scheme, const string &key)
{
    size_t incompleted_prev; do //loop the greedy algorithm
    {
        incompleted_prev = incompleted_count;

        for(const string& name : forwards)
        if(LongName keyname = key+name; completed.find(keyname)==completed.end())
        {
            auto forward_it = scheme.find(keyname);
            if(forward_it==scheme.end())throw NotFoundKeyError(keyname);
            shared_ptr<TypeInfo> info = forward_it->second;
               
            auto check_dependencies = [&completed,&forced_declare,&scheme]
                (const LongName &keyname, const vector<LongName> &deps) -> bool
            {
                for(const LongName &depname : deps)
                    if(completed.find(depname)==completed.end() && keyname!=depname)
                        if(forced_declare.find(depname)!=forced_declare.end())return false;
                        else
                        {
                            auto depend_it = scheme.find(depname);
                            if(depend_it==scheme.end())throw NotFoundKeyError(depname);
                            shared_ptr<TypeInfo> depinfo = depend_it->second;
                            if(!depinfo->isExternal())return false;
                        }
                return true;
            };
            if(check_dependencies(keyname,info->dependencies()))
                { code.declareForward(name); completed.insert(keyname); --incompleted_count; }
        }

        for(auto& [childname,childnode] : attachments)if(childnode.incompleted_count>0)
        {
            code.openNamespace(childname);
            size_t childnode_incompleted = childnode.incompleted_count;
            childnode.renderNode(code,completed,forced_declare,scheme,key+childname+"::");
            incompleted_count -= childnode_incompleted - childnode.incompleted_count;
            code.closeNamespace();
        }
    } while(incompleted_count<incompleted_prev && incompleted_count>0);
}

IntermediateCode Codegen::code(
	const vector<LongName> &include_names,
	const vector<LongName> &declare_names) const
{ 
    IntermediateCode icode;
    set<ModuleName> modules;

    //make namespace tree
    NamespaceModelNode root; queue<LongName> depends;
    for(const LongName &name : declare_names)depends.push(name);
    while(!depends.empty())
    {
        auto forward_it = m_scheme.find(depends.front());
        if(forward_it==m_scheme.end())throw NotFoundKeyError(depends.front());
        shared_ptr<TypeInfo> info = forward_it->second; 
        
        if(root.placeForward(depends.front()))
        {
            modules.insert(info->getModule());
            info->check(depends.front(),m_scheme);
            for(const LongName &depname : info->dependencies())
                if(m_some_fundamental.find(depname)==m_some_fundamental.end())
                {
                    auto depend_it = m_scheme.find(depname);
                    if(depend_it==m_scheme.end())throw NotFoundKeyError(depname);
                    shared_ptr<TypeInfo> depinfo = depend_it->second;
                    if(depinfo->isExternal())modules.insert(depinfo->getModule());
                    else depends.push(depname);
                }
        }
        depends.pop();
    }

    //render modules list
    for(const LongName &keyname : include_names)
        if(auto forward_it = m_scheme.find(keyname); forward_it==m_scheme.end())
            throw NotFoundKeyError(keyname);
        else modules.insert(forward_it->second->getModule());
    for(const ModuleName &mname : modules)
        if(mname.isPerfect())icode.includeModule(mname);

    //render namespaces and forwards
    set<LongName> completed {m_some_fundamental};
    set<LongName> forced_declare(begin(declare_names),end(declare_names));
    root.renderNode(icode,completed,forced_declare,m_scheme);
    if(root.incompleted_count>0)throw LoopForwardError();

    return icode;
}