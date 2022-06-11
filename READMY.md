#The code generator API.
The code generator API provides the ability to automatically output preliminary
announcements and a list of libraries to include in C++ based on the meta
information received. This API has a flexible extensible architecture.
---
The API provides the **CodegenAPI::Codegen** class for loading meta information with
the possibility of its subsequent multiple translation into the intermediate code
**CodegenAPI::IntermediateCode** class with various parameters.
The API provides the **CodegenAPI::ClassTypeInfo**, **CodegenAPI::StructTypeInfo**,
and **CodegenAPI::FunctionTypeInfo** classes to describe the original meta information
and can be extended with new constructs by inheriting from the **CodegenAPI::TypeInfo** class.
When creating intermediate code, the correct and effective grouping and sorting of
language constructs by the namespaces of the generated code is performed.
A special greedy type algorithm is used for this. The required included libraries
will be automatically connected. The contents of the intermediate code 
**CodegenAPI::IntermediateCode** can then be checked using the **verify** class method
and converted to a text form using the **translate** class method.
---
The greedy algorithm used for translation is not optimal in terms
of code generation quality and performance. This can be improved.
Separate copies of **std::string** are used to store meta-information keys,
which requires a lot of memory. It is possible to create a hash table for storing
parameters and store only pointers to its elements in the meta information.