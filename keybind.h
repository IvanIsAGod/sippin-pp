#pragma once
class MKeybind : public MBaseElement
{
public:
	MKeybind() {}
	MBaseElement* parent;
	int Key;
	bool IsGettingKey;
	MKeybind::MKeybind(MBaseElement* parent, std::string name, std::string caption, MCategory* SaveCategory, string savename);
	virtual void Draw() override;
};


class MKeybind2 : public MBaseElement
{
public:
	MKeybind2() {}
	MBaseElement* parent;
	int Key;
	bool IsGettingKey;
	MKeybind2::MKeybind2(MBaseElement* parent, std::string name, std::string caption, MCategory* SaveCategory, string savename);
	virtual void Draw() override;
};