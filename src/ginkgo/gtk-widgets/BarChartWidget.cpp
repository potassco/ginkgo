#include <ginkgo/gtk-widgets/BarChartWidget.h>

#include <iostream>
#include <numeric>
#include <boost/assert.hpp>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BarChartWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

BarChartWidget::BarChartWidget(const Style &style)
:	Gtk::Box(Gtk::ORIENTATION_VERTICAL),
	m_style(style),
	m_grid(m_style),
	m_yAxisUnitFactor{1},
	m_yAxisUnitLabel{""}
{
	m_title.override_color(style.colorPositive);

	pack_start(m_title, Gtk::PACK_SHRINK);
	pack_start(m_content, Gtk::PACK_SHRINK);

	m_content.add(m_grid);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BarChartWidget::setTitle(std::string title)
{
	m_title.set_markup("<b>" + title + "</b>");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BarChartWidget::setYAxisUnit(double factor, std::string label)
{
	m_yAxisUnitFactor = factor;
	m_yAxisUnitLabel = label;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BarChartWidget::setData(Data data, double maxValue)
{
	// Compute maximum value
	double yMax = 0.0;

	if (maxValue > 0.0)
		yMax = maxValue;
	else
		yMax = std::accumulate(data.begin(), data.end(), 0.0,
			[this](double currentMaxValue, const std::vector<BarWidget::Data> &groupData)
			{
				return std::max(currentMaxValue,
					std::accumulate(groupData.begin(), groupData.end(), 0.0, [](double maxValue, const BarWidget::Data &data)
					{
						return std::max(maxValue, data.value);
					}));
			});

	m_grid.setYMax(yMax);

	// Create new y axis labels
	for (const auto &label : m_yAxisLabels)
		m_content.remove(*label);

	m_yAxisLabels.clear();

	const double displayedYMax = m_grid.displayedYMax();
	const double displayedYStep = m_grid.displayedYStep();

	for (double y = 0.0; y <= displayedYMax; y += displayedYStep)
	{
		std::unique_ptr<Gtk::Label> label(new Gtk::Label);

		std::stringstream labelString;
		labelString << "<span color=\"grey\">" << y * m_yAxisUnitFactor << m_yAxisUnitLabel << "</span>";
		label->set_markup(labelString.str());
		m_content.add(*label);

		m_yAxisLabels.push_back(std::move(label));
	}

	// Create new bar widgets and labels
	for (const auto &barWidget : m_barWidgets)
		m_content.remove(*barWidget);

	for (const auto &label : m_xAxisLabels)
		m_content.remove(*label);

	m_barWidgets.clear();
	m_xAxisLabels.clear();

	for (size_t groupID = 0; groupID < data.size(); groupID++)
	{
		size_t i = 0;

		for (const auto &barData : data[groupID])
		{
			std::unique_ptr<BarWidget> barWidget(new BarWidget(m_style));

			barWidget->setData(barData);
			m_content.add(*barWidget);

			m_barWidgets.push_back(std::move(barWidget));

			std::unique_ptr<Gtk::Label> label(new Gtk::Label);

			std::stringstream labelString;
			labelString << "<span color=\"grey\">" << barData.label << "</span>";
			label->set_markup(labelString.str());
			label->set_angle(0);
			m_content.add(*label);

			m_xAxisLabels.push_back(std::move(label));

			i++;
		}

		if (groupID < data.size())
			m_barWidgets.back()->set_margin_right(m_style.barChartGroupSpacing);
	}

	// Compute minimal size
	double x = m_style.barChartYAxisWidth + m_style.barChartBarSpacing;

	for (size_t i = 0; i < m_barWidgets.size(); i++)
		x += m_style.barChartBarWidth + m_style.barChartBarSpacing + m_barWidgets[i]->get_margin_right();

	x += -m_style.barChartBarSpacing + m_style.barChartYAxisWidth;

	show_all();

	m_content.set_size_request(x, m_style.barChartHeight);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BarChartWidget::on_size_allocate(Gtk::Allocation &allocation)
{
	Gtk::Box::on_size_allocate(allocation);
	rearrange();
	Gtk::Box::on_size_allocate(allocation);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BarChartWidget::rearrange()
{
	BOOST_ASSERT(m_xAxisLabels.size() == m_barWidgets.size());

	const auto &allocation = m_content.get_allocation();
	const auto &width = allocation.get_width();
	const auto &height = allocation.get_height() - m_style.barChartTitleSpacing;

	m_content.move(m_grid, m_style.barChartYAxisWidth, m_style.barChartTitleSpacing);
	m_grid.set_size_request(width - 2 * m_style.barChartYAxisWidth, height - m_style.barChartXAxisHeight);

	for (size_t i = 0; i < m_yAxisLabels.size(); i++)
	{
		auto &label = *m_yAxisLabels[i];

		if (m_style.showLabels)
			label.show();
		else
			label.hide();

		const double x = m_style.barChartYAxisWidth - label.get_allocation().get_width() - 10;
		const double y = m_style.barChartTitleSpacing + (double)(m_yAxisLabels.size() - i - 1) / (m_yAxisLabels.size() - 1) * (height - m_style.barChartXAxisHeight) - label.get_allocation().get_height() / 2;
		m_content.move(label, x, y);
	}

	const double displayedYMax = m_grid.displayedYMax();

	double x = m_style.barChartYAxisWidth + m_style.barChartBarSpacing;

	for (size_t i = 0; i < m_barWidgets.size(); i++)
	{
		auto &barWidget = *m_barWidgets[i];
		auto &barLabel = *m_xAxisLabels[i];

		if (m_style.showLabels)
			barLabel.show();
		else
			barLabel.hide();

		barWidget.set_size_request(m_style.barChartBarWidth, height - m_style.barChartXAxisHeight);
		barWidget.setYMax(displayedYMax);
		m_content.move(barWidget, x, m_style.barChartTitleSpacing);

		m_content.move(barLabel, x + (m_style.barChartBarWidth - barLabel.get_allocation().get_width()) / 2, m_style.barChartTitleSpacing + height - m_style.barChartXAxisHeight + 10);

		x += m_style.barChartBarWidth + m_style.barChartBarSpacing + barWidget.get_margin_right();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

