#pragma once

extern std::ofstream logFile;
void writeToLogFile(const std::string& message);
void callback(const char *msg, Parser::Host host_sndr);
void run(Parser parser, std::vector<Parser::Host> hosts);

