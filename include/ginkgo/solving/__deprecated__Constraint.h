#ifndef __SOLVING____DEPRECATED__CONSTRAINT_H
#define __SOLVING____DEPRECATED__CONSTRAINT_H

#include <iosfwd>
#include <memory>
#include <vector>
#include <tuple>

#include <ginkgo/solving/__deprecated__Literal.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Constraint;
using ConstraintPtr = std::shared_ptr<Constraint>;

class Constraint
{
	public:
		Constraint(size_t id, const std::string &string, SymbolTable &symbolTable);
		Constraint(size_t id, Literals literals, size_t lbd);

		size_t id() const;
		const Literals &literals() const;
		size_t numberOfLiterals() const;
		size_t lbd() const;

		bool subsumes(const Constraint &rhs) const;
		bool subsumes(const Constraint &rhs, int offset) const;

		bool containsIdentifier(const std::string &identifier) const;

		std::tuple<size_t, size_t> timeRange() const;

		ConstraintPtr withoutLiterals(size_t index, size_t length = 1) const;

		void removeLiterals(const std::string &literalName);

		void print(std::ostream &ostream) const;
		void print(std::ostream &ostream, Literal::OutputFormat outputFormat, int offset = 0) const;

	private:
		Constraint() = default;
		Constraint(const Constraint &copy) = delete;

		size_t m_id;
		Literals m_literals;
		size_t m_lbd;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Constraint &constraint);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
