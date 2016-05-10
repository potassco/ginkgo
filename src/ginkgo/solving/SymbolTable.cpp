#include <ginkgo/solving/SymbolTable.h>

#include <algorithm>
#include <iostream>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SymbolTable
//
////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string *SymbolTable::identifier(const std::string &identifier)
{
	const auto match = std::find_if(m_identifiers.cbegin(), m_identifiers.cend(), [&](const auto &storedIdentifier)
	{
		return *storedIdentifier == identifier;
	});

	if (match != m_identifiers.cend())
		return (*match).get();

	m_identifiers.push_back(std::make_unique<std::string>(identifier));
	return m_identifiers.back().get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
