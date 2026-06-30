#pragma once
#include "value.hpp"
#include <vector>

// Converts a Lisp list (chain of Pairs ending in Nil) into a std::vector for easy iteration.
inline std::vector<ValuePtr> listToVector(ValuePtr v) {
    std::vector<ValuePtr> out;
    while (v->type == ValueType::Pair) {
        out.push_back(v->car);
        v = v->cdr;
    }
    return out;
}

// Converts a std::vector back into a proper Lisp list.
inline ValuePtr vectorToList(const std::vector<ValuePtr>& items, size_t from = 0) {
    ValuePtr result = makeNil();
    for (size_t i = items.size(); i > from; i--) {
        result = makePair(items[i - 1], result);
    }
    return result;
}

inline std::string toString(const ValuePtr& v);

// Like toString, but renders strings without surrounding quotes -- used by `display`,
// where the user wants the raw text, not a re-readable representation.
inline std::string toDisplayString(const ValuePtr& v) {
    if (v->type == ValueType::String) return v->str;
    return toString(v);
}

inline std::string toString(const ValuePtr& v) {
    switch (v->type) {
        case ValueType::Number: {
            double n = v->number;
            if (n == static_cast<long long>(n)) return std::to_string(static_cast<long long>(n));
            return std::to_string(n);
        }
        case ValueType::Symbol: return v->str;
        case ValueType::String: return "\"" + v->str + "\"";
        case ValueType::Bool: return v->boolean ? "#t" : "#f";
        case ValueType::Nil: return "()";
        case ValueType::Lambda: return "#<lambda>";
        case ValueType::Builtin: return "#<builtin>";
        case ValueType::Pair: {
            std::string out = "(";
            ValuePtr cur = v;
            bool first = true;
            while (cur->type == ValueType::Pair) {
                if (!first) out += " ";
                out += toString(cur->car);
                first = false;
                cur = cur->cdr;
            }
            if (cur->type != ValueType::Nil) { // improper list
                out += " . " + toString(cur);
            }
            out += ")";
            return out;
        }
    }
    return "";
}
