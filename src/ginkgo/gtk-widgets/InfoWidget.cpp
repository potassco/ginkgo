#include <ginkgo/gtk-widgets/InfoWidget.h>

#include <iostream>
#include <algorithm>
#include <boost/assert.hpp>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// InfoWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

InfoWidget::InfoWidget()
:	Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
	pack_start(m_title, Gtk::PACK_SHRINK);
	pack_start(m_content, Gtk::PACK_SHRINK);

	m_content.add(m_chartDrawingArea);

	m_title.set_justify(Gtk::JUSTIFY_CENTER);
	m_title.set_margin_bottom(20);

	m_chartDrawingArea.signal_draw().connect(sigc::mem_fun(*this, &InfoWidget::drawChart));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void InfoWidget::setTitle(std::string title)
{
	m_title.set_markup("<b>" + title + "</b>");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void InfoWidget::setTitleSpacing(double titleSpacing)
{
	m_title.set_margin_bottom(titleSpacing);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void InfoWidget::on_size_allocate(Gtk::Allocation &allocation)
{
	Gtk::Box::on_size_allocate(allocation);
	rearrange();
	Gtk::Box::on_size_allocate(allocation);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void InfoWidget::rearrange()
{
	m_content.move(m_chartDrawingArea, 0, 0);
	m_chartDrawingArea.set_size_request(m_content.get_allocation().get_width(), m_content.get_allocation().get_height());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void InfoWidget::setData(InfoWidget::Data data)
{
	m_data = data;

	if (m_data.subtitle.empty())
		m_content.set_size_request(192, 64);
	else
		m_content.set_size_request(192, 94);

	rearrange();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool InfoWidget::drawChart(const::Cairo::RefPtr<Cairo::Context> &context)
{
	context->save();

	const double width = m_chartDrawingArea.get_allocation().get_width();
	const double height = m_chartDrawingArea.get_allocation().get_height();

	context->set_source_rgb(m_data.color.get_red(), m_data.color.get_green(), m_data.color.get_blue());
	context->rectangle(0, 0, width, height);
	context->fill();

	const Gdk::RGBA color = get_style_context()->get_color();
	context->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());

	const double titleX = width / 2.0;
	const double titleY = height / 2.0;

	auto layout = Pango::Layout::create(context);
	layout->set_justify(true);
	layout->set_alignment(Pango::ALIGN_CENTER);

	if (m_data.subtitle.empty())
		layout->set_markup(m_data.title);
	else
		layout->set_markup(m_data.title + "\n<span size=\"x-small\">" + m_data.subtitle + "</span>");

	auto fontDescription = get_style_context()->get_font();
	fontDescription.set_size(2 * fontDescription.get_size());
	layout->set_font_description(fontDescription);

	const auto rectangle = layout->get_pixel_logical_extents();
	context->translate(titleX - rectangle.get_width() / 2.0, titleY - rectangle.get_height() / 2.0);

	layout->show_in_cairo_context(context);

	context->restore();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
