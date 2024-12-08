#include <iostream>
#include <vector>
#include <map>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <algorithm>

using namespace std;

enum TokenType
{
    T_INT,
    T_FLOAT,
    T_DOUBLE,
    T_STRING,
    T_BOOL,
    T_CHAR,
    T_ID,
    T_NUM,
    T_IF,
    T_ELSE,
    T_RETURN,
    T_ASSIGN,
    T_PLUS,
    T_MINUS,
    T_MUL,
    T_DIV,
    T_LPAREN,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_SEMICOLON,
    T_SENTENCE,
    T_GT,
    T_LT,
    T_EQ,
    T_NEQ,
    T_AND,
    T_OR,
    T_WHILE,
    T_FOR,
    T_EOF,
};

struct Token
{
    TokenType type;
    string value;
};

struct Symbol
{
    TokenType type;
    int scopeLevel;
    bool initialized;
};

class SymbolTable
{
private:
    map<string, Symbol> symbols;

public:
    void insert(string &name, TokenType type, int scopeLevel)
    {
        if (symbols.find(name) == symbols.end())
        {
            symbols[name] = {type, scopeLevel, false};
        }
        else
        {
            cout << "Error: Redefinition of variable '" << name << "'." << endl;
            exit(1);
        }
    }

    bool lookup(const string &name)
    {
        return symbols.find(name) != symbols.end();
    }

    Symbol &get(const string &name)
    {
        if (lookup(name))
        {
            return symbols[name];
        }
        else
        {
            cout << "Error: Variable '" << name << "' not declared." << endl;
            exit(1);
        }
    }

    void markInitialized(const string &name)
    {
        if (lookup(name))
        {
            symbols[name].initialized = true;
        }
    }
    void printTable() const
    {
        cout << "Symbol Table:" << endl;
        cout << "Name\tType\t\tScope\tInitialized" << endl;
        cout << "--------------------------------------------" << endl;
        for (const auto &entry : symbols)
        {
            const string &name = entry.first;
            const Symbol &symbol = entry.second;

            // Convert TokenType to a string for display purposes
            string typeStr;
            switch (symbol.type)
            {
            case T_INT:
                typeStr = "int";
                break;
            case T_FLOAT:
                typeStr = "float";
                break;
            case T_DOUBLE:
                typeStr = "double";
                break;
            case T_STRING:
                typeStr = "string";
                break;
            case T_BOOL:
                typeStr = "bool";
                break;
            case T_CHAR:
                typeStr = "char";
                break;
            default:
                typeStr = "unknown";
            }

            cout << name << "\t" << typeStr << "\t\t" << symbol.scopeLevel << "\t"
                 << (symbol.initialized ? "Yes" : "No") << endl;
        }
    }
};

struct TACInstruction
{
    string op;     // Operator (+, -, *, /, etc.)
    string arg1;   // First operand
    string arg2;   // Second operand (if any)
    string result; // Result variable
};

class TACGenerator
{
private:
    vector<TACInstruction> instructions;
    int tempCount;

public:
    TACGenerator() : tempCount(0) {}

    const vector<TACInstruction> &getInstructions() const
    {
        return instructions;
    }

    string newTemp()
    {
        return "t" + to_string(tempCount++);
    }

    void addInstruction(const string &op, const string &arg1, const string &arg2, const string &result)
    {
        instructions.push_back({op, arg1, arg2, result});
    }

    void printInstructions()
    {
        cout << "Three-Address Code:" << endl;
        for (const auto &instr : instructions)
        {
            cout << instr.result << " = " << instr.arg1 << " " << instr.op << " " << instr.arg2 << endl;
        }
    }
};

class Lexer
{
private:
    string src;
    size_t pos;
    int line;

public:
    Lexer(const string &src)
    {
        this->src = src;
        this->pos = 0;
        this->line = 1;
    }

    vector<Token> tokenize()
    {
        vector<Token> tokens;

        while (pos < src.size())
        {
            char current = src[pos];
            if (current == '\n')
            {
                line++;
            }

            if (isspace(current))
            {
                pos++;
                continue;
            }
            // Single line comment
            if (current == '/' && peek() == '/')
            {
                pos += 2;
                while (src[pos] != '\n')
                {
                    pos++;
                }
                continue;
            }
            // Multi-line comment
            if (current == '/' && peek() == '*')
            {
                pos += 2;
                while (pos < src.size())
                {
                    if (src[pos] == '*' && peek() == '/')
                    {
                        pos += 2;
                        break;
                    }
                    pos++;
                }
                continue;
            }

            if (isdigit(current))
            {
                tokens.push_back(Token{T_NUM, consumeNumber()});
                continue;
            }
            if (current == '"')
            {
                tokens.push_back(Token{T_SENTENCE, consumeString()});
                continue;
            }

            if (isalpha(current))
            {
                string word = consumeWord();
                if (word == "int")
                    tokens.push_back(Token{T_INT, word});
                else if (word == "float")
                    tokens.push_back(Token{T_FLOAT, word});
                else if (word == "double")
                    tokens.push_back(Token{T_DOUBLE, word});
                else if (word == "string")
                    tokens.push_back(Token{T_STRING, word});
                else if (word == "bool")
                    tokens.push_back(Token{T_BOOL, word});
                else if (word == "char")
                    tokens.push_back(Token{T_CHAR, word});
                else if (word == "if")
                    tokens.push_back(Token{T_IF, word});
                else if (word == "else")
                    tokens.push_back(Token{T_ELSE, word});
                else if (word == "return")
                    tokens.push_back(Token{T_RETURN, word});
                else if (word == "while")
                    tokens.push_back(Token{T_WHILE, word});
                else if (word == "for")
                    tokens.push_back(Token{T_FOR, word});
                else
                    tokens.push_back(Token{T_ID, word});
                continue;
            }

            switch (current)
            {
            case '=':
                if (peek() == '=')
                {
                    pos++;
                    tokens.push_back(Token{T_EQ, "=="});
                }
                else
                {
                    tokens.push_back(Token{T_ASSIGN, "="});
                }
                break;
            case '+':
                tokens.push_back(Token{T_PLUS, "+"});
                break;
            case '-':
                tokens.push_back(Token{T_MINUS, "-"});
                break;
            case '*':
                tokens.push_back(Token{T_MUL, "*"});
                break;
            case '/':
                tokens.push_back(Token{T_DIV, "/"});
                break;
            case '(':
                tokens.push_back(Token{T_LPAREN, "("});
                break;
            case ')':
                tokens.push_back(Token{T_RPAREN, ")"});
                break;
            case '{':
                tokens.push_back(Token{T_LBRACE, "{"});
                break;
            case '}':
                tokens.push_back(Token{T_RBRACE, "}"});
                break;
            case ';':
                tokens.push_back(Token{T_SEMICOLON, ";"});
                break;
            case '>':
                tokens.push_back(Token{T_GT, ">"});
                break;
            case '<':
                tokens.push_back(Token{T_LT, "<"});
                break;
            case '&':
                if (peek() == '&')
                {
                    pos++;
                    tokens.push_back(Token{T_AND, "&&"});
                }
                else
                {
                    cout << "Unexpected character '&' at line " << line << endl;
                    exit(1);
                }
                break;
            case '|':
                if (peek() == '|')
                {
                    pos++;
                    tokens.push_back(Token{T_OR, "||"});
                }
                else
                {
                    cout << "Unexpected character '|' at line " << line << endl;
                    exit(1);
                }
                break;
            case '!':
                if (peek() == '=')
                {
                    pos++;
                    tokens.push_back(Token{T_NEQ, "!="});
                }
                else
                {
                    cout << "Unexpected character '!' at line " << line << endl;
                    exit(1);
                }
                break;
            default:
                cout << "Unexpected character: " << current << " at line " << line << endl;
                exit(1);
            }
            pos++;
        }
        tokens.push_back(Token{T_EOF, ""});
        return tokens;
    }

    char peek()
    {
        if (pos + 1 >= src.size())
            return '\0';
        return src[pos + 1];
    }

    string consumeNumber()
    {
        size_t start = pos;
        while (pos < src.size() && isdigit(src[pos]))
            pos++;
        return src.substr(start, pos - start);
    }

    string consumeWord()
    {
        size_t start = pos;
        while (pos < src.size() && isalnum(src[pos]))
            pos++;
        return src.substr(start, pos - start);
    }
    string consumeString()
    {
        pos++; // Skip the opening double quote
        size_t start = pos;
        while (pos < src.size() && src[pos] != '"')
        {
            pos++;
        }

        if (pos >= src.size())
        {
            cout << "Error: Unterminated string at line " << line << endl;
            exit(1);
        }

        string str = src.substr(start, pos - start);
        pos++;
        return str;
    }

    int getLineNumber() const
    {
        return line;
    }
    string tokenTypeToString(TokenType type)
    {
        switch (type)
        {
        case T_INT:
            return "T_INT";
        case T_FLOAT:
            return "T_FLOAT";
        case T_DOUBLE:
            return "T_DOUBLE";
        case T_STRING:
            return "T_STRING";
        case T_BOOL:
            return "T_BOOL";
        case T_CHAR:
            return "T_CHAR";
        case T_ID:
            return "T_ID";
        case T_NUM:
            return "T_NUM";
        case T_IF:
            return "T_IF";
        case T_ELSE:
            return "T_ELSE";
        case T_RETURN:
            return "T_RETURN";
        case T_ASSIGN:
            return "T_ASSIGN";
        case T_PLUS:
            return "T_PLUS";
        case T_MINUS:
            return "T_MINUS";
        case T_MUL:
            return "T_MUL";
        case T_DIV:
            return "T_DIV";
        case T_LPAREN:
            return "T_LPAREN";
        case T_RPAREN:
            return "T_RPAREN";
        case T_LBRACE:
            return "T_LBRACE";
        case T_RBRACE:
            return "T_RBRACE";
        case T_SEMICOLON:
            return "T_SEMICOLON";
        case T_GT:
            return "T_GT";
        case T_LT:
            return "T_LT";
        case T_EQ:
            return "T_EQ";
        case T_NEQ:
            return "T_NEQ";
        case T_AND:
            return "T_AND";
        case T_OR:
            return "T_OR";
        case T_WHILE:
            return "T_WHILE";
        case T_FOR:
            return "T_FOR";
        case T_EOF:
            return "T_EOF";
        case T_SENTENCE:
            return "T_SENTENCE";
        default:
            return "UNKNOWN";
        }
    }
    void printTokens(const vector<Token> &tokens)
    {
        cout << "Tokens:" << endl;
        for (const Token &token : tokens)
        {
            cout << "Type: " << tokenTypeToString(token.type)
                 << ", Value: " << token.value << endl;
        }
    }
};

class Parser
{
private:
    vector<Token> tokens;
    size_t pos;
    Lexer &lexer;
    SymbolTable symbolTable;
    int currentScopeLevel;
    TACGenerator tacGenerator;

public:
    Parser(const vector<Token> &tokens, Lexer &lexer)
        : tokens(tokens), pos(0), lexer(lexer), currentScopeLevel(0) {}

    void parseProgram()
    {
        while (tokens[pos].type != T_EOF)
        {
            parseStatement();
        }
        cout << "Parsing completed successfully! No Syntax Error" << endl;
    }

    void parseStatement()
    {
        if (tokens[pos].type == T_INT || tokens[pos].type == T_FLOAT || tokens[pos].type == T_DOUBLE ||
            tokens[pos].type == T_STRING || tokens[pos].type == T_CHAR || tokens[pos].type == T_BOOL)
        {
            parseDeclaration();
        }
        else if (tokens[pos].type == T_ID)
        {
            parseAssignment();
        }
        else if (tokens[pos].type == T_IF)
        {
            parseIfStatement();
        }
        else if (tokens[pos].type == T_RETURN)
        {
            parseReturnStatement();
        }
        else if (tokens[pos].type == T_LBRACE)
        {
            parseBlock();
        }
        else if (tokens[pos].type == T_WHILE || tokens[pos].type == T_FOR)
        {
            parseLoop();
        }
        else
        {
            cout << "Syntax error: unexpected token " << tokens[pos].value << " at line " << lexer.getLineNumber() << endl;
            exit(1);
        }
    }

    void parseBlock()
    {
        expect(T_LBRACE);
        while (tokens[pos].type != T_RBRACE && tokens[pos].type != T_EOF)
        {
            parseStatement();
        }
        expect(T_RBRACE);
    }

    void parseDeclaration()
    {
        TokenType varType = tokens[pos].type;
        pos++;

        if (tokens[pos].type == T_ID)
        {
            string varName = tokens[pos].value;
            symbolTable.insert(varName, varType, currentScopeLevel);
            pos++;
            expect(T_SEMICOLON);
        }
        else
        {
            cout << "Syntax error: expected identifier after type at line " << lexer.getLineNumber() << endl;
            exit(1);
        }
    }

    void parseAssignment()
    {
        string varName = tokens[pos].value;
        string exprResult;

        if (!symbolTable.lookup(varName))
        {
            cout << "Error: Variable '" << varName << "' not declared at line " << lexer.getLineNumber() << endl;
            exit(1);
        }

        pos++;
        expect(T_ASSIGN);
        if (tokens[pos].type == T_SENTENCE)
        {
            pos++;
            expect(T_SEMICOLON);
        }
        else
        {
            exprResult = parseExpression();
            expect(T_SEMICOLON);
        }

        tacGenerator.addInstruction("=", exprResult, "", varName);
        symbolTable.markInitialized(varName);
    }

    void parseIfStatement()
    {
        expect(T_IF);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();
        if (tokens[pos].type == T_ELSE)
        {
            expect(T_ELSE);
            parseStatement();
        }
    }

    void parseLoop()
    {
        if (tokens[pos].type == T_WHILE)
        {
            expect(T_WHILE);
            expect(T_LPAREN);
            parseExpression();
            expect(T_RPAREN);
            parseStatement();
        }
        else if (tokens[pos].type == T_FOR)
        {
            expect(T_FOR);
            expect(T_LPAREN);
            parseStatement();
            parseExpression();
            expect(T_SEMICOLON);
            parseStatement();
            expect(T_RPAREN);
            parseStatement();
        }
    }

    void parseReturnStatement()
    {
        expect(T_RETURN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    string parseExpression()
    {
        string lhs = parseTerm();

        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS || tokens[pos].type == T_GT ||
               tokens[pos].type == T_LT || tokens[pos].type == T_EQ || tokens[pos].type == T_NEQ ||
               tokens[pos].type == T_AND || tokens[pos].type == T_OR)
        {
            TokenType op = tokens[pos].type;
            pos++;
            string rhs = parseTerm();
            string temp = tacGenerator.newTemp();
            string opStr;
            switch (op)
            {
            case T_PLUS:
                opStr = "+";
                break;
            case T_MINUS:
                opStr = "-";
                break;
            case T_GT:
                opStr = ">";
                break;
            case T_LT:
                opStr = "<";
                break;
            case T_EQ:
                opStr = "==";
                break;
            case T_NEQ:
                opStr = "!=";
                break;
            case T_AND:
                opStr = "&&";
                break;
            case T_OR:
                opStr = "||";
                break;
            default:
                opStr = "?";
            }
            tacGenerator.addInstruction(opStr, lhs, rhs, temp);
            lhs = temp;
        }
        return lhs;
    }

    string parseTerm()
    {
        string lhs = parseFactor();
        while (tokens[pos].type == T_MUL || tokens[pos].type == T_DIV)
        {
            TokenType op = tokens[pos].type;
            pos++;
            string rhs = parseFactor();
            string temp = tacGenerator.newTemp();
            string opStr = (op == T_MUL) ? "*" : "/";
            tacGenerator.addInstruction(opStr, lhs, rhs, temp);
            lhs = temp;
        }
        return lhs;
    }

    string parseFactor()
    {
        if (tokens[pos].type == T_NUM)
        {
            string numValue = tokens[pos].value;
            pos++;
            return numValue;
        }
        else if (tokens[pos].type == T_ID)
        {
            string varName = tokens[pos].value;
            pos++;
            return varName;
        }
        else if (tokens[pos].type == T_LPAREN)
        {
            pos++;
            string exprResult = parseExpression();
            expect(T_RPAREN);
            return exprResult;
        }
        else
        {
            cout << "Syntax error: expected number or identifier at line " << lexer.getLineNumber() << endl;
            exit(1);
        }
    }

    void expect(TokenType expected)
    {
        if (tokens[pos].type == expected)
        {
            pos++;
        }
        else
        {
            cout << "Syntax error: expected token " << expected << " at line " << lexer.getLineNumber() << endl;
            exit(1);
        }
    }
    SymbolTable &getSymbolTable()
    {
        return symbolTable;
    }
    void printTAC()
    {
        tacGenerator.printInstructions();
    }
    TACGenerator &getTACGenerator()
    {
        return tacGenerator;
    }
};

class CodeGenerator
{
public:
    void generateAssembly(const vector<TACInstruction> &intermediateCode)
    {

        vector<string> assemblyCode;

        for (const auto &instr : intermediateCode)
        {
            string instructionString = instr.result + " = " + instr.arg1 + " " + instr.op + " " + instr.arg2;

            // Process the instruction string as before
            vector<string> tokens = split(instructionString, ' ');
            // Parse the instruction

            if (tokens.size() == 3 && tokens[1] == "=")
            {
                // Handle assignment: a = b
                if (isNumber(tokens[2]))
                {
                    // Move immediate value to variable
                    assemblyCode.push_back("mov dword [" + tokens[0] + "], " + tokens[2]);
                }
                else
                {
                    // Move one variable to another
                    assemblyCode.push_back("mov eax, [" + tokens[2] + "]");
                    assemblyCode.push_back("mov [" + tokens[0] + "], eax");
                }
            }
            else if (tokens.size() == 5 && (tokens[3] == "+" || tokens[3] == "-" || tokens[3] == "*" || tokens[3] == "/"))
            {
                // Handle arithmetic operations: t1 = a + b
                assemblyCode.push_back("mov eax, [" + tokens[2] + "]");
                if (tokens[3] == "+")
                    assemblyCode.push_back("add eax, [" + tokens[4] + "]");
                else if (tokens[3] == "-")
                    assemblyCode.push_back("sub eax, [" + tokens[4] + "]");
                else if (tokens[3] == "*")
                    assemblyCode.push_back("imul eax, [" + tokens[4] + "]");
                else if (tokens[3] == "/")
                {
                    assemblyCode.push_back("mov edx, 0"); // Clear edx for division
                    assemblyCode.push_back("mov ebx, [" + tokens[4] + "]");
                    assemblyCode.push_back("idiv ebx");
                }
                assemblyCode.push_back("mov [" + tokens[0] + "], eax");
            }
            else if (tokens.size() >= 2 && tokens[0] == "return")
            {
                // Handle return: return x
                assemblyCode.push_back("mov eax, [" + tokens[1] + "]");
                assemblyCode.push_back("ret");
            }
            else if (tokens[0] == "if")
            {
                // Handle conditional jump: if t1 goto L1
                assemblyCode.push_back("cmp [" + tokens[1] + "], 0");
                assemblyCode.push_back("jne " + tokens[3]);
            }
            else if (tokens[0].find("L") == 0)
            {
                // Add labels
                assemblyCode.push_back(tokens[0] + ":");
            }
            else if (tokens[0] == "goto")
            {
                // Handle unconditional jump: goto L2
                assemblyCode.push_back("jmp " + tokens[1]);
            }
        }

        // Output the assembly code
        for (const auto &line : assemblyCode)
        {
            cout << line << endl;
        }
    }

private:
    bool isNumber(const string &s)
    {
        return !s.empty() && all_of(s.begin(), s.end(), ::isdigit);
    }

    vector<string> split(const string &str, char delimiter)
    {
        vector<string> tokens;
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }
};

int main(int argc, char *argv[])
{

    SymbolTable symTable;
    TACGenerator icg;

    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <source-file>" << endl;
        return 1;
    }

    ifstream file(argv[1]);
    if (!file)
    {
        cout << "Error: Cannot open file " << argv[1] << endl;
        return 1;
    }

    string input((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // Tokenizing phase of the compiler
    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();
    lexer.printTokens(tokens);

    // Parsing phase of the compiler
    Parser parser(tokens, lexer);
    parser.parseProgram();

    parser.getSymbolTable().printTable();
    // TAC is three address code and intermediate code generation
    parser.printTAC();
    CodeGenerator codeGen;

    // Get the TAC instructions from the parser
    vector<TACInstruction> tacInstructions = parser.getTACGenerator().getInstructions();

    cout << "\nGenerated Assembly Code:" << endl;
    codeGen.generateAssembly(tacInstructions);

    return 0;
}
