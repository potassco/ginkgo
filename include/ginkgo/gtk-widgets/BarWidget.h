#ifndef __GTK_WIDGETS__BAR_WIDGET_H
#define __GTK_WIDGETS__BAR_WIDGET_H

#include <gtkmm.h>

#include <ginkgo/gtk-widgets/Style.h>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BarWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class BarWidget : public Gtk::DrawingArea
{
	public:
		struct Data
		{
			double value;
			double ciLow;
			double ciHigh;
			std::string label;
			Gdk::RGBA color;
		};

	public:
		BarWidget(const Style &style);

		void setData(Data data);
		void setYMax(double yMax);

	private:
		bool on_draw(const::Cairo::RefPtr<Cairo::Context> &context);

		Data m_data;
		double m_yMax;

		const Style &m_style;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
