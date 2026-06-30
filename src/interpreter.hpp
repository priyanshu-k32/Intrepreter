#pragma once

#include "value.hpp"
#include "list_utils.hpp"
#include <vector>

class Interpreter {
public:
    // eval is written as a loop (instead of naive recursion) so that tail calls --
    // e.g. the recursive call in `(define (loop n) (if (= n 0) 'done (loop (- n 1))))`
    // -- reuse the same C++ stack frame instead of growing it. This is what lets the
    // interpreter run "infinite" tail-recursive loops without a C++ stack overflow.
    ValuePtr eval(ValuePtr expr, EnvPtr env) {
        while (true) {
            switch (expr->type) {
                case ValueType::Number:
                case ValueType::String:
                case ValueType::Bool:
                case ValueType::Nil:
                    return expr; // self-evaluating

                case ValueType::Symbol:
                    return env->get(expr->str);

                case ValueType::Pair:
                    break; // handled below

                default:
                    return expr;
            }

            ValuePtr op = expr->car;
            ValuePtr args = expr->cdr;

            if (op->type == ValueType::Symbol) {
                const std::string& sym = op->str;

                if (sym == "quote") {
                    return args->car;
                }
                if (sym == "if") {
                    auto v = listToVector(args);
                    ValuePtr test = eval(v[0], env);
                    if (isTruthy(test)) { expr = v[1]; continue; }
                    else if (v.size() > 2) { expr = v[2]; continue; }
                    else return makeNil();
                }
                if (sym == "define") {
                    return evalDefine(args, env);
                }
                if (sym == "set!") {
                    auto v = listToVector(args);
                    env->set(v[0]->str, eval(v[1], env));
                    return makeNil();
                }
                if (sym == "lambda") {
                    return evalLambda(args, env);
                }
                if (sym == "begin") {
                    auto v = listToVector(args);
                    if (v.empty()) return makeNil();
                    for (size_t i = 0; i + 1 < v.size(); i++) eval(v[i], env);
                    expr = v.back(); continue; // tail call on last expr
                }
                if (sym == "let") {
                    EnvPtr newEnv = evalLetBindings(args, env);
                    auto v = listToVector(args);
                    for (size_t i = 1; i + 1 < v.size(); i++) eval(v[i], newEnv);
                    expr = v.back(); env = newEnv; continue;
                }
                if (sym == "cond") {
                    ValuePtr next = evalCond(args, env);
                    if (!next) return makeNil();
                    expr = next; continue;
                }
                if (sym == "and") {
                    auto v = listToVector(args);
                    ValuePtr result = makeBool(true);
                    for (size_t i = 0; i < v.size(); i++) {
                        if (i + 1 == v.size()) { expr = v[i]; goto tailcontinue; }
                        result = eval(v[i], env);
                        if (!isTruthy(result)) return result;
                    }
                    return result;
                    tailcontinue: continue;
                }
                if (sym == "or") {
                    auto v = listToVector(args);
                    for (size_t i = 0; i < v.size(); i++) {
                        if (i + 1 == v.size()) { expr = v[i]; goto tailcontinue2; }
                        ValuePtr result = eval(v[i], env);
                        if (isTruthy(result)) return result;
                    }
                    return makeBool(false);
                    tailcontinue2: continue;
                }
            }

            // ---- Function application ----
            ValuePtr fn = eval(op, env);
            std::vector<ValuePtr> argv;
            for (ValuePtr a : listToVector(args)) argv.push_back(eval(a, env));

            if (fn->type == ValueType::Builtin) {
                return fn->builtin(argv);
            }
            if (fn->type == ValueType::Lambda) {
                EnvPtr callEnv = std::make_shared<Environment>(fn->lambda->closure);
                const auto& params = fn->lambda->params;
                if (params.size() != argv.size()) {
                    throw LispError("Arity mismatch: expected " + std::to_string(params.size()) +
                                     " args, got " + std::to_string(argv.size()));
                }
                for (size_t i = 0; i < params.size(); i++) callEnv->define(params[i], argv[i]);

                const auto& body = fn->lambda->body;
                for (size_t i = 0; i + 1 < body.size(); i++) eval(body[i], callEnv);
                expr = body.back(); env = callEnv; continue; // TAIL CALL: loop instead of recursing
            }
            throw LispError("Attempt to call a non-function: " + toString(fn));
        }
    }

private:
    ValuePtr evalDefine(ValuePtr args, EnvPtr env) {
        ValuePtr target = args->car;
        if (target->type == ValueType::Pair) {
            // (define (name args...) body...)  ==  (define name (lambda (args...) body...))
            std::string name = target->car->str;
            ValuePtr lambdaArgs = makePair(target->cdr, args->cdr);
            env->define(name, evalLambda(lambdaArgs, env));
        } else {
            // (define name value)
            ValuePtr value = eval(args->cdr->car, env);
            env->define(target->str, value);
        }
        return makeNil();
    }

    ValuePtr evalLambda(ValuePtr args, EnvPtr env) {
        auto lambda = std::make_shared<Lambda>();
        for (auto p : listToVector(args->car)) lambda->params.push_back(p->str);
        for (auto b : listToVector(args->cdr)) lambda->body.push_back(b);
        lambda->closure = env;
        return makeLambda(lambda);
    }

    EnvPtr evalLetBindings(ValuePtr args, EnvPtr env) {
        EnvPtr newEnv = std::make_shared<Environment>(env);
        for (auto binding : listToVector(args->car)) {
            auto pair = listToVector(binding);
            newEnv->define(pair[0]->str, eval(pair[1], env));
        }
        return newEnv;
    }

    // Returns the expression to evaluate in tail position for the matching cond clause, or nullptr.
    ValuePtr evalCond(ValuePtr args, EnvPtr env) {
        for (auto clause : listToVector(args)) {
            auto v = listToVector(clause);
            if (v[0]->type == ValueType::Symbol && v[0]->str == "else") {
                return v.size() > 1 ? v.back() : makeBool(true);
            }
            ValuePtr test = eval(v[0], env);
            if (isTruthy(test)) {
                return v.size() > 1 ? v.back() : test;
            }
        }
        return nullptr;
    }
};
