#ifndef PARSER_H
#define PARSER_H

#include <stack>
#include <ctype.h>
#include <stdint.h>
#include <exception>

#include "Utf8.hpp"
#include "Value.hpp"

namespace JSON {
    // JSON escape characters that may appear
    // after a reverse solidus
    const char ESC_REVERSE_SOLIUDS  = 92;
    const char ESC_QUOTATION_MARK   = 34;
    const char ESC_SOLIDUS          = 47;
    const char ESC_BACKSPACE        = 98;
    const char ESC_FORMFEED         = 102;
    const char ESC_NEWLINE          = 110;
    const char ESC_CARRET           = 114;
    const char ESC_HORIZONTAL_TAB   = 116;
    const char ESC_UNICODE          = 117;
    
    // Use 32 bit characters for unicode strings
    typedef std::basic_string<int32_t> wideString;

    // Represents a JSON parser
    class Parser {
        public:
            Parser() : parseIndex(0), lineNumber(1) { }

            void parse(Value& object, const std::string& source);
            void parse(Value& object, const char * source);

        private:
            void reset() {
                lineNumber = 1;
                parseIndex = 0;
                while(!objectStack.empty()) {
                    objectStack.pop();
                }
            }
        
            // Increment the parse index until a non-whitespace character
            // is encountered.
            void clearWhitespace() {
                while (hasNext() && isspace(source.at(parseIndex))) {
                    if (peek() == 10 || peek() == 12 || peek() == 13) {
                        lineNumber++;
                    }
                    parseIndex++;
                }
            }

            // Increase the parse index but ignore the current character
            void consume() {
                parseIndex++;
            }

            // End of Stream reached?
            bool hasNext() const {
                return parseIndex < source.length();
            }
                
            // Return a reference to the current character in the
            // stream and increase the index.
            char& next() {
                if (!hasNext()) {
                    // End of Stream already reached?
                    throw std::runtime_error("Unexpected end of stream");
                }
                // Increase the line number. This is used to print
                // the position when a parse error occurs.
                if (peek() == '\n') {
                    lineNumber++;
                }

                return source.at(parseIndex++);
            }

            // Return a reference to the current character in the
            // stream without increasing the index.
            char& peek() {
                if (!hasNext()) {
                    // End of Stream already reached?
                    throw std::runtime_error("Unexpected end of stream");
                }
                return source.at(parseIndex);
            }

            // Return a reference to the top item on the stack.
            // The top item usually holds a reference to the 
            // current array / object at the parse position.
            Value& top() const {
                return *objectStack.top();
            }

            // Store a parsed value and return a reference
            // to it
            Value& store(const Value& val) {
                if (top().is(JSON_ARRAY)) {
                    // Current parse position is inside an array:
                    // Append the new item to the end of the array
                    top().push_back(val);
                    return top().asMutable<Array>().back();
                } else if (top().is(JSON_OBJECT)) {
                    // Current parse position is inside an object:
                    // Read the current property and store the new item
                    // under this property inside the current object.
                    top()[currentProperty.str().c_str()] = val;
                    return top()[currentProperty.str().c_str()];
                } else {
                    // Parse position is not inside an array and not
                    // inside an object. Put the new item on the top
                    // of the stack.
                    top() = val;
                    return top();
                }
            }

            // Tests if a character is allowed for numeric expressions.
            bool validNumericChar(char code) const {
                return (code >= 48 && code <= 57)
                    || (code == 46)     // '.'
                    || (code == 45)     // '-'
                    || (code == 43)     // '+'
                    || (code == 69)     // 'E'                    
                    || (code == 101);   // 'e'
            }

            // Tests if a character is allowed to introduce a numeric
            // expression.
            bool validNumericStartingChar(char code) const {
                return (code >= 48 && code <= 57)
                    || (code == 45);    // '-'
            }

            // Valid digit within an \u2360 unicode escape?
            bool validHexDigit(char code) const {
                return (code >= 48 && code <= 57)
                    || (code >= 65 && code <= 70)
                    || (code >= 97 && code <= 102);
            }

            void parseObject();
            void parseProperty();
            void parseValue();
            void parseString();
            void parseArray();
            void parseBoolean();
            void parseNumber();
            void parseNull();
            void escapeChar();
            void readUTF8Escape();
                        
            unsigned int parseIndex;
            unsigned int lineNumber;

            std::string source;
            std::ostringstream currentProperty;
            std::ostringstream currentString;
            
            // Since std::stack<Value&> is not possible use
            // a pointer here
            std::stack<Value *> objectStack;
    };

    /**
     * { ... }
     */
    inline void Parser::parseObject() {
        clearWhitespace();
        if (next() == '{') {
            clearWhitespace();
            
            // Push a new object on the stack
            objectStack.push(&store(Object {}));
            
            if (peek() != '}') {
                // Parse object properties
                while (hasNext()) {
                    parseProperty();
                    clearWhitespace();

                    if (hasNext() && peek() != ',') {
                        break;
                    } else {
                        // Another property to parse. Get rid
                        // of the ',' and go on...
                        consume(); // ','
                    }
                }
            }
            
            // End of properties
            clearWhitespace();
            
            if (peek() == '}') {
                consume(); // '}'
                // Done with parsing the object.
                // Pop it from the stack, since this is no longer
                // the reference object for eventual coming items.
                objectStack.pop();
                return;
            } else {
                // Objects must end with ...}
                throw std::runtime_error("Objects must end with }");
            }

        } else {
            // Objects have to start with {...
            throw std::runtime_error("Objects must start with {");
        }
    }

    /**
     * ...:
     */
    inline void Parser::parseProperty() {
        // Reset currentProperty buffer
        currentProperty.str(""); 
        clearWhitespace();
        
        // Properties must start with '"'
        if (peek() == ESC_QUOTATION_MARK) {
            consume(); // '"'
            while (peek() != ESC_QUOTATION_MARK) {
                currentProperty << next();
            }
            
            // Properties must end with '"'
            if (next() != ESC_QUOTATION_MARK) {
                throw std::runtime_error("Properties must end with\"");
            } else {
                clearWhitespace();
                // ...": ...
                // Properties must be in the form of
                // "key": value
                if (peek() != ':') {
                    throw std::runtime_error("Expected colon");
                } else {
                    consume(); // ':'
                    // Parse the value
                    // :... 
                    parseValue();
                }
            }
        } else {
            throw std::runtime_error("Parse error in property");
        }
    }

    /**
     * :...
     */
    inline void Parser::parseValue() {
        clearWhitespace();
        
        // Decide the type of the value on the stream
        switch(peek()) {
            case '{':
                parseObject();
                return;
            case '"':
                parseString();
                return;
            case '[':
                parseArray();
                return;
            case 'n':
              parseNull();
              return;
            case 't':
            case 'f':
                parseBoolean();
                return;
            default:
                // If none of the former types matched always try to
                // parse a number.
                if (validNumericStartingChar(peek())) {
                    parseNumber();
                    return;
                } else {
                    throw std::runtime_error("Parser error in value");
                }
        }
    }

    /**
     * null
     */
    inline void Parser::parseNull() {
      currentString.str("");
      
      // Read the next four characters and test if they
      // are equal to 'null'
      for (int i=0;i<4;i++) {
          currentString << next();
      }
      
      if (currentString.str().compare("null") == 0) {
          store(Value());
      } else {
        throw std::runtime_error("Parse error in null");
      }
    }

    /**
     * numbers
     */
    inline void Parser::parseNumber() {
        currentString.str("");
        while (hasNext() && validNumericChar(peek())) {
            currentString << next();
        }

        store(fromString<double>(currentString.str()));
    }

    /**
     * true | false 
     */
    inline void Parser::parseBoolean() {
        currentString.str("");
        // consume lowercase letters
        while (hasNext() && peek() >= 97 && peek() <= 122) {
            currentString << next();
        }

        bool result;
        if (currentString.str().compare("true") == 0) {
            result = true;
        } else if (currentString.str().compare("false") == 0) {
            result = false;
        } else {
            throw std::runtime_error("Parse error in boolean");
        } 
        
        store(result);
    }

    /**
     * "..."
     */
    inline void Parser::parseString() {
        // Reset string buffer
        currentString.str("");
        consume(); // '"'
        while (peek() != ESC_QUOTATION_MARK) {
            // String contains an escaped character?
            if (peek() == ESC_REVERSE_SOLIUDS) {
                escapeChar();
            } else {
                currentString << next();
            }
        }

        consume(); // '"'
        store(currentString.str());
    }

    /**
     * \...
     */
    inline void Parser::escapeChar() {
      consume(); // REVERSE_SOLIDUS
      // Decide which escape character follows
      switch(peek()) {
          case ESC_BACKSPACE:
              consume();
              currentString << (char) 8;
              break;
          case ESC_HORIZONTAL_TAB:
              consume();
              currentString << (char) 9;
              break;
          case ESC_NEWLINE:
              consume();
              currentString << (char) 10;
              break;
          case ESC_FORMFEED:
              consume();
              currentString << (char) 12;
              break;
          case ESC_CARRET:
              consume();
              currentString << (char) 13;
              break;
          case ESC_QUOTATION_MARK:
          case ESC_REVERSE_SOLIUDS:
          case ESC_SOLIDUS:
              currentString << next();
              break;
          case ESC_UNICODE:
              // a \u occured
              // read the following sequence and insert it's
              // unicode representation into the string.
              readUTF8Escape();            
              break;
          default:
              throw std::runtime_error("Parse error in escape sequence");
      }
    }

    inline void Parser::readUTF8Escape() {
        consume(); // u
        int codePoint = -1;
        std::string tmp(4,32);
        std::stringstream ss;
        std::string result;
        wideString str32;
        
        for (unsigned int index=0;index<tmp.length();index++) {
            if (!hasNext() || !validHexDigit(peek())) {
              throw std::runtime_error("Parse error in UTF8");
            }
            
            tmp[index] = next();
        }
        
        ss << std::hex << tmp;
        ss >> codePoint;
        str32.push_back(codePoint);
        utf8::utf32to8(str32.begin(), str32.end(), std::back_inserter(result));
        currentString << result;
    }

    /**
     * [...]
     */
    inline void Parser::parseArray() {
        consume(); // '['
                
        objectStack.push(&store(Array {}));
        clearWhitespace();

        // Empty array?
        if (peek() == ']') {
            consume(); // ']'
            objectStack.pop();
            return;
        }
                
        while(hasNext()) {                        
            parseValue();
            clearWhitespace();
            
            if (peek() != ',') {
                break;
            } else {
                consume(); // ','
            }
        }
        
        if (peek() != ']') {
            throw std::runtime_error("Parse error in array");
        } else {
            consume(); // ']'
        }
                
        objectStack.pop();
    }

    /**
     * Entry points
     */
    inline void Parser::parse(Value& value, const std::string &source) {
        reset();
        if (source.length() > 0) {
            value = null;
            this->source = source;
            objectStack.push(&value);
            parseValue();
            clearWhitespace();
            if (parseIndex < source.length()) {
                throw std::runtime_error("Parse error");
            }
        }
    }

    inline void Parser::parse(Value& value, const char *source) {
        std::string _source(source);
        parse(value, _source);
    }
}

#endif // PARSER_H
