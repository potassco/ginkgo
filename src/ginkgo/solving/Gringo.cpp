#include <ginkgo/solving/Gringo.h>

#include <fstream>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Gringo
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Gringo::Gringo(boost::filesystem::path binary, std::vector<std::string> arguments)
:	Process(binary.string(), arguments)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
