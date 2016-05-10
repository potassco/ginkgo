#include <ginkgo/feedback-loop/evaluation/EvaluationWindow.h>

#include <iostream>
#include <iomanip>
#include <glibmm.h>
#include <cairomm/cairomm.h>

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

EvaluationWindow::EvaluationWindow()
:	m_style(gtkWidgets::Style::styleGTK(get_style_context())),
	m_content(Gtk::ORIENTATION_HORIZONTAL)
{
	resize(1024, 768);

	set_position(Gtk::WIN_POS_CENTER);
	set_wmclass("Feedback Loop Evaluation", "Feedback Loop Evaluation");
	set_icon_name("logview");

	override_background_color(m_style.colorBackground);

	m_openFileButton.set_label("Open");
	m_openFileButton.show();
	m_openFileButton.signal_clicked().connect(sigc::mem_fun(*this, &EvaluationWindow::handleOpenButtonClicked));
	m_headerBar.set_show_close_button(true);
	m_headerBar.set_title("Feedback Loop Evaluation");
	m_headerBar.pack_start(m_openFileButton);
	m_headerBar.show();
	set_titlebar(m_headerBar);

	m_content.set_margin_left(40);
	m_content.set_margin_right(40);
	m_content.set_margin_top(40);
	m_content.set_margin_bottom(40);
	m_content.set_spacing(40);

	m_container.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

	// Comparison panels
	for (size_t i = 0; i < 1; i++)
	{
		auto comparisonPanel = std::make_unique<ComparisonPanel>(m_style);
		m_content.pack_start(*comparisonPanel, Gtk::PACK_EXPAND_PADDING);
		comparisonPanel->show_all();
		comparisonPanel->onSelectionChanged().connect(sigc::mem_fun(*this, &EvaluationWindow::updateMaxValues));
		m_comparisonPanels.push_back(std::move(comparisonPanel));
	}

	m_wrapper.pack_start(m_content, Gtk::PACK_EXPAND_WIDGET);
	m_container.add(m_wrapper);
	add(m_container);

	show_all();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool EvaluationWindow::on_key_press_event(GdkEventKey *event)
{
	if (event->state & GDK_CONTROL_MASK && (event->keyval == GDK_KEY_w || event->keyval == GDK_KEY_q))
		close();

	if (event->state & GDK_CONTROL_MASK && event->keyval == GDK_KEY_o)
		handleOpenButtonClicked();

	if (event->keyval == GDK_KEY_p)
	{
		auto surface = Cairo::PdfSurface::create("/tmp/test.pdf", get_allocation().get_width(), get_allocation().get_height());
		auto context = Cairo::Context::create(surface);
		draw(context);
		surface->finish();
	}

	if (event->keyval == GDK_KEY_f)
	{
		resize(1900, 4000);
	}

	return Gtk::Window::on_key_press_event(event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void EvaluationWindow::handleOpenButtonClicked()
{
/*	Gtk::FileChooserDialog fileChooserDialog(*this, "Open", Gtk::FILE_CHOOSER_ACTION_OPEN);

	fileChooserDialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fileChooserDialog.add_button("_Open", Gtk::RESPONSE_OK);

	auto fileFilter = Gtk::FileFilter::create();
	fileFilter->set_name("Ginkgo result file");
	fileFilter->add_mime_type("text/csv");

	fileChooserDialog.add_filter(fileFilter);

	const int result = fileChooserDialog.run();

	if (result != Gtk::RESPONSE_OK)
		return;

	loadResultsFromDirectory(fileChooserDialog.get_filename());*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void EvaluationWindow::loadResultsFromDirectory(const boost::filesystem::path &directory)
{
	m_plainAnalyses.clear();

	scanDirectory(directory);

	for (auto &comparisonPanel : m_comparisonPanels)
		comparisonPanel->setPlainAnalyses(&m_plainAnalyses);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void EvaluationWindow::scanDirectory(const boost::filesystem::path &directory)
{
	boost::filesystem::directory_iterator i(directory), end;

	for (; i != end; i++)
		if (boost::filesystem::is_regular_file(*i) && i->path().extension() == ".analysis-produce")
		{
			const auto filePrefix = i->path().parent_path() / i->path().stem();

			std::cout << "[Info ] Reading " << filePrefix.string() << std::endl;

			const auto plainAnalysis = analysis::Analysis<Plain>::fromFiles(filePrefix);
			m_plainAnalyses.push_back(plainAnalysis);
		}
		else if (boost::filesystem::is_directory(*i))
			scanDirectory(*i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void EvaluationWindow::updateMaxValues()
{
	const auto maxValue = std::max(m_comparisonPanels[0]->maxValue(), m_comparisonPanels[1]->maxValue());

	std::for_each(m_comparisonPanels.begin(), m_comparisonPanels.end(),
		[&](auto &comparisonPanel)
		{
			comparisonPanel->setDisplayedMaxValue(maxValue);
		});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
