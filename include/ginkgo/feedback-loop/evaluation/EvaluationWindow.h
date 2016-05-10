#ifndef __FEEDBACK_LOOP__EVALUATION__EVALUATION_WINDOW_H
#define __FEEDBACK_LOOP__EVALUATION__EVALUATION_WINDOW_H

#include <gtkmm.h>

#include <boost/filesystem.hpp>

#include <ginkgo/feedback-loop/analysis/ProductionAnalysis.h>
#include <ginkgo/feedback-loop/analysis/ConsumptionAnalysis.h>
#include <ginkgo/feedback-loop/evaluation/ComparisonPanel.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace evaluation
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EvaluationWindow
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class EvaluationWindow : public Gtk::Window
{
	public:
		EvaluationWindow();

		void loadResultsFromDirectory(const boost::filesystem::path &directory);

	private:
		void scanDirectory(const boost::filesystem::path &directory);

		void updateMaxValues();

		bool on_key_press_event(GdkEventKey *event);

		void handleOpenButtonClicked();

		std::vector<analysis::Analysis<Plain>> m_plainAnalyses;
		gtkWidgets::Style m_style;

		Gtk::HeaderBar m_headerBar;
		Gtk::ScrolledWindow m_container;
		Gtk::Box m_wrapper;
		Gtk::Box m_content;

		std::vector<std::unique_ptr<ComparisonPanel>> m_comparisonPanels;

		Gtk::Button m_openFileButton;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
