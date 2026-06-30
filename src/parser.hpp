#pragma once

#include "lexer.hpp"
#include "value.hpp"
#include <vector>
#include <stdexcept>

// Builds a list of top-level S-expressions (as Value trees) from a token stream.
class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), pos_(0) {}

    // Parses every top-level form in the input (a program is a sequence of expressions).
    std::vector<ValuePtr> parseProgram() {
        std::vector<ValuePtr> exprs;
        while (peek().type != TokenType::End) {
            exprs.push_back(parseExpr());
        }
        return exprs;
    }

private:
    std::vector<Token> tokens_;
    size_t pos_;

    const Token& peek() { return tokens_[pos_]; }
    Token advance() { return tokens_[pos_++]; }

    ValuePtr parseExpr() {
        Token tok = peek();

        if (tok.type == TokenType::LParen) {
            advance();
            return parseList();
        }
        if (tok.type == TokenType::Quote) {
            advance();
            // 'x  ==  (quote x)
            return makePair(makeSymbol("quote"), makePair(parseExpr(), makeNil()));
        }
        if (tok.type == TokenType::Atom) {
            advance();
            return parseAtom(tok.text);
        }
        throw std::runtime_error("Unexpected end of input while parsing expression");
    }

    ValuePtr parseList() {
        if (peek().type == TokenType::RParen) {
            advance();
            return makeNil();
        }
        ValuePtr head = parseExpr();
        ValuePtr tail = parseList();
        return makePair(head, tail);
    }

    ValuePtr parseAtom(const std::string& text) {
        // String literal (lexer prefixed it with a leading '"')
        if (!text.empty() && text[0] == '"') {
            return makeString(text.substr(1));
        }
        // #t / #f booleans
        if (text == "#t") return makeBool(true);
        if (text == "#f") return makeBool(false);

        // Try to parse as a number
        if (isNumber(text)) {
            return makeNumber(std::stod(text));
        }
        // Otherwise it's a symbol (identifier, operator, keyword)
        return makeSymbol(text);
    }

    static bool isNumber(const std::string& s) {
        if (s.empty()) return false;
        size_t i = 0;
        if (s[i] == '+' || s[i] == '-') i++;
        if (i >= s.size()) return false;
        bool sawDigit = false;
        for (; i < s.size(); i++) {
            if (std::isdigit(static_cast<unsigned char>(s[i]))) sawDigit = true;
            else if (s[i] == '.') continue;
            else return false;
        }
        return sawDigit;
    }
};
