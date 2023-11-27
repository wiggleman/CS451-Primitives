#pragma once


void callback(const std::string& msg, Parser::Host host_sndr);
void PFDCallback(Parser::Host host_crashed);
void run(Parser parser, std::vector<Parser::Host> hosts);

