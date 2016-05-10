#include <ginkgo/feedback-loop/evaluation/ComparisonPanel.h>

#include <iostream>
#include <glibmm.h>
#include <cairomm/cairomm.h>

#include <ginkgo/utils/Utils.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace evaluation
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ComparisonPanel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ComparisonPanel::ComparisonPanel(const gtkWidgets::Style &style)
:	Gtk::Box(Gtk::ORIENTATION_VERTICAL),
	m_style(style),
	m_maxValue{0.0},
	m_displayedMaxValue{0.0},
	m_content(Gtk::ORIENTATION_VERTICAL),
	m_barChart(style)
{
	m_content.set_margin_left(40);
	m_content.set_margin_right(40);
	m_content.set_margin_top(40);
	m_content.set_margin_bottom(40);
	m_content.set_spacing(40);

//	m_infoRow.set_spacing(40);
	m_infoRow.set_size_request(800, -1);
	m_infoRowContainer.pack_start(m_infoRow, Gtk::PACK_EXPAND_PADDING);
	pack_start(m_content, Gtk::PACK_SHRINK);
	pack_start(m_infoRowContainer, Gtk::PACK_SHRINK);

	m_infoRow.add(m_runtimeTotalMeanInfo);
	m_infoRow.add(m_abnormalFinishesInfo);
	m_infoRow.add(m_numberOfMeasurementsInfo);

	m_infoRow.add(m_proofTimeInfo);
	m_infoRow.add(m_minimizationProofTimeInfo);
	m_infoRow.add(m_hypothesisDegreeInfo);
	m_infoRow.add(m_hypothesisLiteralsInfo);

	m_infoRow.add(m_consumptionTimeoutsPieChart);

	m_infoRow.add(m_proofsInfo);
	m_infoRow.add(m_successfulProofsPieChart);
	m_infoRow.add(m_minimizationProofsInfo);
	m_infoRow.add(m_successfulMinimizationProofsPieChart);
	m_infoRow.add(m_literalsEliminatedByMinimizationPieChart);

	m_infoRow.add(m_feedbackExtractionTimeInfo);
	m_infoRow.add(m_feedbackExtractionConstraintsInfo);
	m_infoRow.add(m_hypothesesSkippedPieChart);

	m_runtimeTotalMeanInfo.setTitle("Ø Feedback Loop Runtime");
	m_runtimeTotalMeanInfo.set_valign(Gtk::ALIGN_START);

	m_abnormalFinishesInfo.setTitle("Abnormal Termination");
	m_abnormalFinishesInfo.set_valign(Gtk::ALIGN_START);

	m_numberOfMeasurementsInfo.setTitle("Measurements");
	m_numberOfMeasurementsInfo.set_valign(Gtk::ALIGN_START);

	m_consumptionTimeoutsPieChart.setTitle("Timeouts (Consumption)");
	m_consumptionTimeoutsPieChart.set_valign(Gtk::ALIGN_START);

	m_proofsInfo.setTitle("Proofs");
	m_proofsInfo.set_valign(Gtk::ALIGN_START);

	m_minimizationProofsInfo.setTitle("Minimization Proofs");
	m_minimizationProofsInfo.set_valign(Gtk::ALIGN_START);

	m_successfulProofsPieChart.setTitle("Successful Proofs");
	m_successfulProofsPieChart.set_valign(Gtk::ALIGN_START);

	m_successfulMinimizationProofsPieChart.setTitle("Successful Minimization Proofs");
	m_successfulMinimizationProofsPieChart.set_valign(Gtk::ALIGN_START);

	m_proofTimeInfo.setTitle("Ø Proof Solving Time");
	m_proofTimeInfo.set_valign(Gtk::ALIGN_START);

	m_minimizationProofTimeInfo.setTitle("Ø Minimization Proof Solving Time");
	m_minimizationProofTimeInfo.set_valign(Gtk::ALIGN_START);

	m_hypothesisDegreeInfo.setTitle("Ø Hypothesis Degree");
	m_hypothesisDegreeInfo.set_valign(Gtk::ALIGN_START);

	m_hypothesisLiteralsInfo.setTitle("Ø Hypothesis Literals");
	m_hypothesisLiteralsInfo.set_valign(Gtk::ALIGN_START);

	m_literalsEliminatedByMinimizationPieChart.setTitle("Literals Eliminated\nby Minimization");
	m_literalsEliminatedByMinimizationPieChart.set_valign(Gtk::ALIGN_START);

	m_feedbackExtractionTimeInfo.setTitle("Ø Extraction Time");
	m_feedbackExtractionTimeInfo.set_valign(Gtk::ALIGN_START);

	m_feedbackExtractionConstraintsInfo.setTitle("Ø Extracted Constraints");
	m_feedbackExtractionConstraintsInfo.set_valign(Gtk::ALIGN_START);

	m_hypothesesSkippedPieChart.setTitle("Hypotheses tested/\nskipped TLS/untested");
	m_hypothesesSkippedPieChart.set_valign(Gtk::ALIGN_START);

	m_proofMethodBox.onSelectedRowsChanged().connect(sigc::mem_fun(*this, &ComparisonPanel::refresh));
	m_testingPolicyBox.onSelectedRowsChanged().connect(sigc::mem_fun(*this, &ComparisonPanel::refresh));
	m_minimizationStrategyBox.onSelectedRowsChanged().connect(sigc::mem_fun(*this, &ComparisonPanel::refresh));
	m_fluentClosureUsageBox.onSelectedRowsChanged().connect(sigc::mem_fun(*this, &ComparisonPanel::refresh));
	m_constraintsToExtractBox.onSelectedRowsChanged().connect(sigc::mem_fun(*this, &ComparisonPanel::refresh));
	m_instanceBox.onSelectedRowsChanged().connect(sigc::mem_fun(*this, &ComparisonPanel::refresh));
	m_feedbackTypeBox.onSelectedRowsChanged().connect(sigc::mem_fun(*this, &ComparisonPanel::refresh));
	m_proofMethodBox.onSelectedRowsChanged().connect(m_onSelectionChanged);
	m_testingPolicyBox.onSelectedRowsChanged().connect(m_onSelectionChanged);
	m_minimizationStrategyBox.onSelectedRowsChanged().connect(m_onSelectionChanged);
	m_fluentClosureUsageBox.onSelectedRowsChanged().connect(m_onSelectionChanged);
	m_constraintsToExtractBox.onSelectedRowsChanged().connect(m_onSelectionChanged);
	m_instanceBox.onSelectedRowsChanged().connect(m_onSelectionChanged);
	m_feedbackTypeBox.onSelectedRowsChanged().connect(m_onSelectionChanged);

	m_configurationBox.pack_start(m_proofMethodBox);
	m_configurationBox.pack_start(m_testingPolicyBox);
	m_configurationBox.pack_start(m_minimizationStrategyBox);
	m_configurationBox.pack_start(m_fluentClosureUsageBox);
	m_configurationBox.pack_start(m_constraintsToExtractBox);
	m_configurationBox.pack_start(m_instanceBox);
	m_configurationBox.pack_start(m_feedbackTypeBox);

	m_content.pack_start(m_configurationBox, Gtk::PACK_SHRINK);
	m_content.pack_start(m_barChartBox, Gtk::PACK_SHRINK);

	m_barChartBox.pack_start(m_barChart, Gtk::PACK_SHRINK);
	m_barChartBox.set_halign(Gtk::ALIGN_CENTER);

	// TODO
	m_barChart.setTitle("");
	m_barChart.setYAxisUnit(100.0, " %");
	//m_barChart.setBarSpacing(0.0);
	//m_barChart.setBarWidth(5.0);

	show_all();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ComparisonPanel::setPlainAnalyses(const std::vector<analysis::Analysis<Plain>> *plainAnalyses)
{
	m_plainAnalyses = plainAnalyses;

	if (m_plainAnalyses->empty())
		return;

	m_totalAnalysis = analysis::Analysis<Aggregated>::aggregate(*m_plainAnalyses);

	m_proofMethodBox.clear();

	const auto &configuration = m_totalAnalysis.productionAnalysis.configuration;

	const auto &proofMethods = configuration.proofMethod.uniqueValues;
	const auto &availableTestingPolicies = configuration.testingPolicy.uniqueValues;
	const auto &availableMinimizationStrategies = configuration.minimizationStrategy.uniqueValues;
	const auto &availableFluentClosureUsages = configuration.fluentClosureUsage.uniqueValues;
	const auto &availableConstraintsToExtract = configuration.constraintsToExtract.uniqueValues;
	const auto &availableInstances = configuration.instance.uniqueValues;
	const auto &availableFeedbackTypes = plainAnalyses->front().feedbackTypes();

	std::for_each(proofMethods.cbegin(), proofMethods.cend(),
		[&](const auto &proofMethod)
		{
			std::stringstream proofMethodStream;
			proofMethodStream << proofMethod;

			m_proofMethodBox.addItem(proofMethodStream.str());
		});

	std::for_each(availableTestingPolicies.cbegin(), availableTestingPolicies.cend(),
		[&](const auto &testingPolicy)
		{
			std::stringstream testingPolicyStream;
			testingPolicyStream << testingPolicy;

			m_testingPolicyBox.addItem(testingPolicyStream.str());
		});

	std::for_each(availableMinimizationStrategies.cbegin(), availableMinimizationStrategies.cend(),
		[&](const auto &minimizationStrategy)
		{
			std::stringstream minimizationStrategyStream;
			minimizationStrategyStream << minimizationStrategy;

			m_minimizationStrategyBox.addItem(minimizationStrategyStream.str());
		});

	std::for_each(availableFluentClosureUsages.cbegin(), availableFluentClosureUsages.cend(),
		[&](const auto &fluentClosureUsage)
		{
			std::stringstream fluentClosureUsageStream;
			fluentClosureUsageStream << fluentClosureUsage;

			m_fluentClosureUsageBox.addItem(fluentClosureUsageStream.str());
		});

	std::for_each(availableConstraintsToExtract.cbegin(), availableConstraintsToExtract.cend(),
		[&](const auto &constraintsToExtract)
		{
			std::stringstream constraintsToExtractStream;
			constraintsToExtractStream << constraintsToExtract;

			m_constraintsToExtractBox.addItem(constraintsToExtractStream.str());
		});

	std::for_each(availableInstances.cbegin(), availableInstances.cend(),
		[&](const auto &instance)
		{
			/*const auto position1 = encoding.find("/") + 1;
			const auto position2 = encoding.find("/asp_domain/domain.pddl");

			if (position2 > position1)
				m_instanceBox.addItem(encoding.substr(position1, position2 - position1));
			else
				m_instanceBox.addItem("Unknown");*/

			m_instanceBox.addItem(instance.string());
		});

	std::for_each(availableFeedbackTypes.cbegin(), availableFeedbackTypes.cend(),
		[&](const auto &feedbackType)
		{
			std::stringstream feedbackTypeStream;
			feedbackTypeStream << feedbackType;

			m_feedbackTypeBox.addItem(feedbackTypeStream.str());
		});

	m_proofMethodBox.selectAll();
	m_testingPolicyBox.selectAll();
	m_minimizationStrategyBox.selectAll();
	m_fluentClosureUsageBox.selectAll();
	m_constraintsToExtractBox.selectAll();
	m_instanceBox.selectAll();
	m_feedbackTypeBox.selectAll();

	refresh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ComparisonPanel::setDisplayedMaxValue(double displayedMaxValue)
{
	m_displayedMaxValue = displayedMaxValue;

	refresh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double ComparisonPanel::maxValue() const
{
	return m_maxValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ComparisonPanel::refresh()
{
	m_runtimeTotalMeanInfo.setData({"", "", Gdk::RGBA("#505050")});
	m_numberOfMeasurementsInfo.setData({"", "", Gdk::RGBA("#505050")});

	m_numberOfMeasurementsInfo.setData({"0", "", Gdk::RGBA("#505050")});
	m_consumptionTimeoutsPieChart.setData({{1, false, Gdk::RGBA("#505050")}});

	m_proofsInfo.setData({"", "", Gdk::RGBA("#505050")});
	m_successfulProofsPieChart.setData({{1, false, Gdk::RGBA("#505050")}});
	m_proofTimeInfo.setData({"", "", Gdk::RGBA("#505050")});

	m_minimizationProofsInfo.setData({"", "", Gdk::RGBA("#505050")});
	m_successfulMinimizationProofsPieChart.setData({{1, false, Gdk::RGBA("#505050")}});
	m_minimizationProofTimeInfo.setData({"", "", Gdk::RGBA("#505050")});

	m_hypothesisDegreeInfo.setData({"", "", Gdk::RGBA("#505050")});
	m_hypothesisLiteralsInfo.setData({"", "", Gdk::RGBA("#505050")});

	m_literalsEliminatedByMinimizationPieChart.setData({{1, false, Gdk::RGBA("#505050")}});

	m_feedbackExtractionTimeInfo.setData({"", "", Gdk::RGBA("#505050")});
	m_feedbackExtractionConstraintsInfo.setData({"", "", Gdk::RGBA("#505050")});

	m_hypothesesSkippedPieChart.setData({{1, false, Gdk::RGBA("#505050")}});

	const auto selectedProofMethodRows = m_proofMethodBox.selectedRows();
	const auto selectedTestingPolicyRows = m_testingPolicyBox.selectedRows();
	const auto selectedMinimizationStrategyRows = m_minimizationStrategyBox.selectedRows();
	const auto selectedFluentClosureUsageRows = m_fluentClosureUsageBox.selectedRows();
	const auto selectedConstraintsToExtractRows = m_constraintsToExtractBox.selectedRows();
	const auto selectedInstanceRows = m_instanceBox.selectedRows();
	const auto selectedFeedbackTypeRows = m_feedbackTypeBox.selectedRows();

	std::vector<production::ProofMethod> selectedProofMethods;
	std::vector<production::TestingPolicy> selectedTestingPolicies;
	std::vector<production::MinimizationStrategy> selectedMinimizationStrategies;
	std::vector<production::FluentClosureUsage> selectedFluentClosureUsages;
	std::vector<size_t> selectedConstraintsToExtract;
	std::vector<boost::filesystem::path> selectedInstances;
	std::vector<consumption::FeedbackType> selectedFeedbackTypes;

	const auto &configuration = m_totalAnalysis.productionAnalysis.configuration;

	for (const auto &row : selectedProofMethodRows)
		selectedProofMethods.push_back(configuration.proofMethod.uniqueValues[row->get_index()]);

	for (const auto &row : selectedTestingPolicyRows)
		selectedTestingPolicies.push_back(configuration.testingPolicy.uniqueValues[row->get_index()]);

	for (const auto &row : selectedMinimizationStrategyRows)
		selectedMinimizationStrategies.push_back(configuration.minimizationStrategy.uniqueValues[row->get_index()]);

	for (const auto &row : selectedFluentClosureUsageRows)
		selectedFluentClosureUsages.push_back(configuration.fluentClosureUsage.uniqueValues[row->get_index()]);

	for (const auto &row : selectedConstraintsToExtractRows)
		selectedConstraintsToExtract.push_back(configuration.constraintsToExtract.uniqueValues[row->get_index()]);

	for (const auto &row : selectedInstanceRows)
		selectedInstances.push_back(configuration.instance.uniqueValues[row->get_index()]);

	for (const auto &row : selectedFeedbackTypeRows)
		selectedFeedbackTypes.push_back(m_totalAnalysis.feedbackTypes()[row->get_index()]);

	if (selectedProofMethods.empty()
		|| selectedTestingPolicies.empty()
		|| selectedMinimizationStrategies.empty()
		|| selectedFluentClosureUsages.empty()
		|| selectedConstraintsToExtract.empty()
		|| selectedInstances.empty()
		|| selectedFeedbackTypes.empty())
		return;

	const auto selector =
		[&](const auto &analysis)
		{
			const auto &configuration = analysis.productionAnalysis.configuration;

			if (std::find(selectedProofMethods.cbegin(), selectedProofMethods.cend(), configuration.proofMethod) == selectedProofMethods.cend())
				return false;

			if (std::find(selectedTestingPolicies.cbegin(), selectedTestingPolicies.cend(), configuration.testingPolicy) == selectedTestingPolicies.cend())
				return false;

			if (std::find(selectedMinimizationStrategies.cbegin(), selectedMinimizationStrategies.cend(), configuration.minimizationStrategy) == selectedMinimizationStrategies.cend())
				return false;

			if (std::find(selectedFluentClosureUsages.cbegin(), selectedFluentClosureUsages.cend(), configuration.fluentClosureUsage) == selectedFluentClosureUsages.cend())
				return false;

			if (std::find(selectedConstraintsToExtract.cbegin(), selectedConstraintsToExtract.cend(), configuration.constraintsToExtract) == selectedConstraintsToExtract.cend())
				return false;

			if (std::find(selectedInstances.cbegin(), selectedInstances.cend(), configuration.instance) == selectedInstances.cend())
				return false;

			return true;
		};

	const auto partialAnalysis = analysis::Analysis<Aggregated>::aggregate(*m_plainAnalyses, selector);

	gtkWidgets::BarChartWidget::Data barChartData;
	barChartData.resize(2);

	const auto &feedbackType = selectedFeedbackTypes.front();

	const auto match = partialAnalysis.consumptionAnalyses.find(feedbackType);
	BOOST_ASSERT(match != partialAnalysis.consumptionAnalyses.cend());
	const auto &consumptionAnalysis = std::get<1>(*match);

	const auto baselineTime = consumptionAnalysis.measurements[0].timeMeasurement.time.geometric.mean;

	barChartData[0].push_back({1.0, 1.0, 1.0, std::string("base"), Gdk::RGBA("#808080")});

	std::cout << 0 << "\t" << baselineTime << std::endl;

	m_maxValue = 1.0;

	for (size_t i = 1; i < consumptionAnalysis.measurements.size(); i++)
	{
		const auto &measurement = consumptionAnalysis.measurements[i];
		const auto minTime = measurement.timeMeasurement.time.arithmetic.min / baselineTime;
		const auto time = measurement.timeMeasurement.time.geometric.mean / baselineTime;
		const auto timeout = measurement.timeMeasurement.timeout.arithmetic.max;

		std::cout << i << "\t" << time << std::endl;

		std::stringstream label;
		if (i % 4 == 1)
			label << measurement.selectedConstraints;

		Gdk::RGBA color;

		/*if (minTime == 0.0 && timeout)
			color = m_style.colorGray;
		else if (minTime == 0.0 && !timeout)
			color = Gdk::RGBA("#2baaa0");
		else if (minTime != 0.0 && timeout)
			color = Gdk::RGBA("#7962d3");
		else*/
			color = m_style.colorAccent;

		barChartData[1].push_back({time, time, time, label.str(), color});

		m_maxValue = std::max(m_maxValue, time);
	}

	m_barChart.setData(barChartData, 2.0);

	const auto &productionAnalysis = partialAnalysis.productionAnalysis;

	const auto runtimeTotalMean = productionAnalysis.runtime.arithmetic.mean;
	m_runtimeTotalMeanInfo.setData({toString(runtimeTotalMean, 1) + " s", "", Gdk::RGBA("#505050")});

	const auto abnormalFinishes = productionAnalysis.finishedNormally.sampleSize - productionAnalysis.finishedNormally.arithmetic.sum;
	m_abnormalFinishesInfo.setData({toString(abnormalFinishes), "", Gdk::RGBA("#505050")});

	const auto proofs = productionAnalysis.proofs.arithmetic.sum;
	const auto proofsSuccessful = productionAnalysis.proofsSuccessful.arithmetic.sum;
	const auto proofTimeGroundingTotal = productionAnalysis.proofTimeGroundingTotal.arithmetic.sum;
	const auto proofTimeSolvingTotal = productionAnalysis.proofTimeSolvingTotal.arithmetic.sum;

	const auto proofTimeGroundingMean = static_cast<double>(proofTimeGroundingTotal) / proofs;
	const auto proofTimeSolvingMean = static_cast<double>(proofTimeSolvingTotal) / proofs;

	m_proofTimeInfo.setData({toString(proofTimeSolvingMean, 3) + " s", "ground: " + toString(proofTimeGroundingMean, 3) + " s", Gdk::RGBA("#505050")});

	m_proofsInfo.setData({toString(static_cast<double>(proofs / productionAnalysis.proofs.sampleSize)), "", Gdk::RGBA("#505050")});

	if (proofs > 0 && proofsSuccessful > 0)
		m_successfulProofsPieChart.setData({{static_cast<double>(proofsSuccessful), true, Gdk::RGBA("#3080e9")}, {static_cast<double>(proofs - proofsSuccessful), false, Gdk::RGBA("#505050")}});

	const auto minimizationProofs = productionAnalysis.minimizationProofs.arithmetic.sum;
	const auto minimizationProofsSuccessful = productionAnalysis.minimizationProofsSuccessful.arithmetic.sum;
	const auto minimizationProofTimeGroundingTotal = productionAnalysis.minimizationProofTimeGroundingTotal.arithmetic.sum;
	const auto minimizationProofTimeSolvingTotal = productionAnalysis.minimizationProofTimeSolvingTotal.arithmetic.sum;

	const auto minimizationProofTimeGroundingMean = static_cast<double>(minimizationProofTimeGroundingTotal) / proofs;
	const auto minimizationProofTimeSolvingMean = static_cast<double>(minimizationProofTimeSolvingTotal) / proofs;

	m_minimizationProofTimeInfo.setData({toString(minimizationProofTimeSolvingMean, 3) + " s", "ground: " + toString(minimizationProofTimeGroundingMean, 3) + " s", Gdk::RGBA("#505050")});

	m_minimizationProofsInfo.setData({toString(static_cast<double>(minimizationProofs / productionAnalysis.minimizationProofs.sampleSize)), "", Gdk::RGBA("#505050")});

	if (minimizationProofs > 0 && minimizationProofsSuccessful > 0)
		m_successfulMinimizationProofsPieChart.setData({{static_cast<double>(minimizationProofsSuccessful), true, Gdk::RGBA("#3080e9")}, {static_cast<double>(proofs - proofsSuccessful), false, Gdk::RGBA("#505050")}});

	const auto minimizationLiteralsTotal = productionAnalysis.minimizationLiteralsTotal.arithmetic.sum;
	const auto minimizationLiteralsRemoved = productionAnalysis.minimizationLiteralsRemoved.arithmetic.sum;
	const auto minimizationLiteralsRemovedRatio = static_cast<double>(minimizationLiteralsRemoved) / minimizationLiteralsTotal;

	if (minimizationLiteralsRemovedRatio > 0)
		m_literalsEliminatedByMinimizationPieChart.setData({{minimizationLiteralsRemovedRatio, true, Gdk::RGBA("#3080e9")}, {1.0 - minimizationLiteralsRemovedRatio, false, Gdk::RGBA("#505050")}});

	const auto hypothesisDegreeTotal = productionAnalysis.hypothesisDegreeTotal.arithmetic.sum;
	const auto hypothesisDegreeMin = productionAnalysis.hypothesisDegreeMin.arithmetic.min;
	const auto hypothesisDegreeMax = productionAnalysis.hypothesisDegreeMax.arithmetic.max;
	const auto hypothesisDegreeMean = static_cast<double>(hypothesisDegreeTotal) / proofs;

	const auto hypothesisLiteralsTotal = productionAnalysis.hypothesisLiteralsTotal.arithmetic.sum;
	const auto hypothesisLiteralsMin = productionAnalysis.hypothesisLiteralsMin.arithmetic.min;
	const auto hypothesisLiteralsMax = productionAnalysis.hypothesisLiteralsMax.arithmetic.max;
	const auto hypothesisLiteralsMean = static_cast<double>(hypothesisLiteralsTotal) / proofs;

	m_hypothesisDegreeInfo.setData({toString(hypothesisDegreeMean, 1), "[" + toString(hypothesisDegreeMin) + ", " + toString(hypothesisDegreeMax) + "]", Gdk::RGBA("#505050")});
	m_hypothesisLiteralsInfo.setData({toString(hypothesisLiteralsMean, 1), "[" + toString(hypothesisLiteralsMin) + ", " + toString(hypothesisLiteralsMax) + "]", Gdk::RGBA("#505050")});

	/*const auto numberOfStepsPerMeasurement = evaluationResult.solvingTimes.size();
	const auto singleAnalyses = evaluationResult.singleAnalyses;
	const auto numberOfConsumptionTimeouts = static_cast<double>(evaluationResult.numberOfConsumptionTimeouts);

	m_numberOfMeasurementsInfo.setData({toString(singleAnalyses), "", Gdk::RGBA("#505050")});

	if (singleAnalyses > 0 && numberOfConsumptionTimeouts > 0)
		m_consumptionTimeoutsPieChart.setData({{numberOfConsumptionTimeouts, true, Gdk::RGBA("#3080e9")}, {singleAnalyses * numberOfStepsPerMeasurement - numberOfConsumptionTimeouts, false, Gdk::RGBA("#505050")}});

	const auto proofs = evaluationResult.proofsTotal;
	const auto successfulProofs = evaluationResult.proofsSuccessful;
	const auto minimizationTests = evaluationResult.minimizationTestsMean;

	m_proofsInfo.setData({toString(static_cast<double>(proofs) / singleAnalyses), "validate: " + toString(evaluationResult.hypothesesTestedMean) + "\nminimize: " + toString(minimizationTests), Gdk::RGBA("#505050")});

	if (proofs > 0 && successfulProofs > 0)
		m_successfulProofsPieChart.setData({{static_cast<double>(successfulProofs), true, Gdk::RGBA("#3080e9")}, {static_cast<double>(proofs - successfulProofs), false, Gdk::RGBA("#505050")}});

	const auto proofTimeSolvingMean = evaluationResult.proofTimeSolvingMean;
	const auto proofTimeGroundingMean = evaluationResult.proofTimeGroundingMean;

	m_proofTimeInfo.setData({toString(proofTimeSolvingMean, 3) + " s", "ground: " + toString(proofTimeGroundingMean, 3) + " s", Gdk::RGBA("#505050")});

	const auto hypothesisDegreeMin = evaluationResult.hypothesisDegreeMin;
	const auto hypothesisDegreeMean = evaluationResult.hypothesisDegreeMean;
	const auto hypothesisDegreeMax = evaluationResult.hypothesisDegreeMax;

	const auto hypothesisLiteralsMin = evaluationResult.hypothesisLiteralsMin;
	const auto hypothesisLiteralsMean = evaluationResult.hypothesisLiteralsMean;
	const auto hypothesisLiteralsMax = evaluationResult.hypothesisLiteralsMax;

	m_hypothesisDegreeInfo.setData({toString(hypothesisDegreeMean, 1), "[" + toString(hypothesisDegreeMin) + ", " + toString(hypothesisDegreeMax) + "]", Gdk::RGBA("#505050")});
	m_hypothesisLiteralsInfo.setData({toString(hypothesisLiteralsMean, 1), "[" + toString(hypothesisLiteralsMin) + ", " + toString(hypothesisLiteralsMax) + "]", Gdk::RGBA("#505050")});

	const auto literalsEliminatedByMinimization = evaluationResult.literalsRemovedByMinimization;

	if (literalsEliminatedByMinimization > 0)
		m_literalsEliminatedByMinimizationPieChart.setData({{literalsEliminatedByMinimization, true, Gdk::RGBA("#3080e9")}, {1.0 - literalsEliminatedByMinimization, false, Gdk::RGBA("#505050")}});

	const auto feedbackExtractionTimeMean = evaluationResult.feedbackExtractionTimeMean;
	const auto feedbackExtractionConstraintsMean = evaluationResult.feedbackExtractionConstraintsMean;
	const auto feedbackExtractionResumesMean = evaluationResult.feedbackExtractionResumesMean;
	const auto feedbackExtractionRestartsMean = evaluationResult.feedbackExtractionRestartsMean;

	m_feedbackExtractionTimeInfo.setData({toString(feedbackExtractionTimeMean, 1) + " s", "resumes: " + toString(feedbackExtractionResumesMean, 2) + "\nrestarts: " + toString(feedbackExtractionRestartsMean, 2), Gdk::RGBA("#505050")});
	m_feedbackExtractionConstraintsInfo.setData({toString(feedbackExtractionConstraintsMean), "", Gdk::RGBA("#505050")});

	const auto hypothesesTestedMean = evaluationResult.hypothesesTestedMean;
	const auto hypothesesSkippedContainsTerminalLiteralMean = evaluationResult.hypothesesSkippedContainsTerminalLiteralMean;
	const auto hypothesesSkippedDegreeTooHighMean = evaluationResult.hypothesesSkippedDegreeTooHighMean;
	const auto hypothesesSkippedContainsTooManyLiteralsMean = evaluationResult.hypothesesSkippedContainsTooManyLiteralsMean;
	const auto hypothesesSkippedSubsumedMean = evaluationResult.hypothesesSkippedSubsumedMean;

	m_hypothesesSkippedPieChart.setData({
		{hypothesesTestedMean, true, Gdk::RGBA("#3080e9")},
		{hypothesesSkippedContainsTerminalLiteralMean, false, Gdk::RGBA("#5c3566")},
		{hypothesesSkippedDegreeTooHighMean, false, Gdk::RGBA("#75507b")},
		{hypothesesSkippedContainsTooManyLiteralsMean, false, Gdk::RGBA("#92668d")},
		{hypothesesSkippedSubsumedMean, false, Gdk::RGBA("#ad7fa8")},
		{feedbackExtractionConstraintsMean - hypothesesTestedMean - hypothesesSkippedContainsTerminalLiteralMean - hypothesesSkippedDegreeTooHighMean - hypothesesSkippedContainsTooManyLiteralsMean - hypothesesSkippedSubsumedMean, false, Gdk::RGBA("#505050")}});*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sigc::signal<void> &ComparisonPanel::onSelectionChanged()
{
	return m_onSelectionChanged;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
