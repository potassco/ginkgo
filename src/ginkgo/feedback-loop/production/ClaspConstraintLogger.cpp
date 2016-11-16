#include <ginkgo/feedback-loop/production/ClaspConstraintLogger.h>

#include <iostream>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ClaspConstraintLogger
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ClaspConstraintLogger::ClaspConstraintLogger(std::stringstream &program, GroundConstraintBuffer &constraintBuffer,
	const Configuration<Plain> &configuration)
:	m_state{State::Full},
	m_clingoControl{{"--heuristic=Domain", "--dom-mod=1,16", "--loops=no", "--reverse-arcs=0", "--otfs=0", "--stats"}},
	m_constraintBuffer(constraintBuffer),
	m_currentConstraintID{0},
	m_seenSymbols{0},
	m_configuration(configuration)
{
	m_clingoControl.add("base", {}, program.str().c_str());
	m_clingoControl.ground({{"base", {}}});

	auto &claspFacade = *static_cast<Clasp::ClaspFacade *>(m_clingoControl.claspFacade());

	m_childEventHandler = claspFacade.ctx.eventHandler();

	claspFacade.ctx.setEventHandler(this, Clasp::SharedContext::report_conflict);

	const auto handleModel = [](auto model)
		{
			std::cout << "model found" << std::endl;

			return true;
		};

	const auto handleFinished = [](auto result)
		{
			std::cout << "[Info ] Terminated constraint extraction" << std::endl;
		};

	m_clingoSolveAsync = std::make_unique<Clingo::SolveAsync>(m_clingoControl.solve_async(handleModel, handleFinished));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::fill(size_t constraintBufferSize)
{
	{
		std::lock_guard<std::mutex> lock(m_constraintBufferMutex);
		m_constraintBufferSize = constraintBufferSize;

		if (m_constraintBuffer.size() >= constraintBufferSize)
			return;

		m_state = State::Filling;
	}

	m_constraintBufferCondition.notify_one();

	std::unique_lock<std::mutex> lock(m_constraintBufferMutex);
	m_constraintBufferCondition.wait(lock, [&](){return m_state == State::Full;});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::terminate()
{
	{
		std::lock_guard<std::mutex> lock(m_constraintBufferMutex);
		m_state = State::Terminated;
	}

	m_constraintBufferCondition.notify_one();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::onEvent(const Clasp::Event &event)
{
	m_childEventHandler->onEvent(event);

	const auto conflictEvent = Clasp::event_cast<Clasp::NewConflictEvent>(event);

	if (conflictEvent)
		log(*conflictEvent->solver, *conflictEvent->learnt, conflictEvent->info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::log(const Clasp::Solver &solver, const Clasp::LitVec &claspLiterals,
	const Clasp::ConstraintInfo &constraintInfo)
{
	if (solver.id() != 0)
		throw std::runtime_error("warning: multi-threading currently unsupported");

	const auto &outputTable = solver.sharedContext()->output;

	readSymbolTable(outputTable);

	const uint32_t lbdMax = 127;
	const auto lbdOriginal = constraintInfo.lbd();
	auto lbdAfterResolution = constraintInfo.lbd();

	const auto allowedVariables = Clasp::VarInfo::Input | Clasp::VarInfo::Output;

	Clasp::LitVec output;

	if (lbdOriginal > lbdMax)
	{
		std::cout << "\033[1;31mwarning: skipped conflict (LBD too high: " << lbdOriginal << ", maximum allowed: " << lbdMax << ")\033[0m" << std::endl;
		return;
	}

	if (!solver.resolveToFlagged(claspLiterals, allowedVariables, output, lbdAfterResolution))
	{
		std::cout << "\033[1;33mwarning: skipped conflict (cannot be resolved to selected variables)\033[0m" << std::endl;
		return;
	}

	Literals literals;
	literals.reserve(output.size());

	for (auto i = output.begin(); i != output.end(); i++)
	{
		const auto literal = ~*i;

		const auto *symbol = &m_symbols.at(literal.var());
		const auto sign = (symbol->claspLiteral == literal ? Literal::Sign::Positive : Literal::Sign::Negative);

		literals.emplace_back(Literal(sign, symbol));
	}

	std::sort(literals.begin(), literals.end());

	std::unique_lock<std::mutex> lock(m_constraintBufferMutex);
	m_constraintBufferCondition.wait(lock, [&](){return m_state == State::Filling || m_state == State::Terminated;});

	if (m_state == State::Terminated)
		return;

	GroundConstraint constraint(m_currentConstraintID, std::move(literals), lbdOriginal, lbdAfterResolution);

	if (constraint.degree() > m_configuration.maxDegree)
	{
		std::cout << "\033[1;33mwarning: skipped conflict (degree too high)\033[0m" << std::endl;
		return;
	}

	if (constraint.literals().size() > m_configuration.maxNumberOfLiterals)
	{
		std::cout << "\033[1;33mwarning: skipped conflict (too many literals)\033[0m" << std::endl;
		return;
	}

	const auto subsumed = std::find_if(m_constraintBuffer.cbegin(), m_constraintBuffer.cend(),
		[&constraint](const auto &otherConstraint)
		{
			return subsumes(otherConstraint, constraint);
		})
		!= m_constraintBuffer.cend();

	if (subsumed)
	{
		std::cout << "\033[1;33mwarning: skipped conflict (subsumed by previous one)\033[0m" << std::endl;
		return;
	}

	m_constraintBuffer.insert(constraint);

	m_currentConstraintID++;

	if (m_constraintBuffer.size() >= m_constraintBufferSize)
	{
		m_state = State::Full;

		lock.unlock();
		m_constraintBufferCondition.notify_one();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::readSymbolTable(const Clasp::OutputTable &outputTable)
{
	if (m_seenSymbols == outputTable.size())
		return;

	const char *noName = "<?>";

	std::for_each(outputTable.pred_begin(), outputTable.pred_end(),
		[&](const auto &predicate)
		{
			const auto variable = predicate.cond.var();

			if (m_symbols.size() <= variable)
				m_symbols.resize(variable + 1, {noName, Clingo::Symbol(), Clasp::lit_false()});

			if (m_symbols[variable].name == noName || (!predicate.cond.sign() && m_symbols[variable].claspLiteral.sign()))
				m_symbols[variable] = {predicate.name.c_str(), Clingo::parse_term(predicate.name.c_str()), predicate.cond};
		});

	m_seenSymbols = outputTable.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
