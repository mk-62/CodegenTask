/*
file:   ErrorClasses.h

author:	Aleksey Yakovlev
data:	July 10, 2022

Description of error classes for a test on the topic of code generation.
*/

#ifndef ERROR_CLASSES_H
#define ERROR_CLASSES_H

#include <stdexcept>

namespace CodegenAPI
{
    class SyntaxError : public std::runtime_error
    {
    public:
        SyntaxError() 
            : runtime_error("syntax error") { }
    };

    class NotFoundKeyError : public std::runtime_error
    {
    public:
       NotFoundKeyError(const std::string &key)
           : runtime_error("not found key: "+key) { }
    };

    class DuplicateKeyError : public std::runtime_error
    {
    public:
       DuplicateKeyError(const std::string &key)
            : runtime_error("duplicate key: "+key) { }
    };

    class NotFoundModuleError : public std::runtime_error
    {
    public:
       NotFoundModuleError(const std::string &name)
           : runtime_error("not found module: "+name) { }
    };

    class NamespaceNestingError : public std::runtime_error
    {
    public:
       NamespaceNestingError()  
           : runtime_error("namespace nesting error") { }
    };

    class DuplicateForwardError : public std::runtime_error
    {
    public:
       DuplicateForwardError(const std::string &key)
            : runtime_error("duplicate forward: "+key) { }
    };

    class NotFoundForwardError : public std::runtime_error
    {
    public:
       NotFoundForwardError(const std::string &key)  
           : runtime_error("not found forward error: "+key) { }
    };

    class LoopForwardError : public std::runtime_error
    {
    public:
       LoopForwardError()  
           : runtime_error("loop forward error") { }
    };
}
#endif

