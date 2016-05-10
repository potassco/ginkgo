#ifndef __GTK_WIDGETS__BAR_CHART_WIDGET_H
#define __GTK_WIDGETS__BAR_CHART_WIDGET_H

#include <gtkmm.h>

#include <ginkgo/gtk-widgets/GridWidget.h>
#include <ginkgo/gtk-widgets/BarWidget.h>
#include <ginkgo/gtk-widgets/Style.h>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BarChartWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class BarChartWidget : public Gtk::Box
{
	public:
		using Data = std::vector<std::vector<BarWidget::Data>>;

	public:
		BarChartWidget(const Style &style);

		void setTitle(std::string title);
		void setData(Data data, double maxValue = 0.0);
		void setYAxisUnit(double factor, std::string label);

	private:
		void on_size_allocate(Gtk::Allocation &allocation);

		void rearrange();

		const Style &m_style;

		Gtk::Layout m_content;
		Gtk::Label m_title;

		GridWidget m_grid;

		double m_yAxisUnitFactor;
		std::string m_yAxisUnitLabel;

		std::vector<std::unique_ptr<Gtk::Label>> m_yAxisLabels;
		std::vector<std::unique_ptr<Gtk::Label>> m_xAxisLabels;
		std::vector<std::unique_ptr<BarWidget>> m_barWidgets;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
