#include <ginkgo/gtk-widgets/BarWidget.h>

#include <iostream>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BarWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

BarWidget::BarWidget(const Style &style)
:	m_style(style)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BarWidget::setData(Data data)
{
	m_data = data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BarWidget::setYMax(double yMax)
{
	m_yMax = yMax;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool BarWidget::on_draw(const::Cairo::RefPtr<Cairo::Context> &context)
{
	std::function<double (double)> roundToPixels;

	if (m_style.roundToPixels)
		roundToPixels = [](double x){return std::round(x) + 0.5;};
	else
		roundToPixels = [](double x){return x;};

	context->save();

	context->set_source_rgb(m_data.color.get_red(), m_data.color.get_green(), m_data.color.get_blue());

	const double width = get_allocation().get_width();
	const double maxHeight = get_allocation().get_height();

	const double height = m_data.value / m_yMax * maxHeight;

	context->rectangle(0, maxHeight - height, width, height);
	context->fill();

	const auto errorBarY1 = roundToPixels(m_data.ciLow / m_yMax * maxHeight);
	const auto errorBarY2 = roundToPixels(m_data.ciHigh / m_yMax * maxHeight);

	const auto color = m_style.colorPositiveLight;

	context->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
	context->set_line_width(m_style.lineWidthNormal);

	const auto centerX = roundToPixels(width / 2.0);
	const auto leftX = roundToPixels(width / 3.0);
	const auto rightX = roundToPixels(width * 2.0 / 3.0) + 2 * roundToPixels(0.0);

	context->move_to(centerX, maxHeight - errorBarY1);
	context->line_to(centerX, maxHeight - errorBarY2);
	context->stroke();

	// Do not draw error bar ending strokes if too small
	if (width >= 6.0)
	{
		if (m_data.ciLow < m_data.value)
		{
			context->move_to(leftX, maxHeight - errorBarY1);
			context->line_to(rightX, maxHeight - errorBarY1);
			context->stroke();
		}

		if (m_data.ciHigh > m_data.value)
		{
			context->move_to(leftX, maxHeight - errorBarY2);
			context->line_to(rightX, maxHeight - errorBarY2);
			context->stroke();
		}
	}

	context->restore();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
