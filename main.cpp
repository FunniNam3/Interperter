#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <optional>

using namespace std;

enum class TokenType : int
{
    Operand = 0,
    Operator = 1,
    Eof = 2
};

struct Token
{
    TokenType type;
    char value;

    Token(TokenType _type, char _value = '\0') : value(_value), type(_type) {};
};

enum class ExpressionType : int
{
    Operand = 0,
    Operation = 1
};

struct Lexer
{
    vector<Token> tokens;

    Lexer(string input)
    {
        for (char i : input)
        {
            if (i == ' ')
            {
                continue;
            }
            else if (isalnum(i))
            {
                tokens.push_back(Token(TokenType::Operand, i));
            }
            else
            {
                tokens.push_back(Token(TokenType::Operator, i));
            }
        }
        tokens.push_back(Token(TokenType::Eof));
        reverse(tokens.begin(), tokens.end());
    }

    Token next()
    {
        Token last = tokens.back();
        tokens.pop_back();
        return last;
    }
    Token peek()
    {
        return tokens.back();
    }
};

pair<float, float> infix_bind_power(char op)
{
    switch (op)
    {
    case '=':
        return {0.2, 0.1};
        break;
    case '+':
    case '-':
        return {1.0, 1.1};
        break;
    case '*':
    case '/':
        return {2.0, 2.1};
        break;
    case '^':
    case '√':
        return {3.1, 3.0};
        break;
    default:
        throw std::runtime_error("Unknown Operator");
        break;
    }
}

struct Expression
{
    ExpressionType type;
    char value;
    vector<Expression> expressions;

    Expression() = default;

    Expression(char _value) : type(ExpressionType::Operand), value(_value)
    {
    }

    Expression(char _value, vector<Expression> _expressions) : type(ExpressionType::Operation), value(_value), expressions(_expressions)
    {
    }

    Expression parse_Expression(Lexer &lexer, float min_bp)
    {
        Token firstToken = lexer.next();
        Expression lhs;
        if (firstToken.type == TokenType::Operator && firstToken.value == '(')
        {
            lhs = parse_Expression(lexer, 0.0);
            Token temp = lexer.next();
            if (temp.value != ')')
            {
                string error = "missing closing par";
                throw runtime_error(error);
            }
        }
        else if (firstToken.type != TokenType::Operand)
        {
            string error = "bad token 1: ";
            error += firstToken.value;
            throw runtime_error(error);
        }
        else
        {
            Expression temp(firstToken.value);
            lhs = temp;
        }

        while (true)
        {
            Token op = lexer.peek();
            if (op.type == TokenType::Eof)
            {
                break;
            }
            else if (op.type == TokenType::Operand)
            {
                string error = "bad token 2: ";
                error += op.value;
                throw runtime_error(error);
            }
            else if (op.value == ')')
            {
                break;
            }

            pair<float, float> bp = infix_bind_power(op.value);
            float l_bp = bp.first, r_bp = bp.second;

            if (l_bp < min_bp)
            {
                break;
            }
            lexer.next();

            Expression rhs = parse_Expression(lexer, r_bp);
            Expression temp(op.value, {lhs, rhs});
            lhs = temp;
        }
        return lhs;
    }

    Expression(string input)
    {
        Lexer lexer(input);
        *this = parse_Expression(lexer, 0.0);
    }

    optional<pair<char, Expression>> is_assign()
    {
        if (type == ExpressionType::Operation)
        {
            if (value == '=')
            {
                if (expressions[0].type == ExpressionType::Operand)
                {
                    if (isalpha(expressions[0].value))
                    {
                        return {{expressions[0].value, expressions[1]}};
                    }
                    else
                    {
                        string error = "Not a possible var name";
                        throw runtime_error(error);
                    }
                }
                else
                {
                    string error = "Not a possible var name";
                    throw runtime_error(error);
                }
            }
            return nullopt;
        }
        return nullopt;
    }

    float eval(unordered_map<char, float> *variables)
    {
        if (type == ExpressionType::Operand)
        {
            if (isalnum(value) && !isalpha(value))
            {
                return atof(&value);
            }
            else if (variables->count(value) == 1)
            {
                return (*variables)[value];
            }
            else
            {
                string error = "Undefined variable";
                throw runtime_error(error);
            }
        }
        else if (type == ExpressionType::Operation)
        {
            float lhs = expressions[0].eval(variables);
            float rhs = expressions[1].eval(variables);
            switch (value)
            {
            case '+':
                return lhs + rhs;
            case '-':
                return lhs - rhs;
            case '*':
                return lhs * rhs;
            case '/':
                return lhs / rhs;
            case '^':
                return pow(lhs, rhs);
            case '√':
                return pow(lhs, 1.0 / (rhs));
            default:
                string error = "Bad Operator";
                throw runtime_error(error);
            }
        }
    }
};

string to_String(Expression input)
{
    string result = "";
    if (input.type == ExpressionType::Operand)
    {
        result += input.value;
    }
    else
    {
        result += "(";
        result += input.value;
        result += " " + to_String(input.expressions[0]) + " ";
        result += to_String(input.expressions[1]);
        result += ")";
    }
    return result;
}

int main()
{
    unordered_map<char, float> variables;
    while (true)
    {
        cout << ">> ";
        string input;
        getline(cin, input);

        if (input == "q")
        {
            break;
        }

        Expression expr(input);
        optional<pair<char, Expression>> assign = expr.is_assign();
        if (assign.has_value())
        {
            float value = assign.value().second.eval(&variables);
            variables[assign.value().first] = value;
            continue;
        }
        float value = expr.eval(&variables);
        cout << value << endl;
    }
}