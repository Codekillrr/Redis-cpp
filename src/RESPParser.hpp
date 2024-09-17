#ifndef RESPPARSER_HPP
#define RESPPARSER_HPP

#include <iostream>
#include <vector>
#include <variant>
#include <sstream>
#include <string>

struct RESPArray;
using RESPValue = std::variant<int, long long, std::string, RESPArray>;

std::string upCase(std::string str);
RESPValue RESP_Parser(std::istringstream &pipeline);
RESPValue String_parser(std::istringstream &pipeline);
RESPValue Errror_parser(std::istringstream &pipeline);
RESPValue Int_parser(std::istringstream &pipeline);
RESPValue Long_parser(std::istringstream &pipeline);
RESPValue Bulk_parser(std::istringstream &pipeline);
RESPValue Array_parser(std::istringstream &pipeline);
void printRESP(RESPValue &value, char end = '\n');
RESPValue processCommand(RESPValue &value);

#endif
