#pragma once

#include <string>
#include <vector>
#include <cctype>

enum class TokenType { LParen, RParen, Quote, Atom, End };

struct Token {
    TokenType type;
    std::string text;
};

// Converts source text into tokens: '(' ')' '\'' and atoms (numbers, symbols, strings).
class Lexer {
public:
    explicit Lexer(const std::string& source) : src_(source), pos_(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (true) {
            skipWhitespaceAndComments();
            if (pos_ >= src_.size()) break;

            char c = src_[pos_];
            if (c == '(') {
                tokens.push_back({TokenType::LParen, "("});
                pos_++;
            } else if (c == ')') {
                tokens.push_back({TokenType::RParen, ")"});
                pos_++;
            } else if (c == '\'') {
                tokens.push_back({TokenType::Quote, "'"});
                pos_++;
            } else if (c == '"') {
                tokens.push_back(readString());
            } else {
                tokens.push_back(readAtom());
            }
        }
        tokens.push_back({TokenType::End, ""});
        return tokens;
    }

private:
    const std::string& src_;
    size_t pos_;

    void skipWhitespaceAndComments() {
        while (pos_ < src_.size()) {
            char c = src_[pos_];
            if (std::isspace(static_cast<unsigned char>(c))) {
                pos_++;
            } else if (c == ';') { // line comment
                while (pos_ < src_.size() && src_[pos_] != '\n') pos_++;
            } else {
                break;
            }
        }
    }

    Token readString() {
        std::string out;
        pos_++; // skip opening quote
        while (pos_ < src_.size() && src_[pos_] != '"') {
            out += src_[pos_++];
        }
        if (pos_ < src_.size()) pos_++; // skip closing quote
        return {TokenType::Atom, "\"" + out}; // prefix marks it as a string literal for the parser
    }

    Token readAtom() {
        size_t start = pos_;
        while (pos_ < src_.size() &&
               !std::isspace(static_cast<unsigned char>(src_[pos_])) &&
               src_[pos_] != '(' && src_[pos_] != ')' && src_[pos_] != '\'') {
            pos_++;
        }
        return {TokenType::Atom, src_.substr(start, pos_ - start)};
    }
};
