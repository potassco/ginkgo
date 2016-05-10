#include <ginkgo/gtk-widgets/PieChartWidget.h>

#include <iostream>
#include <iomanip>
#include <numeric>
#include <boost/assert.hpp>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PieChartWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

PieChartWidget::PieChartWidget()
:	Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
	pack_start(m_title, Gtk::PACK_SHRINK);
	pack_start(m_content, Gtk::PACK_SHRINK);

	m_content.add(m_chartDrawingArea);

	m_title.set_justify(Gtk::JUSTIFY_CENTER);
	m_title.set_margin_bottom(20);

	m_chartDrawingArea.signal_draw().connect(sigc::mem_fun(*this, &PieChartWidget::drawChart));
	m_content.set_size_request(192, 192);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PieChartWidget::setTitle(std::string title)
{
	m_title.set_markup("<b>" + title + "</b>");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PieChartWidget::setTitleSpacing(double titleSpacing)
{
	m_title.set_margin_bottom(titleSpacing);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PieChartWidget::on_size_allocate(Gtk::Allocation &allocation)
{
	Gtk::Box::on_size_allocate(allocation);
	rearrange();
	Gtk::Box::on_size_allocate(allocation);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PieChartWidget::rearrange()
{
	m_content.move(m_chartDrawingArea, 0, 0);
	m_chartDrawingArea.set_size_request(m_content.get_allocation().get_width(), m_content.get_allocation().get_height());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PieChartWidget::setData(PieChartWidget::Data data)
{
	m_data = data;
	rearrange();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool PieChartWidget::drawChart(const::Cairo::RefPtr<Cairo::Context> &context)
{
	if (m_data.empty())
		return true;

	context->save();

	const double width = m_chartDrawingArea.get_allocation().get_width();
	const double height = m_chartDrawingArea.get_allocation().get_height();

	const double size = std::min(width, height);
	const double radius = size / 2.0;
	const double centerX = width / 2.0;
	const double centerY = height / 2.0;

	const double sum = std::accumulate(m_data.begin(), m_data.end(), 0.0, [](double sum, const PieData &pieData)
	{
		return sum + pieData.value;
	});

	// Draw pie
	double angle = -M_PI_2;

	for (const auto &pieData : m_data)
	{
		const double nextAngle = angle + pieData.value / sum * 2 * M_PI;
		context->set_source_rgb(pieData.color.get_red(), pieData.color.get_green(), pieData.color.get_blue());
		context->arc(centerX, centerY, radius, angle, nextAngle);
		context->line_to(centerX, centerY);
		context->fill();

		angle = nextAngle;
	}

	// Draw labels
	angle = -M_PI_2;

	for (const auto &pieData : m_data)
	{
		const double nextAngle = angle + pieData.value / sum * 2 * M_PI;

		const auto styleContext = get_style_context();

		const auto labelX = centerX + cos((angle + nextAngle) / 2.0) * radius / 2.0;
		const auto labelY = centerX + sin((angle + nextAngle) / 2.0) * radius / 2.0;

		if (!pieData.showLabel || pieData.value == 0)
		{
			angle = nextAngle;
			continue;
		}

		context->save();

		std::stringstream labelText;
		labelText << std::fixed << std::setprecision(1) << (pieData.value / sum * 100) << " %";

		const Gdk::RGBA color = styleContext->get_color();
		context->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());

		auto layout = Pango::Layout::create(context);
		layout->set_justify(true);
		layout->set_alignment(Pango::ALIGN_CENTER);
		layout->set_markup(labelText.str());
		layout->set_font_description(get_style_context()->get_font());

		const auto rectangle = layout->get_pixel_logical_extents();
		context->translate(labelX - rectangle.get_width() / 2.0, labelY - rectangle.get_height() / 2.0);

		layout->show_in_cairo_context(context);

		context->restore();
	}

	context->restore();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
