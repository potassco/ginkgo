#include <ginkgo/gtk-widgets/GridWidget.h>

#include <iostream>
#include <boost/assert.hpp>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GridWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

GridWidget::GridWidget(const Style &style)
:	m_style(style),
	m_displayedYMax{0.0},
	m_displayedYStep{0.0}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GridWidget::setYMax(double yMax)
{
	m_displayedYStep = computeStep(yMax);
	m_displayedYMax = std::ceil(yMax / m_displayedYStep) * m_displayedYStep;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double GridWidget::displayedYMax() const
{
	return m_displayedYMax;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double GridWidget::displayedYStep() const
{
	return m_displayedYStep;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool GridWidget::on_draw(const::Cairo::RefPtr<Cairo::Context> &context)
{
	std::function<double (double)> roundToPixels;

	if (m_style.roundToPixels)
		roundToPixels = [](double x){return std::round(x) + 0.5;};
	else
		roundToPixels = [](double x){return x;};

	context->save();

	const auto color = m_style.colorGrid;
	context->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
	context->set_line_width(m_style.lineWidthNormal);

	context->rectangle(roundToPixels(0.0), roundToPixels(0.0), get_allocation().get_width() - 2 * roundToPixels(0.0), get_allocation().get_height() - 2 * roundToPixels(0.0));
	context->stroke();

	if (m_displayedYMax >= 0.0)
	{
		const auto yStep = m_displayedYStep / m_displayedYMax;

		double y = get_allocation().get_height();
		size_t i = 0;

		while (y >= 0)
		{
			context->move_to(0.0, roundToPixels(y));
			context->line_to(get_allocation().get_width(), roundToPixels(y));
			context->stroke();

			y -= yStep * get_allocation().get_height();
			i++;
		}
	}

	context->restore();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double GridWidget::computeStep(double rangeWidth)
{
	if (rangeWidth <= 0.0)
		return 0.0;

	const double lastPowerOf10 = std::pow(10.0, std::floor(std::round(std::log(rangeWidth) / std::log(10.0) * 1.0e6) / 1.0e6));

	for (const auto stepRelative : {2.5, 2.0, 1.0, 0.5, 0.25})
	{
		const double step = stepRelative * lastPowerOf10;

		if (std::ceil(rangeWidth / step) >= 3 && std::ceil(rangeWidth / step) < 6)
			return step;
	}

	return 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
