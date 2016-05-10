#ifndef __GTK_WIDGETS__GRID_WIDGET_H
#define __GTK_WIDGETS__GRID_WIDGET_H

#include <gtkmm.h>

#include <ginkgo/gtk-widgets/Style.h>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GridWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class GridWidget : public Gtk::DrawingArea
{
	public:
		GridWidget(const Style &style);

		void setYMax(double yMax);

		double displayedYMax() const;
		double displayedYStep() const;

	private:
		const Style &m_style;

		double m_displayedYMax;
		double m_displayedYStep;

		bool on_draw(const::Cairo::RefPtr<Cairo::Context> &context);

		static double computeStep(double rangeWidth);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
