#pragma once

#include "value.hpp"
#include "list_utils.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

inline double asNumber(const ValuePtr& v) {
    if (v->type != ValueType::Number) throw LispError("Expected a number, got: " + toString(v));
    return v->number;
}

inline EnvPtr makeGlobalEnv() {
    EnvPtr env = std::make_shared<Environment>();

    // ---- Arithmetic ----
    env->define("+", makeBuiltin([](const std::vector<ValuePtr>& a) {
        double sum = 0; for (auto& x : a) sum += asNumber(x); return makeNumber(sum);
    }));
    env->define("-", makeBuiltin([](const std::vector<ValuePtr>& a) {
        if (a.empty()) throw LispError("- needs at least 1 argument");
        if (a.size() == 1) return makeNumber(-asNumber(a[0]));
        double r = asNumber(a[0]);
        for (size_t i = 1; i < a.size(); i++) r -= asNumber(a[i]);
        return makeNumber(r);
    }));
    env->define("*", makeBuiltin([](const std::vector<ValuePtr>& a) {
        double r = 1; for (auto& x : a) r *= asNumber(x); return makeNumber(r);
    }));
    env->define("/", makeBuiltin([](const std::vector<ValuePtr>& a) {
        if (a.empty()) throw LispError("/ needs at least 1 argument");
        double r = asNumber(a[0]);
        for (size_t i = 1; i < a.size(); i++) {
            double d = asNumber(a[i]);
            if (d == 0) throw LispError("Division by zero");
            r /= d;
        }
        return makeNumber(a.size() == 1 ? 1.0 / r : r);
    }));
    env->define("modulo", makeBuiltin([](const std::vector<ValuePtr>& a) {
        return makeNumber(std::fmod(asNumber(a[0]), asNumber(a[1])));
    }));

    // ---- Comparisons ----
    auto cmp = [](const std::vector<ValuePtr>& a, std::function<bool(double,double)> f) {
        for (size_t i = 0; i + 1 < a.size(); i++)
            if (!f(asNumber(a[i]), asNumber(a[i + 1]))) return makeBool(false);
        return makeBool(true);
    };
    env->define("=",  makeBuiltin([cmp](auto& a){ return cmp(a, [](double x,double y){return x==y;}); }));
    env->define("<",  makeBuiltin([cmp](auto& a){ return cmp(a, [](double x,double y){return x<y;}); }));
    env->define(">",  makeBuiltin([cmp](auto& a){ return cmp(a, [](double x,double y){return x>y;}); }));
    env->define("<=", makeBuiltin([cmp](auto& a){ return cmp(a, [](double x,double y){return x<=y;}); }));
    env->define(">=", makeBuiltin([cmp](auto& a){ return cmp(a, [](double x,double y){return x>=y;}); }));

    // ---- Pairs / lists ----
    env->define("cons", makeBuiltin([](const std::vector<ValuePtr>& a) { return makePair(a[0], a[1]); }));
    env->define("car", makeBuiltin([](const std::vector<ValuePtr>& a) {
        if (a[0]->type != ValueType::Pair) throw LispError("car: not a pair");
        return a[0]->car;
    }));
    env->define("cdr", makeBuiltin([](const std::vector<ValuePtr>& a) {
        if (a[0]->type != ValueType::Pair) throw LispError("cdr: not a pair");
        return a[0]->cdr;
    }));
    env->define("list", makeBuiltin([](const std::vector<ValuePtr>& a) { return vectorToList(a); }));
    env->define("null?", makeBuiltin([](const std::vector<ValuePtr>& a) {
        return makeBool(a[0]->type == ValueType::Nil);
    }));
    env->define("pair?", makeBuiltin([](const std::vector<ValuePtr>& a) {
        return makeBool(a[0]->type == ValueType::Pair);
    }));
    env->define("length", makeBuiltin([](const std::vector<ValuePtr>& a) {
        return makeNumber(static_cast<double>(listToVector(a[0]).size()));
    }));
    env->define("append", makeBuiltin([](const std::vector<ValuePtr>& a) {
        std::vector<ValuePtr> result;
        for (auto& lst : a) for (auto& v : listToVector(lst)) result.push_back(v);
        return vectorToList(result);
    }));
    env->define("reverse", makeBuiltin([](const std::vector<ValuePtr>& a) {
        auto v = listToVector(a[0]);
        std::reverse(v.begin(), v.end());
        return vectorToList(v);
    }));

    // ---- Predicates ----
    env->define("not", makeBuiltin([](const std::vector<ValuePtr>& a) { return makeBool(!isTruthy(a[0])); }));
    env->define("eq?", makeBuiltin([](const std::vector<ValuePtr>& a) {
        if (a[0]->type != a[1]->type) return makeBool(false);
        switch (a[0]->type) {
            case ValueType::Number: return makeBool(a[0]->number == a[1]->number);
            case ValueType::Symbol: return makeBool(a[0]->str == a[1]->str);
            case ValueType::Bool: return makeBool(a[0]->boolean == a[1]->boolean);
            case ValueType::Nil: return makeBool(true);
            default: return makeBool(a[0].get() == a[1].get());
        }
    }));

    // ---- I/O ----
    env->define("display", makeBuiltin([](const std::vector<ValuePtr>& a) {
        std::cout << toDisplayString(a[0]); return makeNil();
    }));
    env->define("newline", makeBuiltin([](const std::vector<ValuePtr>&) {
        std::cout << "\n"; return makeNil();
    }));

    return env;
}
