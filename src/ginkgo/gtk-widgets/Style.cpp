#include <ginkgo/gtk-widgets/Style.h>

#include <iostream>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Style
//
////////////////////////////////////////////////////////////////////////////////////////////////////

const Style Style::styleGTK(Glib::RefPtr<Gtk::StyleContext> styleContext)
{
	const auto colorPositive = styleContext->get_color();
	auto colorPositiveLight = colorPositive;
	colorPositiveLight.set_alpha(0.5);

	return
		{
			true,
			true,
			1.0,
			styleContext->get_background_color(),
			colorPositive,
			colorPositiveLight,
			Gdk::RGBA("#3080e9"),
			Gdk::RGBA("#505050"),
			Gdk::RGBA("#606060"),
			288.0,
			60.0,
			30.0,
			20.0,
			5.0,
			3.0,
			18.0
		};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Style Style::StylePrintFullWidth =
	{
		false,
		true,
		0.6,
		Gdk::RGBA("#ffffff"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#4a90d9"),
		Gdk::RGBA("#a0a29d"),
		Gdk::RGBA("#babdb6"),
		0.440958992 * 288.0,
		0.440958992 * 60.0,
		0.440958992 * 30.0,
		0.440958992 * 20.0,
		0.440958992 * 5.0,
		0.440958992 * 5.0,
		0.440958992 * 25.0
	};

////////////////////////////////////////////////////////////////////////////////////////////////////

const Style Style::StylePrintHalfWidth =
	{
		false,
		false,
		0.4 / 0.54605753425,
		Gdk::RGBA("#ffffff"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#4a90d9"),
		Gdk::RGBA("#a0a29d"),
		Gdk::RGBA("#babdb6"),
		4 * 288.0,
		4 * 60.0,
		4 * 30.0,
		4 * 20.0,
		4 * 5.0,
		4 * 5.0,
		4 * 25.0
	};

////////////////////////////////////////////////////////////////////////////////////////////////////

const Style Style::StylePrintLoopFullWidth =
	{
		false,
		true,
		0.6,
		Gdk::RGBA("#ffffff"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#4a90d9"),
		Gdk::RGBA("#a0a29d"),
		Gdk::RGBA("#babdb6"),
		0.440958992 * 360.0,
		0.440958992 * 60.0,
		0.440958992 * 30.0,
		0.440958992 * 20.0,
		0.440958992 * 5.0,
		0.440958992 * 5.0,
		0.440958992 * 25.0
	};

////////////////////////////////////////////////////////////////////////////////////////////////////

const Style Style::StylePrintLoopHalfWidth =
	{
		false,
		false,
		0.5 * 0.8,
		Gdk::RGBA("#ffffff"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#000000"),
		Gdk::RGBA("#4a90d9"),
		Gdk::RGBA("#a0a29d"),
		Gdk::RGBA("#babdb6"),
		0.4 * 360.0,
		0.4 * 60.0,
		0.4 * 30.0,
		0.4 * 20.0,
		0.4 * 5.0,
		0.4 * 5.0,
		0.4 * 25.0
	};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
