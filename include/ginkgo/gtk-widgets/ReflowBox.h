#ifndef __GTK_WIDGETS__REFLOW_BOX_H
#define __GTK_WIDGETS__REFLOW_BOX_H

#include <gtkmm.h>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ReflowBox
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ReflowBox : public Gtk::Layout
{
	protected:
		virtual void on_size_allocate(Gtk::Allocation &allocation);
		void rearrange();

		virtual Gtk::SizeRequestMode get_request_mode_vfunc() const;
		virtual void get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
