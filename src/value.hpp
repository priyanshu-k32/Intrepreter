#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <variant>
#include <stdexcept>

// Forward declarations
struct Value;
class Environment;
using ValuePtr = std::shared_ptr<Value>;
using EnvPtr = std::shared_ptr<Environment>;

// A user-defined function (lambda): parameter names + body expressions + closure environment
struct Lambda {
    std::vector<std::string> params;
    std::vector<ValuePtr> body;   // a lambda body can be multiple expressions (implicit "begin")
    EnvPtr closure;
};

// A builtin (native C++) function
using BuiltinFn = std::function<ValuePtr(const std::vector<ValuePtr>&)>;

enum class ValueType { Number, Symbol, String, Bool, Nil, Pair, Lambda, Builtin };

// The core dynamic value type. Every Lisp value (number, symbol, list, function...) is one of these.
struct Value : std::enable_shared_from_this<Value> {
    ValueType type;

    double number = 0.0;
    std::string str;            // used for Symbol and String
    bool boolean = false;

    // Pair: cons cell -- the building block of lists
    ValuePtr car;
    ValuePtr cdr;

    std::shared_ptr<Lambda> lambda;
    BuiltinFn builtin;

    Value(ValueType t) : type(t) {}
};

// ---- Convenience constructors ----
inline ValuePtr makeNumber(double n) {
    auto v = std::make_shared<Value>(ValueType::Number);
    v->number = n;
    return v;
}
inline ValuePtr makeSymbol(const std::string& s) {
    auto v = std::make_shared<Value>(ValueType::Symbol);
    v->str = s;
    return v;
}
inline ValuePtr makeString(const std::string& s) {
    auto v = std::make_shared<Value>(ValueType::String);
    v->str = s;
    return v;
}
inline ValuePtr makeBool(bool b) {
    auto v = std::make_shared<Value>(ValueType::Bool);
    v->boolean = b;
    return v;
}
inline ValuePtr makeNil() {
    static ValuePtr nilVal = std::make_shared<Value>(ValueType::Nil);
    return nilVal;
}
inline ValuePtr makePair(ValuePtr car, ValuePtr cdr) {
    auto v = std::make_shared<Value>(ValueType::Pair);
    v->car = car;
    v->cdr = cdr;
    return v;
}
inline ValuePtr makeLambda(std::shared_ptr<Lambda> l) {
    auto v = std::make_shared<Value>(ValueType::Lambda);
    v->lambda = l;
    return v;
}
inline ValuePtr makeBuiltin(BuiltinFn fn) {
    auto v = std::make_shared<Value>(ValueType::Builtin);
    v->builtin = fn;
    return v;
}

inline bool isTruthy(const ValuePtr& v) {
    // Everything is truthy except #f and nil (Scheme convention: only #f is falsy;
    // we also treat nil as falsy for convenience in `if` with empty lists)
    if (v->type == ValueType::Bool) return v->boolean;
    return true;
}

// Custom exception used for runtime errors (unbound variable, type errors, etc.)
struct LispError : std::runtime_error {
    explicit LispError(const std::string& msg) : std::runtime_error(msg) {}
};

// ---- Environment: a symbol table with a parent pointer for lexical scoping ----
class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(EnvPtr parent = nullptr) : parent_(parent) {}

    void define(const std::string& name, ValuePtr value) {
        vars_[name] = value;
    }

    // Walk up the chain of enclosing scopes until the symbol is found
    ValuePtr get(const std::string& name) {
        Environment* env = this;
        while (env) {
            auto it = env->vars_.find(name);
            if (it != env->vars_.end()) return it->second;
            env = env->parent_.get();
        }
        throw LispError("Unbound variable: " + name);
    }

    void set(const std::string& name, ValuePtr value) {
        Environment* env = this;
        while (env) {
            auto it = env->vars_.find(name);
            if (it != env->vars_.end()) {
                it->second = value;
                return;
            }
            env = env->parent_.get();
        }
        throw LispError("Cannot set! unbound variable: " + name);
    }

private:
    std::unordered_map<std::string, ValuePtr> vars_;
    EnvPtr parent_;
};
