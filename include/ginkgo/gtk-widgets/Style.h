#ifndef __GTK_WIDGETS__STYLE_H
#define __GTK_WIDGETS__STYLE_H

#include <gtkmm.h>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Style
//
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Style
{
	public:
		static const Style styleGTK(Glib::RefPtr<Gtk::StyleContext> styleContext);
		static const Style StylePrintFullWidth;
		static const Style StylePrintHalfWidth;
		static const Style StylePrintLoopFullWidth;
		static const Style StylePrintLoopHalfWidth;

	public:
		bool roundToPixels;
		bool showLabels;
		double lineWidthNormal;
		Gdk::RGBA colorBackground;
		Gdk::RGBA colorPositive;
		Gdk::RGBA colorPositiveLight;
		Gdk::RGBA colorAccent;
		Gdk::RGBA colorGray;
		Gdk::RGBA colorGrid;
		double barChartHeight;
		double barChartYAxisWidth;
		double barChartXAxisHeight;
		double barChartTitleSpacing;
		double barChartGroupSpacing;
		double barChartBarSpacing;
		double barChartBarWidth;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
