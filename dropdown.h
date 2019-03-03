#pragma once



class MDropdown : public MBaseElement
{
public:
	MDropdown() {}

	MBaseElement* parent;
	std::vector<std::string> Items;
	int SelectedIndex;
	bool IsOpen;
	MDropdown(MBaseElement* parent, std::string name, std::string caption,  MCategory* SaveCategory, std::string savename, std::vector<string> Items);
	virtual void Draw() override;
	bool holding;
};




