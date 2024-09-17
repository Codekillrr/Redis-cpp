#include "RESPParser.hpp"
#include <algorithm>

struct RESPArray 
{
    std::vector<RESPValue> elements;
};

std::string upCase(std::string str) 
{ 
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){return std::toupper(c);});return str; 
}

RESPValue String_parser(std::istringstream &pipeline) 
{
    std::string line;
    getline(pipeline, line, '\r');
    pipeline.ignore();

    return line;
}

RESPValue Errror_parser(std::istringstream &pipeline) 
{
    std::string line;
    getline(pipeline, line, '\r');
    pipeline.ignore();

    return std::string("(ERROR: " + line +")");
}

RESPValue Int_parser(std::istringstream &pipeline) 
{
    std::string line;
    getline(pipeline, line, '\r');
    pipeline.ignore();

    return std::stoi(line);
}

RESPValue Long_parser(std::istringstream &pipeline) 
{
    std::string line;
    getline(pipeline, line, '\r');
    pipeline.ignore();

    return std::stoll(line);
}

RESPValue Bulk_parser(std::istringstream &pipeline) 
{
    std::string size;
    getline(pipeline, size, '\r');
    pipeline.ignore();

    std::string line;
    getline(pipeline, line, '\r');
    pipeline.ignore();

    return line;
}

RESPValue Array_parser(std::istringstream &pipeline) 
{
    std::string size;
    getline(pipeline, size, '\r');
    pipeline.ignore();

    RESPArray array;

    for(int i = 0; i < stoi(size); ++i) 
    {
        array.elements.push_back(RESP_Parser(pipeline));
    }

    return array;
}

RESPValue RESP_Parser(std::istringstream &pipeline)
{
    char type = pipeline.get();

    switch (type)
    {
        case '+':
            return String_parser(pipeline);
            break;
        case '-':
            return Errror_parser(pipeline);
            break;
        case ':':
            return Int_parser(pipeline);
            break;
        case '(':
            return Long_parser(pipeline);
            break;
        case '$':
            return Bulk_parser(pipeline);
            break;
        case '*':
            return Array_parser(pipeline);
            break;
        default:
            return std::string("\\NOT A RESP COMMAND");
            break;
    }
}

void printRESP(RESPValue &value, char end = '\n') 
{
    if(std::holds_alternative<std::string>(value))
    {
        std::cout << std::get<std::string>(value) << end; 
    } 
    else if(std::holds_alternative<int>(value))
    {
        std::cout << std::get<int>(value) << end;
    }
    else if(std::holds_alternative<long long>(value))
    {
        std::cout << std::get<long long>(value) << end;
    }
    else if(std::holds_alternative<RESPArray>(value))
    {
        std::cout << "[ ";
        for(RESPValue item: std::get<RESPArray>(value).elements)
        {
            printRESP(item, ' ');
        }
        std::cout << "]";
        std::cout << end;
    }
}

RESPValue processCommand(RESPValue &value)
{
    printRESP(value);
    
    if(std::holds_alternative<std::string>(value))
    {
        return std::string("+OK\r\n");
    } 
    else if(std::holds_alternative<int>(value))
    {
        return std::string("+OK\r\n");
    }
    else if(std::holds_alternative<long long>(value))
    {
        return std::string("+OK\r\n");
    }
    else if(std::holds_alternative<RESPArray>(value))
    {
        auto array = std::get<RESPArray>(value).elements; 
        if(std::holds_alternative<std::string>(array[0]))
        {
            if(upCase(std::get<std::string>(array[0])) == "ECHO")
            {
                if(array.size() != 2)
                {
                    return std::string("-ECHO must have 2 arguments\r\n");
                }

                if(std::holds_alternative<std::string>(array[1]))
                {
                    std::string arg = std::get<std::string>(array[1]);
                    return std::string("$" + std::to_string(arg.size()) + arg + "\r\n");
                }
            }
        }
    }
}