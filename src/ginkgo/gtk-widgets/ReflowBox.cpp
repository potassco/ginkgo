#include <ginkgo/gtk-widgets/ReflowBox.h>

#include <iostream>
#include <numeric>

namespace ginkgo
{
namespace gtkWidgets
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ReflowBox
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ReflowBox::on_size_allocate(Gtk::Allocation &allocation)
{
	Gtk::Layout::on_size_allocate(allocation);
	rearrange();
	Gtk::Layout::on_size_allocate(allocation);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ReflowBox::rearrange()
{
	const auto &children = get_children();

	if (children.empty())
		return;

	// Get maximum child dimensions
	const auto maxChildWidth = std::accumulate(children.begin(), children.end(), 0, [](int maxChildWidth, const Gtk::Widget *widget)
	{
		return std::max(maxChildWidth, widget->get_allocation().get_width());
	});

	const auto &width = get_allocation().get_width();

	const size_t innerMarginX = 40;
	const size_t innerMarginY = 40;

	const size_t columns = std::max<size_t>(1, (width + innerMarginX) / (maxChildWidth + innerMarginX));

	const size_t outerMarginX = (width + innerMarginX - columns * (maxChildWidth + innerMarginX)) / 2;

	int y = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		const auto row = i / columns;
		const auto column = i % columns;

		const int x = outerMarginX + column * (maxChildWidth + innerMarginX);

		move(*children[i], x, y);

		if (column == columns - 1)
		{
			const auto begin = children.begin() + columns * row;
			const auto end = (columns * (row + 1) >= children.size()) ? children.end() : children.begin() + columns * (row + 1);

			const auto maxChildHeight = std::accumulate(begin, end, 0, [](int maxChildHeight, const Gtk::Widget *widget)
			{
				return std::max(maxChildHeight, widget->get_allocation().get_height());
			});

			y += maxChildHeight + innerMarginY;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Gtk::SizeRequestMode ReflowBox::get_request_mode_vfunc() const
{
	return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ReflowBox::get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const
{
	const auto &children = get_children();

	if (children.empty())
	{
		minimum_height = 1;
		natural_height = 1;
		return;
	}

	// Get maximum child dimensions
	const auto maxChildWidth = std::accumulate(children.begin(), children.end(), 0, [](int maxChildWidth, const Gtk::Widget *widget)
	{
		return std::max(maxChildWidth, widget->get_allocation().get_width());
	});

	const size_t innerMarginX = 40;
	const size_t innerMarginY = 40;

	const size_t columns = std::max<size_t>(1, (width + innerMarginX) / (maxChildWidth + innerMarginX));

	int y = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		const auto row = i / columns;
		const auto column = i % columns;

		if (column == columns - 1 || i == children.size() - 1)
		{
			const auto begin = children.begin() + columns * row;
			const auto end = (columns * (row + 1) >= children.size()) ? children.end() : children.begin() + columns * (row + 1);

			const auto maxChildHeight = std::accumulate(begin, end, 0, [](int maxChildHeight, const Gtk::Widget *widget)
			{
				return std::max(maxChildHeight, widget->get_allocation().get_height());
			});

			y += maxChildHeight + innerMarginY;
		}
	}

	minimum_height = y;
	natural_height = y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
