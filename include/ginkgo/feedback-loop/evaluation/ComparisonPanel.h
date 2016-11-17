#ifndef __FEEDBACK_LOOP__EVALUATION__COMPARISON_PANEL_H
#define __FEEDBACK_LOOP__EVALUATION__COMPARISON_PANEL_H

#include <gtkmm.h>

#include <ginkgo/gtk-widgets/BarChartWidget.h>
#include <ginkgo/gtk-widgets/TextBox.h>
#include <ginkgo/gtk-widgets/InfoWidget.h>
#include <ginkgo/gtk-widgets/PieChartWidget.h>
#include <ginkgo/gtk-widgets/ReflowBox.h>
#include <ginkgo/feedback-loop/analysis/Analysis.h>

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

class ComparisonPanel : public Gtk::Box
{
	public:
		ComparisonPanel(const gtkWidgets::Style &style);

		void setPlainAnalyses(const std::vector<analysis::Analysis<Plain>> *plainAnalyses);
		void setDisplayedMaxValue(double displayedMaxValue);
		double maxValue() const;

		sigc::signal<void> &onSelectionChanged();

	private:
		void refresh();

		sigc::signal<void> m_onSelectionChanged;

		const std::vector<analysis::Analysis<Plain>> *m_plainAnalyses;
		analysis::Analysis<Aggregated> m_totalAnalysis;

		const gtkWidgets::Style &m_style;

		double m_maxValue;
		double m_displayedMaxValue;

		Gtk::Box m_content;

		gtkWidgets::TextBox m_proofMethodBox;
		gtkWidgets::TextBox m_testingPolicyBox;
		gtkWidgets::TextBox m_minimizationStrategyBox;
		gtkWidgets::TextBox m_fluentClosureUsageBox;
		gtkWidgets::TextBox m_constraintsToExtractBox;
		gtkWidgets::TextBox m_instanceBox;
		gtkWidgets::TextBox m_feedbackTypeBox;

		Gtk::Box m_infoRowContainer;
		gtkWidgets::ReflowBox m_infoRow;

		gtkWidgets::InfoWidget m_runtimeTotalMeanInfo;
		gtkWidgets::InfoWidget m_abnormalFinishesInfo;

		gtkWidgets::InfoWidget m_numberOfMeasurementsInfo;
		gtkWidgets::PieChartWidget m_consumptionTimeoutsPieChart;

		gtkWidgets::InfoWidget m_proofsInfo;
		gtkWidgets::PieChartWidget m_successfulProofsPieChart;
		gtkWidgets::InfoWidget m_proofTimeInfo;

		gtkWidgets::InfoWidget m_minimizationProofsInfo;
		gtkWidgets::PieChartWidget m_successfulMinimizationProofsPieChart;
		gtkWidgets::InfoWidget m_minimizationProofTimeInfo;

		gtkWidgets::InfoWidget m_candidateDegreeInfo;
		gtkWidgets::InfoWidget m_candidateLiteralsInfo;

		gtkWidgets::InfoWidget m_feedbackExtractionTimeInfo;
		gtkWidgets::InfoWidget m_feedbackExtractionConstraintsInfo;

		gtkWidgets::PieChartWidget m_literalsEliminatedByMinimizationPieChart;

		gtkWidgets::PieChartWidget m_candidatesSkippedPieChart;

		Gtk::Box m_configurationBox;
		Gtk::Box m_barChartBox;
		gtkWidgets::BarChartWidget m_barChart;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
