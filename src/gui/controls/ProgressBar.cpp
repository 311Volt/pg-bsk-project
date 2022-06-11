#include "ProgressBar.hpp"

ProgressBar::ProgressBar(al::Vec2<> size, al::Coord<> pos)
	: Window(size, pos),
	fill({0,getHeight()-2}, {1,0})
{
	setBgColor(al::DarkGray);
	setEdgeType(EDGE_BEVELED_INWARD);

	fill.setBgColor(al::Blue);
	fill.setAlignment(ALIGN_LEFT_CENTER);
	fill.setEdgeType(EDGE_NONE);

	addChild(fill);
}

void ProgressBar::setValue(float value)
{
	value = std::max(0.f, std::clamp(value, 0.f, 1.f) * (getWidth() - 2.f));
	fill.resize({value, fill.getHeight()});
}