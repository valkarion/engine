#pragma once

#include <vector>
#include <string>

#define UNSET_ID	-1
#define UNSET_S		"__unset"

std::vector<std::string> SplitString( const std::string &text, char sep );