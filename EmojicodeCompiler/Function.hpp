//
//  Function.h
//  Emojicode
//
//  Created by Theo Weidmann on 04/01/16.
//  Copyright © 2016 Theo Weidmann. All rights reserved.
//

#ifndef Function_hpp
#define Function_hpp

#include <queue>
#include <map>
#include <functional>
#include "Token.hpp"
#include "TokenStream.hpp"
#include "Type.hpp"
#include "Callable.hpp"
#include "CallableParserAndGeneratorMode.hpp"
#include "CallableWriter.hpp"
#include "Class.hpp"

enum class AccessLevel {
    Public, Private, Protected
};

class Closure: public Callable {
public:
    Closure(SourcePosition p) : Callable(p) {};
};

/** Functions are callables that belong to a class as either a method, a class method or an initializer. */
class Function: public Callable {
    friend void Class::finalize();
    friend Protocol;
    friend void generateCode(Writer &writer);
public:
    static bool foundStart;
    static Function *start;
    static std::queue<Function *> compilationQueue;
    /** Returns a VTI for a function. */
    static int nextFunctionVti() { return nextVti_++; }
    /** Returns the number of funciton VTIs assigned. This should be equal to the number of compiled functions. */
    static int functionCount() { return nextVti_; }
    
    static void checkReturnPromise(Type returnThis, Type returnSuper, EmojicodeChar name, SourcePosition position,
                                   const char *on, Type contextType);
    static void checkArgumentCount(size_t thisCount, size_t superCount, EmojicodeChar name, SourcePosition position,
                                   const char *on, Type contextType);
    static void checkArgument(Type thisArgument, Type superArgument, int index, SourcePosition position,
                              const char *on, Type contextType);
    
    Function(EmojicodeChar name, AccessLevel level, bool final, Type owningType, Package *package, SourcePosition p,
             bool overriding, EmojicodeString documentationToken, bool deprecated, CallableParserAndGeneratorMode mode)
        : Callable(p),
          name(name),
          final_(final),
          overriding_(overriding),
          deprecated_(deprecated),
          access_(level),
          owningType_(owningType),
          package_(package),
          documentation_(documentationToken),
          compilationMode_(mode) {}
    
    /** The function name. A Unicode code point for an emoji */
    EmojicodeChar name;
    
    /** Whether the method is implemented natively and Run-Time Native Linking must occur. */
    bool native = false;
    /** Whether the method was marked as final and can’t be overriden. */
    bool final() const { return final_; }
    /** Whether the method is intended to override a super method. */
    bool overriding() const { return overriding_; }
    /** Whether the method is deprecated. */
    bool deprecated() const { return deprecated_; }
    /** Returns the access level to this method. */
    AccessLevel accessLevel() const { return access_; }
    
    /** Type to which this function belongs. 
        This can be Nothingness if the function doesn’t belong to any type (e.g. 🏁). */
    Type owningType() const { return owningType_; }
    
    const EmojicodeString& documentation() const { return documentation_; }
    
    /** The types for the generic arguments. */
    std::vector<Type> genericArgumentConstraints;
    /** Generic type arguments as variables */
    std::map<EmojicodeString, Type> genericArgumentVariables;
    
    /** The namespace in which the function was defined.
        This does not necessarily match the package of @c owningType. */
    Package* package() const { return package_; }
    
    /** Issues a warning on at the given token if the function is deprecated. */
    void deprecatedWarning(const Token &callToken);
    
    /**
     * Check whether this function is breaking promises of @c superFunction.
     */
    void checkPromises(Function *superFunction, const char *on, Type contextType);
    
    bool checkOverride(Function *superFunction);
    
    /** Returns the VTI for this function or fetches one by calling the VTI Assigner.
        @warning This method must only be called if the function will be needed at run-time and 
        should be assigned a VTI. */
    int vtiForUse();
    int getVti() const;
    void setVtiAssigner(std::function<int()>);
    
    CallableParserAndGeneratorMode compilationMode() const { return compilationMode_; }
    
    /** Whether this function should be treated as type method at Run-Time Native Linking time. */
    bool typeMethod() const;
    
    bool assigned() const;
    
    int maxVariableCount() const { return maxVariableCount_; }
    void setMaxVariableCount(int c) { maxVariableCount_ = c; }
    
    CallableWriter writer_;
private:
    /** Sets the VTI to @c vti and enters this functions into the list of functions to be compiled into the binary. */
    void setVti(int vti);
    
    static int nextVti_;
    int vti_ = -1;
    bool final_;
    bool overriding_;
    bool deprecated_;
    AccessLevel access_;
    Type owningType_;
    Package *package_;
    EmojicodeString documentation_;
    std::function<int()> vtiAssigner_ = nullptr;
    CallableParserAndGeneratorMode compilationMode_;
    int maxVariableCount_ = -1;
};
 
class Initializer: public Function {
public:
    Initializer(EmojicodeChar name, AccessLevel level, bool final, Type owningType, Package *package, SourcePosition p,
                bool overriding, EmojicodeString documentationToken, bool deprecated, bool r, bool crn,
                CallableParserAndGeneratorMode mode)
        : Function(name, level, final, owningType, package, p, overriding, documentationToken, deprecated, mode),
          required(r),
          canReturnNothingness(crn) {
              returnType = owningType;
    }
    
    bool required;
    bool canReturnNothingness;
    
    virtual Type type() const override;
};

#endif /* Function_hpp */
