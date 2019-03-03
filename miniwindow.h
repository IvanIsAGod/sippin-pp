#pragma once
class MMiniWindow : public MBaseElement
{
public:
	
	bool bRender;
	bool bDragging;
	MBaseElement* parent;
	MMiniWindow(std::string name, Vector pos, Vector size, MBaseElement* parent, bool render_if_no_menu);
	virtual void Draw() override;
	int iXDif = 0;
	int iYDif = 0;
};