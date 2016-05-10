#ifndef __GTK_WIDGETS__INFO_WIDGET_H
#define __GTK_WIDGETS__INFO_WIDGET_H

#include <gtkmm.h>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// InfoWidget
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class InfoWidget : public Gtk::Box
{
	public:
		struct Data
		{
			std::string title;
			std::string subtitle;
			Gdk::RGBA color;
		};

	public:
		InfoWidget();

		void setTitle(std::string title);
		void setData(Data data);
		void setTitleSpacing(double titleSpacing);

	private:
		void on_size_allocate(Gtk::Allocation &allocation);

		void rearrange();

		bool drawChart(const::Cairo::RefPtr<Cairo::Context> &context);

		Gtk::Layout m_content;
		Gtk::DrawingArea m_chartDrawingArea;
		Gtk::Label m_title;

		Data m_data;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
