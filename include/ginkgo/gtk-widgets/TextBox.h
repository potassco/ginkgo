#ifndef __GTK_WIDGETS__TEXT_BOX_H
#define __GTK_WIDGETS__TEXT_BOX_H

#include <gtkmm.h>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TextBox
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextBox : public Gtk::Bin
{
	public:
		TextBox();

		void setTitle(std::string title);
		void addItem(std::string itemText);
		void clear();

		void selectAll();

		sigc::signal<void> &onSelectedRowsChanged();
		std::vector<Gtk::ListBoxRow *> selectedRows();
		std::vector<Gtk::ListBoxRow *> selectedRows() const;

	private:
		sigc::signal<void> m_onSelectedRowsChanged;

		Gtk::ListBox m_listBox;
		std::vector<std::unique_ptr<Gtk::Label>> m_labels;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
