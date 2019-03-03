#include "sdk.h"
#include "hooks.h"
#include "Menu.h"
#include "global.h"
#include "ESP.h"
#include "imgui\imconfig.h"
#include "ImGUI\imgui.h"
#include "ImGUI\imgui_internal.h"
#include "ImGUI\stb_rect_pack.h"
#include "ImGUI\stb_textedit.h"
#include "ImGUI\stb_truetype.h"
#include "ImGUI\dx9\imgui_dx9.h"
#include "Items.h"
#include "event_log.h"
#include "GameUtils.h"
#include "render.h"
#include "BacktrackingHelper.h"
#include <Shlobj.h>
#include <intrin.h>

IDirect3DStateBlock9* pixel_state = NULL; IDirect3DVertexDeclaration9* vertDec; IDirect3DVertexShader9* vertShader;
DWORD dwOld_D3DRS_COLORWRITEENABLE;

void SaveState(IDirect3DDevice9 * pDevice)
{
	pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
	pDevice->GetVertexDeclaration(&vertDec);
	pDevice->GetVertexShader(&vertShader);
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);
}

void RestoreState(IDirect3DDevice9 * pDevice) // not restoring everything. Because its not needed.
{
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
	pDevice->SetVertexDeclaration(vertDec);
	pDevice->SetVertexShader(vertShader);
}

bool ClientVariables::Save(std::string file_name)
{
	CreateDirectory("C:\\SippinPiss", NULL);

	std::string file_path = "C:\\SippinPiss\\" + file_name + ".cfg";

	std::fstream file(file_path, std::ios::out | std::ios::in | std::ios::trunc);
	file.close();

	file.open(file_path, std::ios::out | std::ios::in);
	if (!file.is_open())
	{
		file.close();
		return false;
	}

	const size_t settings_size = sizeof(ClientVariables);
	for (int i = 0; i < settings_size; i++)
	{
		byte current_byte = *reinterpret_cast<byte*>(uintptr_t(this) + i);
		for (int x = 0; x < 8; x++)
		{
			file << (int)((current_byte >> x) & 1);
		}
	}

	file.close();

	return true;
}

bool ClientVariables::Load(std::string file_name)
{
	std::string file_path = "C:\\SippinPiss\\" + file_name + ".cfg";

	std::fstream file;
	file.open(file_path, std::ios::out | std::ios::in);
	if (!file.is_open())
	{
		file.close();
		return false;
	}

	std::string line;
	while (file)
	{
		std::getline(file, line);

		const size_t settings_size = sizeof(ClientVariables);
		if (line.size() > settings_size * 8)
		{
			file.close();
			return false;
		}
		for (int i = 0; i < settings_size; i++)
		{
			byte current_byte = *reinterpret_cast<byte*>(uintptr_t(this) + i);
			for (int x = 0; x < 8; x++)
			{
				if (line[(i * 8) + x] == '1')
					current_byte |= 1 << x;
				else
					current_byte &= ~(1 << x);
			}
			*reinterpret_cast<byte*>(uintptr_t(this) + i) = current_byte;
		}
	}

	file.close();

	return true;
}

void ClientVariables::CreateConfig(std::string name)
{
	CreateDirectory("C:\\SippinPiss\\", NULL); CreateDirectory("C:\\SippinPiss\\", NULL);

	std::ofstream ofs("C:\\SippinPiss\\" + name + ".cfg");

}

char* string_as_array(std::string* str)
{
	return str->empty() ? NULL : &*str->begin();
}

void ClientVariables::Delete(std::string name) {

	std::string l1 = "C:\\SippinPiss\\";
	std::string l2 = ".cfg";
	l1 = l1 + name + l2;
	const char* cfg = string_as_array(&l1);
	const int result = remove(cfg);
}

std::vector<std::string> ClientVariables::GetConfigs()
{
	std::vector<std::string> configs;

	WIN32_FIND_DATA ffd;
	auto directory = "C:\\SippinPiss\\*";
	auto hFind = FindFirstFile(directory, &ffd);

	while (FindNextFile(hFind, &ffd))
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::string file_name = ffd.cFileName;
			if (file_name.size() < 4) // .cfg
				continue;

			std::string end = file_name;
			end.erase(end.begin(), end.end() - 4); // erase everything but the last 4 letters
			if (end != ".cfg")
				continue;

			file_name.erase(file_name.end() - 4, file_name.end()); // erase the .cfg part
			configs.push_back(file_name);
		}
	}

	return configs;
}

LockCursor_t oLockCursor;

void __stdcall Hooks::hk_lockcursor()
{
	if (csgo::ShowMenu)
	{
		g_pSurface->unlockcursor();
		return;
	}

	oLockCursor(g_pSurface);
}


namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	IMGUI_API bool ComboBoxArray(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}
	
	void SelectTabs(int *selected, const char* items[], int item_count, ImVec2 size = ImVec2(0, 0))
	{
		auto color_grayblue = GetColorU32(ImVec4(0.48, 0.48, 0.48, 0.20));
		auto color_deepblue = GetColorU32(ImVec4(0.12, 0.12, 0.12, 0.15));
		auto color_shade_hover = GetColorU32(ImVec4(0.8, 0.8, 0.8, 0.05));
		auto color_shade_clicked = GetColorU32(ImVec4(0.1, 0.1, 0.1, 0.1));
		auto color_black_outlines = GetColorU32(ImVec4(0, 0, 0, 1));

		ImGuiStyle &style = GetStyle();
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		std::string names;
		for (size_t i = 0; i < item_count; i++)
			names += items[i];

		ImGuiContext* g = GImGui;
		const ImGuiID id = window->GetID(names.c_str());
		const ImVec2 label_size = CalcTextSize(names.c_str(), NULL, true);

		ImVec2 Min = window->DC.CursorPos;
		ImVec2 Max = ((size.x <= 0 || size.y <= 0) ? ImVec2(Min.x + GetContentRegionMax().x - style.WindowPadding.x, Min.y + label_size.y * 2) : Min);

		ImRect bb(Min, Max);
		ItemSize(bb, style.FramePadding.y);
		if (!ItemAdd(bb, &id))
			return;

		PushClipRect(ImVec2(Min.x, Min.y - 1), ImVec2(Max.x, Max.y + 1), false);

		window->DrawList->AddRectFilledMultiColor(Min, Max, color_grayblue, color_grayblue, color_deepblue, color_deepblue); // Main gradient.

		ImVec2 mouse_pos = GetMousePos();
		bool mouse_click = g->IO.MouseClicked[0];

		float TabSize = ceil((Max.x - Min.x) / item_count);

		for (size_t i = 0; i < item_count; i++)
		{
			ImVec2 Min_cur_label = ImVec2(Min.x + (int)TabSize * i, Min.y);
			ImVec2 Max_cur_label = ImVec2(Min.x + (int)TabSize * i + (int)TabSize, Max.y);

			// Imprecision clamping. gay but works :^)
			Max_cur_label.x = (Max_cur_label.x >= Max.x ? Max.x : Max_cur_label.x);

			if (mouse_pos.x > Min_cur_label.x && mouse_pos.x < Max_cur_label.x &&
				mouse_pos.y > Min_cur_label.y && mouse_pos.y < Max_cur_label.y)
			{
				if (mouse_click)
					*selected = i;
				else if (i != *selected)
					window->DrawList->AddRectFilled(Min_cur_label, Max_cur_label, color_shade_hover);
			}

			if (i == *selected) {
				window->DrawList->AddRectFilled(Min_cur_label, Max_cur_label, color_shade_clicked);
				window->DrawList->AddRectFilledMultiColor(Min_cur_label, Max_cur_label, color_deepblue, color_deepblue, color_grayblue, color_grayblue);
				window->DrawList->AddLine(ImVec2(Min_cur_label.x - 1.5f, Min_cur_label.y - 1), ImVec2(Max_cur_label.x - 0.5f, Min_cur_label.y - 1), color_black_outlines);
			}
			else
				window->DrawList->AddLine(ImVec2(Min_cur_label.x - 1, Min_cur_label.y), ImVec2(Max_cur_label.x, Min_cur_label.y), color_black_outlines);
			window->DrawList->AddLine(ImVec2(Max_cur_label.x - 1, Max_cur_label.y), ImVec2(Max_cur_label.x - 1, Min_cur_label.y - 0.5f), color_black_outlines);

			const ImVec2 text_size = CalcTextSize(items[i], NULL, true);
			float pad_ = style.FramePadding.x + g->FontSize + style.ItemInnerSpacing.x;
			ImRect tab_rect(Min_cur_label, Max_cur_label);
			RenderTextClipped(Min_cur_label, Max_cur_label, items[i], NULL, &text_size, style.WindowTitleAlign, &tab_rect);
		}

		window->DrawList->AddLine(ImVec2(Min.x, Min.y - 0.5f), ImVec2(Min.x, Max.y), color_black_outlines);
		window->DrawList->AddLine(ImVec2(Min.x, Max.y), Max, color_black_outlines);
		PopClipRect();
	}
}

const char* KeyStrings[] = {
	"",
	"Mouse 1",
	"Mouse 2",
	"Cancel",
	"Middle Mouse",
	"Mouse 4",
	"Mouse 5",
	"",
	"Backspace",
	"Tab",
	"",
	"",
	"Clear",
	"Enter",
	"",
	"",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Caps",
	"",
	"",
	"",
	"",
	"",
	"",
	"Escape",
	"",
	"",
	"",
	"",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"",
	"",
	"",
	"Print",
	"Insert",
	"Delete",
	"",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"",
	"",
	"",
	"",
	"",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Multiply",
	"Add",
	"",
	"Subtract",
	"Decimal",
	"Divide",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",

};

const char* KnifeModel[] =
{
	"Bayonet",
	"Flip Knife",
	"Gut Knife",
	"Karambit",
	"M9 Bayonet",
	"Huntsman Knife",
	"Butterfly Knife",
	"Falchion Knife",
	"Shaddow Daggers",
	"Bowie Knife",
	"Navaja Knife",
	"Stiletto Knife",
	"Ursus Knife",
	"Talon Knife"
};
const char* knifeskins[] =
{
	"None",
	"Crimson Web",
	"Bone Mask",
	"Fade",
	"Night",
	"Blue Steel",
	"Stained",
	"Case Hardened",
	"Slaughter",
	"Safari Mesh",
	"Boreal Forest",
	"Ultraviolet",
	"Urban Masked",
	"Scorched",
	"Rust Coat",
	"Tiger Tooth",
	"Damascus Steel",
	"Damascus Steel",
	"Marble Fade",
	"Rust Coat",
	"Doppler Ruby",
	"Doppler Sapphire",
	"Doppler Blackpearl",
	"Doppler Phase 1",
	"Doppler Phase 2",
	"Doppler Phase 3",
	"Doppler Phase 4",
	"Gamma Doppler Phase 1",
	"Gamma Doppler Phase 2",
	"Gamma Doppler Phase 3",
	"Gamma Doppler Phase 4",
	"Gamma Doppler Emerald",
	"Lore",
	"Black Laminate",
	"Autotronic",
	"Freehand"
};
const char* ak47[] =
{
	"None",
	"Fire Serpent",
	"Fuel Injector",
	"Bloodsport",
	"Vulcan",
	"Case Hardened",
	"Hydroponic",
	"Aquamarine Revenge",
	"Frontside Misty",
	"Point Disarray",
	"Neon Revolution",
	"Red Laminate",
	"Redline",
	"Jaguar",
	"Jet Set",
	"Wasteland Rebel",
	"The Empress",
	"Elite Build",
	"Neon Rider"
};

const char* m4a4[] =
{
	"None",
	"Asiimov",
	"Howl",
	"Dragon King",
	"Poseidon",
	"Daybreak",
	"Royal Paladin",
	"The BattleStar",
	"Desolate Space",
	"Buzz Kill",
	"Bullet Rain",
	"Hell Fire",
	"Evil Daimyo",
	"Griffin",
	"Zirka",
	"Radiation Hazard",
	"Neo-Noir"
};

const char* dual[] =
{
	"None",
	"Panther",
	"Dualing Dragons",
	"Cobra Strike",
	"Royal Consorts",
	"Duelist"
};

const char* revolver[] =
{
	"None",
	"Llama Cannon",
	"Fade",
};

const char* galil[] =
{
	"None",
	"Chatterbox",
	"Crimson Tsunami",
	"Sugar Rush",
	"Eco",
	"Cerberus"
};

const char* mp9[] =
{
	"None",
	"Rose Iron",
	"Ruby Poison Dart",
	"Airlock"
};

const char* bizon[] =
{
	"None",
	"High Roller",
	"Judgement of Anubis",
	"Fuel Rod"
};

const char* sg553[] =
{
	"None",
	"Tiger Moth",
	"Cyrex",
	"Pulse",
	"Phantom"
};

const char* g3sg1[] =
{
	"None",
	"Hunter",
	"The Executioner",
	"Flux"
};

const char* mp7[] =
{
	"None",
	"Nemesis",
	"Impire",
	"Special Delivery"
};

const char* five[] =
{
	"None",
	"Monkey Business",
	"Hyper Beast",
	"Fowl Play",
	"Triumvirate",
	"Retrobution",
	"Capillary",
	"Violent Daimyo",
	"Flame Test"
};

const char* mac10[] =
{
	"None",
	"Neon Rider",
	"Last Dive",
	"Curse",
	"Rangeen"
};

const char* aug[] =
{
	"None",
	"Bengal Tiger",
	"Hot Rod",
	"Chameleon",
	"Akihabara Accept",
	"Stymphalian"
};

const char* sawed[] =
{
	"None",
	"WesteLand Princess",
	"The Kraken",
	"Yorick"
};

const char* mag[] =
{
	"None",
	"Bulldozer",
	"Heat",
	"Petroglyph",
	"Swag-7"
};

const char* xm[] =
{
	"None",
	"Seasons",
	"Tranquility",
	"Ziggy"
};

const char* nova[] =
{
	"None",
	"Hyper Beast",
	"Koi"
};

const char* famas[] =
{
	"None",
	"Djinn",
	"Styx",
	"Neural Net",
	"Survivor"
};

const char* awp[] =
{
	"None",
	"Asiimov",//19
	"Dragon Lore", //1
	"Fever Dream", //0
	"Medusa",//10
	"Hyper Beast",//11
	"BOOM", //0
	"Lightning Strike",//3
	"Pink DDPAT", //2
	"Corticera",//4
	"Redline",//5
	"Man-o'-war",//6
	"Graphite",//7
	"Electric Hive",//18
	"Sun In Leo",
	"Hyper Beast",
	"Pit Viper",
	"Phobos",
	"Elite Build",
	"Worm God",
	"Oni Taiji",
	"Fever Dream",
	"Paw"
};

const char* ssg08[] =
{
	"None",
	"Lichen Dashed",
	"Dark Water",
	"Blue Spruce",
	"Sand Dune",
	"Palm",
	"Mayan Dreams",
	"Blood in the Water",
	"Tropical Storm",
	"Acid Fade",
	"Slashed",
	"Detour",
	"Abyss",
	"Big Iron",
	"Necropos",
	"Ghost Crusader",
	"Dragonfire"
};

const char* negev[] =
{
	"None",
	"Power Loader",
	"Loudmouth",
	"Man-o'-War"
};

const char* m249[] =
{
	"None",
	"Nebula Crusader",
	"System Lock",
	"Magma"
};
const char* m4a1s[] =
{
	"None",
	"Decimator",
	"Knight",
	"Chantico's Fire",
	"Golden Coi",
	"Hyper Beast",
	"Master Piece",
	"Hot Rod",
	"Mecha Industries",
	"Cyrex",
	"Icarus Fell",
	"Flashback",
	"Leaded Glass",
	"Hyper Beast",
	"Atomic Alloy",
	"Guardian",
	"Briefing",
	"Nightmare"
};

const char* cz75[] =
{
	"None",
	"Red Astor",
	"Pole Position",
	"Victoria",
	"Xiangliu"
};

const char* scar20[] =
{
	"None",
	"Blueprint",
	"Cyrex",
	"Emerald",
	"Green Marine",
	"Outbreak",
	"Bloodsport",
	"Jungle Slipstream"
};

const char* p90[] =
{
	"None",
	"Death by Kitty",
	"Fallout Warning",
	"Scorched",
	"Emerald Dragon",
	"Teardown",
	"Blind Spot",
	"Trigon",
	"Desert Warfare",
	"Module",
	"Asiimov",
	"Elite Build",
	"Shapewood",
	"Shallow Grave"
};

const char* ump45[] =
{
	"None",
	"Blaze",
	"Minotaur's Labyrinth",
	"Riot",
	"Primal Saber",
	"Exposure",
	"Arctic Wolf"
};

const char* glock[] =
{
	"None",
	"Fade",
	"Dragon Tattoo",
	"Twilight Galaxy",
	"Wasteland Rebel",
	"Water Elemental",
	"Off World",
	"Weasel",
	"Royal Legion",
	"Grinder",
	"Steel Disruption",
	"Brass",
	"Ironwork",
	"Wraiths",
	"Bunsen Burner",
	"Reactor",
	"Moonrise"
};

const char* usp[] =
{
	"None",
	"Neo-Noir",
	"Cyrex",
	"Orion",
	"Kill Confirmed",
	"Overgrowth",
	"Caiman",
	"Serum",
	"Guardian",
	"Road Rash",
	"Cortex"
};

const char* deagle[] =
{
	"None",
	"Blaze",
	"Kumicho Dragon",
	"Oxide Blaze",
	"Golden Koi",
	"Blaze",
	"Oxide Blaze",
	"Cobalt Disruption",
	"Directive"
};

const char* tec9[] =
{
	"None",
	"Nuclear Threat",
	"Red Quartz",
	"Blue Titanium",
	"Titanium Bit",
	"Sandstorm",
	"Isaac",
	"Toxic",
	"Re-Entry",
	"Fuel Injector"
};

const char* p2000[] =
{
	"None",
	"Handgun",
	"Fade",
	"Corticera",
	"Ocean Foam",
	"Fire Elemental",
	"Asterion",
	"Pathfinder"
};

const char* p250[] =
{
	"None",
	"Whiteout",
	"Crimson Kimono",
	"Mint Kimono",
	"Wing Shot",
	"Asiimov",
	"See Ya Later"
};
const char* gloves[] =
{
	"Off",
	"Bloodhound",
	"Sport",
	"Driver",
	"Hand Wraps",
	"Motorcycle",
	"Hydra",
	"Specialist",
};
const char* gloveskin[] =
{
	"None",
	"1",
	"2",
	"3"
};

static const char* ticks[] = {
	"Last record",
	"All records"
};
static const char* wpncfgchar[] = {
	"Global",
	"Auto",
	"Scout",
	"AWP",
	"Pistols",
	"Desert Eagle",
	"R8 Revolver",
	"Rifles",
	"Machine guns",
	"Submachine guns",
	"Shotguns"
};
static const char* stopmodes[] = {
	"Off",
	"Quick Stop",
	"Slow Walk"
};
static const char* extratype[] = {
	"Off",
	"Default",
	"Test"
};

static const char* Pitch[] =
{
	"Off",
	"Down",
	"Up",
	"Fake down",
	"Fake up",
	"Zero",
	"Fake Zero"
};
static const char* direction[] =
{
	"Off",
	"Manual AA",
	"Freestanding"
};
static const char* aabase[] =
{
	"Local View",
	"At Targets",
};
static const char* betamode[] =
{
	"Jitter",
	"Static",
};
static const char* Hitboxes[] =
{
	"Head",
	"Neck",
	"Chest",
	"Stomach",
	"MultiHitbox"
};

static const char* hitsound[] =
{
	"default",
	"bameware",
	"click",
	"pop up",
	"gamesense",
	"stapler",
	"pop up 2"
};
static const char* hitmarkertype[] =
{
	"cross",
	"triangle",
	"circle"
};

#define FCVAR_CHEAT				(1<<14)

PresentFn oPresent;

tReset oResetScene;

void spread_crosshair(IDirect3DDevice9* pDevice)
{
	if (Menu.Visuals.SpreadCrosshair)
	{
		if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
		{
			int r, g, b, a;
			r = Menu.Colors.SpreadCrosshair[0] * 255;
			g = Menu.Colors.SpreadCrosshair[1] * 255;
			b = Menu.Colors.SpreadCrosshair[2] * 255;
			a = Menu.Colors.SpreadCrosshair[3] * 255;
			if (csgo::LocalPlayer && csgo::MainWeapon && csgo::MainWeapon->IsValid())
			{
				float radius = csgo::spread;
				if (csgo::LocalPlayer->GetHealth() > 0 && csgo::LocalPlayer->isAlive())
				{
					if (radius > 0)
						g_pRender->CircleFilledDualColor(csgo::Screen.width / 2, csgo::Screen.height / 2, radius, 0, full, 100, D3DCOLOR_ARGB(a, r, g, b), D3DCOLOR_ARGB(255, 255, 255, 255), pDevice);
				}
			}
		}
	}
}
ImFont* iconfont;
ImFont* menufont;

void GUI_Init(IDirect3DDevice9* pDevice)
{
	ImGui_ImplDX9_Init(csgo::Window, pDevice);

	ImGuiStyle        _style = ImGui::GetStyle();

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\comic.ttf", 15.f);
	iconfont = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 40.f);
	menufont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\comic.ttf", 15.f);
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.71f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.06f, 0.06f, 0.06f, 0.01f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.71f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.07f, 0.48f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 0.48f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.07f, 0.48f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.66f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.37f, 0.00f, 0.12f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.00f, 0.27f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.00f, 0.23f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.00f, 0.23f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(1.00f, 0.00f, 0.30f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.00f, 0.30f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.00f, 0.33f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(255, 255, 255, 255);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(255, 255, 255, 255);
	colors[ImGuiCol_SeparatorActive] = ImVec4(255, 255, 255, 255);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.00f, 0.30f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	_style.WindowPadding = ImVec2(8, 8);      // Padding within a window
	_style.WindowMinSize = ImVec2(32, 32);    // Minimum window size
	_style.WindowRounding = 9.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
	_style.WindowTitleAlign = ImVec2(5.f, 0.5f);// Alignment for title bar text
	_style.FramePadding = ImVec2(4, 3);      // Padding within a framed rectangle (used by most widgets)
	_style.FrameRounding = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
	_style.ItemSpacing = ImVec2(0, 4);      // Horizontal and vertical spacing between widgets/lines
	_style.ItemInnerSpacing = ImVec2(4, 4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
	_style.TouchExtraPadding = ImVec2(0, 0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
	_style.IndentSpacing = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
	_style.ColumnsMinSpacing = 6.0f;             // Minimum horizontal spacing between two columns
	_style.ScrollbarSize = 8.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
	_style.ScrollbarRounding = 9.0f;             // Radius of grab corners rounding for scrollbar
	_style.GrabMinSize = 10.0f;            // Minimum width/height of a grab box for slider/scrollbar
	_style.GrabRounding = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
	_style.ButtonTextAlign = ImVec2(0.5f, 0.5f);// Alignment of button text when button is larger than text.
	_style.DisplayWindowPadding = ImVec2(22, 22);    // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
	_style.DisplaySafeAreaPadding = ImVec2(4, 4);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
	_style.AntiAliasedLines = true;             // Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
	_style.CurveTessellationTol = 1.25f;            // Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
	csgo::Init = true;
}

static int iTab = 0;
void color()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
	style.Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
	style.Colors[ImGuiCol_Border] = ImColor(15, 15, 15, 255);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.60f, 0.78f, 0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
}

void styled()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.f;
	style.FramePadding = ImVec2(4, 0);
	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing = ImVec2(0, 0);
	style.ScrollbarSize = 10.f;
	style.ScrollbarRounding = 0.f;
	style.GrabMinSize = 5.f;
}

void ButtonColor(int r, int g, int b)
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Button] = ImColor(r, g, b);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(r, g, b);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(r, g, b);
}

namespace CPlayerList
{
	std::vector<int> Players;
}


struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};


template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(FindPatternIDA(("client_panorama.dll"), ("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(FindPatternIDA(("client_panorama.dll"), ("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));
	return (T*)find_hud_element(pThis, name);

}

void FullUpdate()
{
	static auto clear_hud_weapon_icon_fn = reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(FindPatternIDA(("client_panorama.dll"), ("55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C")));
	auto element = FindHudElement<std::uintptr_t*>(("CCSGO_HudWeaponSelection"));

	auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xA0);
	if (hud_weapons == nullptr)
		return;

	if (!*hud_weapons->get_weapon_count())
		return;

	for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
		i = clear_hud_weapon_icon_fn(hud_weapons, i);

	g_pEngine->ForceFullUpdate();
}

void draw_menu()
{
	if (g_pEngine->IsInGame() && g_pEngine->IsConnected())
		ImGui::GetIO().MouseDrawCursor = Menu.Gui.Opened;
	else
		ImGui::GetIO().MouseDrawCursor = true;

	auto& style = ImGui::GetStyle();
	ImVec2 pos;

	ImGui_ImplDX9_NewFrame();

	if (Menu.Gui.Opened)
		csgo::ShowMenu = true;
	else
		csgo::ShowMenu = false;

	if (csgo::ShowMenu)
	{
		ImGui::SetNextWindowSize(ImVec2(750, 660));
		ImGui::Begin("Sippin Piss", &Menu.Gui.Opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			ImGui::SetColorEditOptions(ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs | ImGuiWindowFlags_ShowBorders | ImGuiColorEditFlags_AlphaBar);
			pos = ImGui::GetWindowPos();

			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnOffset(1, 135);

			ButtonColor(15, 15, 15);
			ImGui::Button("##upprtabs", ImVec2(114, 20));

			ImGui::SameLine();
			styled();
			color();
			ButtonColor(50, 50, 50);
			ImGui::Button("##rageupline", ImVec2(1, 20));
			ImGui::PushFont(iconfont);
			/*rage*/
			{
				if (iTab == 0) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##rageupline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));

				ButtonColor(15, 15, 15);
				if (iTab == 0) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
				if (ImGui::Button("A", ImVec2(118, 100))) iTab = 0;

				ImGui::SameLine();

				if (iTab != 0)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

				if (iTab == 0) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##ragedownline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));

			}

			/*legit*/
			{
				if (iTab == 1) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##lupline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));

				ButtonColor(15, 15, 15);
				if (iTab == 1) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
				if (ImGui::Button("B", ImVec2(118, 100))) iTab = 1;

				ImGui::SameLine();

				if (iTab != 1)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

				if (iTab == 1) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##ldownline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));
			}

			/*visuals*/
			{
				if (iTab == 2) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##vupline", ImVec2(118, 1));
				 
				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));

				ButtonColor(15, 15, 15);
				if (iTab == 2) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
				if (ImGui::Button("C", ImVec2(118, 100))) iTab = 2;
				ImGui::SameLine();

				if (iTab != 2)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

				if (iTab == 2) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##vdownline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));
			}

			/*misc*/
			{
				if (iTab == 3) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##mupline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));

				ButtonColor(15, 15, 15);
				if (iTab == 3) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
				if (ImGui::Button("W", ImVec2(118, 100))) iTab = 3;
				ImGui::SameLine();

				if (iTab != 3)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

				if (iTab == 3) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##mdownline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));
			}

			/*skins*/
			{
				if (iTab == 4) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##supline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));

				ButtonColor(15, 15, 15);
				if (iTab == 4) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
				if (ImGui::Button("V", ImVec2(118, 100))) iTab = 4;
				ImGui::SameLine();

				if (iTab != 4)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

				if (iTab == 4) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##sdownline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));
			}

			{
				if (iTab == 5) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##supline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));

				ButtonColor(15, 15, 15);
				if (iTab == 5) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
				if (ImGui::Button("P", ImVec2(118, 100))) iTab = 5;
				ImGui::SameLine();

				if (iTab != 5)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

				if (iTab == 5) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
				ImGui::Button("##sdownline", ImVec2(118, 1));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##fgfgfg", ImVec2(1, 1));
			}
			ImGui::PopFont();

			ButtonColor(15, 15, 15);
			ImGui::Button("##upprtabs", ImVec2(118, 20));

			ImGui::SameLine();

			ButtonColor(50, 50, 50);
			ImGui::Button("##rageupline", ImVec2(1, 20));

			ImGui::NextColumn();
			style.WindowPadding = ImVec2(8, 8);
			style.ItemSpacing = ImVec2(4, 4);
			style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
			style.Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 255);
			style.Colors[ImGuiCol_ChildWindowBg] = ImColor(15, 15, 15, 255);
			style.Colors[ImGuiCol_Border] = ImColor(255, 255, 255, 255);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.60f, 0.78f, 0.78f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
			style.Colors[ImGuiCol_CloseButton] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
			style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
			ImGui::BeginChild("main", ImVec2(800, 800));
			{
				if (iTab == 0)
				{
					ImGui::BeginChild("Rage", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Enable ragebot", &Menu.Ragebot.EnableAimbot);
						ImGui::Text("FOV");
						ImGui::SliderInt("###Fov", &Menu.Ragebot.Fov, 0, 180);
						ImGui::Text("Target Selection");
						ImGui::Combo("###Selection", &Menu.Ragebot.AimbotSelection, SelectionMode, ARRAYSIZE(SelectionMode));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Prioritize target based on: ");
						ImGui::Checkbox("Silent", &Menu.Ragebot.SilentAimbot);
						ImGui::Checkbox("No Recoil", &Menu.Ragebot.NoRecoil);
						if (!Menu.Misc.AntiUT) {
							ImGui::Checkbox("No Spread", &Menu.Ragebot.NoSpread);
						}
						ImGui::Checkbox("Auto Revolver (!)", &Menu.Ragebot.AutomaticRevolver);
						ImGui::Checkbox("Revolver Delay", &Menu.Ragebot.NewAutomaticRevolver);
						if (Menu.Ragebot.NewAutomaticRevolver)
							ImGui::SliderInt("Delay Amount", &Menu.Ragebot.NewAutomaticRevolverFactor, 5, 20, "%.0f");
						ImGui::Checkbox("Auto Fire", &Menu.Ragebot.AutomaticFire);
						ImGui::Checkbox("Auto Scope", &Menu.Ragebot.AutomaticScope);
						ImGui::Checkbox("Auto Wall", &Menu.Ragebot.Autowall);						
					}
					ImGui::EndChild();
					ImGui::SameLine();
					ImGui::BeginChild("Rage 2", ImVec2(300, 320), true);
					{
						ImGui::Combo("WPN Config", &Menu.Ragebot.wpncfg, wpncfgchar, ARRAYSIZE(wpncfgchar));
						ImGui::Separator();
						switch (Menu.Ragebot.wpncfg) {
						case 0: 
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Global.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Global.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Global.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Global.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Global.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Global.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Global.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Global.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Global.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Global.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Global.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Global.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Global.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Global.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Global.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Global.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Global.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Global.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Global.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Global.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Global.baimafterx, 0, 10, 0);
							break;
						case 1:
							ImGui::Checkbox("Auto Config", &Menu.Ragebot.Auto.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Auto.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Auto.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Auto.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Auto.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Auto.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Auto.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Auto.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Auto.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Auto.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Auto.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Auto.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Auto.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Auto.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Auto.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Auto.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Auto.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Auto.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Auto.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Auto.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Auto.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Auto.baimafterx, 0, 10, 0);
							break;
						case 2:
							ImGui::Checkbox("Scout Config", &Menu.Ragebot.Scout.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Scout.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Scout.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Scout.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Scout.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Scout.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Scout.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Scout.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Scout.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Scout.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Scout.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Scout.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Scout.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Scout.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Scout.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Scout.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Scout.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Scout.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Scout.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Scout.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Scout.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Scout.baimafterx, 0, 10, 0);
							break;
						case 3:
							ImGui::Checkbox("AWP Config", &Menu.Ragebot.AWP.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.AWP.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.AWP.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.AWP.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.AWP.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.AWP.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.AWP.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.AWP.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.AWP.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.AWP.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.AWP.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.AWP.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.AWP.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.AWP.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.AWP.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.AWP.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.AWP.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.AWP.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.AWP.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.AWP.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.AWP.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.AWP.baimafterx, 0, 10, 0);
							break;
						case 4:
							ImGui::Checkbox("Pistols Config", &Menu.Ragebot.Pistols.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Pistols.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Pistols.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Pistols.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Pistols.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Pistols.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Pistols.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Pistols.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Pistols.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Pistols.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Pistols.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Pistols.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Pistols.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Pistols.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Pistols.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Pistols.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Pistols.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Pistols.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Pistols.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Pistols.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Pistols.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Pistols.baimafterx, 0, 10, 0);
							break;
						case 5:
							ImGui::Checkbox("Deagle Config", &Menu.Ragebot.Deagle.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Deagle.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Deagle.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Deagle.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Deagle.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Deagle.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Deagle.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Deagle.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Deagle.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Deagle.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Deagle.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Deagle.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Deagle.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Deagle.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Deagle.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Deagle.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Deagle.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Deagle.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Deagle.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Deagle.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Deagle.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Deagle.baimafterx, 0, 10, 0);
							break;
						case 6:
							ImGui::Checkbox("Revolver Config", &Menu.Ragebot.Revolver.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Revolver.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Revolver.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Revolver.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Revolver.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Revolver.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Revolver.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Revolver.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Revolver.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Revolver.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Revolver.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Revolver.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Revolver.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Revolver.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Revolver.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Revolver.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Revolver.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Revolver.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Revolver.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Revolver.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Revolver.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Revolver.baimafterx, 0, 10, 0);
							break;
						case 7:
							ImGui::Checkbox("Rifles Config", &Menu.Ragebot.Rifles.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Rifles.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Rifles.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Rifles.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Rifles.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Rifles.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Rifles.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Rifles.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Rifles.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Rifles.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Rifles.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Rifles.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Rifles.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Rifles.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Rifles.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Rifles.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Rifles.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Rifles.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Rifles.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Rifles.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Rifles.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Rifles.baimafterx, 0, 10, 0);
							break;
						case 8:
							ImGui::Checkbox("Machine Gun Config", &Menu.Ragebot.MG.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.MG.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.MG.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.MG.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.MG.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.MG.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.MG.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.MG.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.MG.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.MG.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.MG.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.MG.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.MG.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.MG.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.MG.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.MG.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.MG.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.MG.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.MG.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.MG.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.MG.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Global.baimafterx, 0, 10, 0);
							break;
						case 9:
							ImGui::Checkbox("SMG Config", &Menu.Ragebot.SMG.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.SMG.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.SMG.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.SMG.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.SMG.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.SMG.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.SMG.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.SMG.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.SMG.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.SMG.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.SMG.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.SMG.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.SMG.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.SMG.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.SMG.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.SMG.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.SMG.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.SMG.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.SMG.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.SMG.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.SMG.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.SMG.baimafterx, 0, 10, 0);
							break;
						case 10:
							ImGui::Checkbox("Shotgun Config", &Menu.Ragebot.Shotgun.enable);
							ImGui::Separator();
							ImGui::Text("Auto Stop");
							ImGui::Combo("##Auto Stop", &Menu.Ragebot.Shotgun.AutomaticStop, stopmodes, ARRAYSIZE(stopmodes));
							ImGui::Separator();
							ImGui::SliderFloat("Hitchance", &Menu.Ragebot.Shotgun.Minhitchance, 0, 100, "%.0f%%");
							ImGui::SliderFloat("Min damage", &Menu.Ragebot.Shotgun.Mindamage, 0, 100, "%.0f%%");
							ImGui::Separator();
							//Called within GameUtils Mutlibox
							ImGui::Text("Head Scale");
							ImGui::SliderFloat("###HeadScale", &Menu.Ragebot.Shotgun.headscale, 0.f, 100.f, "%.2f%%");
							ImGui::Text("Body Scale");
							ImGui::SliderFloat("###BodyScale", &Menu.Ragebot.Shotgun.bodyscale, 0.f, 100.f, "%.2f%%");
							ImGui::Separator();
							ImGui::Checkbox("Backtrack", &Menu.Ragebot.Shotgun.PositionAdjustment);
							ImGui::Combo("Type", &Menu.Ragebot.Shotgun.TickType, ticks, ARRAYSIZE(ticks));
							ImGui::Combo("Extrapolation", &Menu.Ragebot.Shotgun.extrapolation, extratype, ARRAYSIZE(extratype));
							ImGui::Checkbox("Delay Shot", &Menu.Ragebot.Shotgun.delayshot);
							ImGui::Separator();
							ImGui::Text("Priority Hitbox");
							ImGui::Combo("###Priority Hitbox", &Menu.Ragebot.Shotgun.Fakehitbox, HitboxMode, ARRAYSIZE(HitboxMode));
							ImGui::Separator();
							ImGui::Text("Hitscans");
							ImGui::Separator();
							ImGui::Selectable("Head", &Menu.Ragebot.Shotgun.headhitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Chest", &Menu.Ragebot.Shotgun.chesthitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Stomach", &Menu.Ragebot.Shotgun.stomachhitscan, 0, ImVec2(125, 15));
							ImGui::SameLine();
							ImGui::Selectable("Arms", &Menu.Ragebot.Shotgun.armshitscan, 0, ImVec2(120, 15));
							ImGui::Selectable("Legs", &Menu.Ragebot.Shotgun.legshitscan, 0, ImVec2(120, 15));
							ImGui::SameLine();
							ImGui::Selectable("Feet", &Menu.Ragebot.Shotgun.feethitscan, 0, ImVec2(120, 15));
							ImGui::Separator();
							ImGui::Text("Body Aim");
							ImGui::Separator();
							ImGui::Selectable("if Lethal", &Menu.Ragebot.Shotgun.baimiflethal, 0, ImVec2(240, 15));
							ImGui::Selectable("if Air", &Menu.Ragebot.Shotgun.baimifair, 0, ImVec2(240, 15));
							ImGui::Selectable("if Slow Mo", &Menu.Ragebot.Shotgun.baimifslow, 0, ImVec2(240, 15));
							ImGui::Selectable("if Moving", &Menu.Ragebot.Shotgun.baimifmove, 0, ImVec2(240, 15));
							ImGui::Text("After x shots");
							ImGui::SliderInt("###Baim After X", &Menu.Ragebot.Shotgun.baimafterx, 0, 10, 0);
							break;
						default:
							Menu.Ragebot.wpncfg = 0;
							break;

						}
					}
					ImGui::EndChild();
					ImGui::BeginChild("Rage 3", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Resolver (!)", &Menu.Ragebot.AutomaticResolver);
						ImGui::Text("Resolver Type");
						ImGui::Combo("##resolvetype", &Menu.Ragebot.ResolverType, ResolverType, ARRAYSIZE(ResolverType));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Default: Desync | Experimental: LBY");
						ImGui::Text("Resolver Override Key");
						ImGui::Combo("##Override", &Menu.Ragebot.ResolverOverride, KeyStrings, ARRAYSIZE(KeyStrings));
						ImGui::Text("Force Body Aim Key");
						ImGui::Combo("##Force Pelvis", &Menu.Ragebot.ForcePelvis, KeyStrings, ARRAYSIZE(KeyStrings));
						ImGui::Checkbox("Fake Duck", &Menu.Antiaim.FakeDuck);
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Under Development");
						ImGui::Combo("Key###fakeduck", &Menu.Antiaim.FakeDuckKey, KeyStrings, ARRAYSIZE(KeyStrings));
					}
					ImGui::EndChild();
					ImGui::SameLine();
					ImGui::BeginChild("Rage 4", ImVec2(300, 320), true);
					{
						static int AAPage = 0;
						ImGui::Checkbox("Enable###_anti_aim", &Menu.Antiaim.AntiaimEnable);
						ImGui::Combo("AA Base", &Menu.Antiaim.aatype, aabase, ARRAYSIZE(aabase));
						ImGui::Separator();
						if (ImGui::Button("Stand", ImVec2(68, 30))) AAPage = 0;
						ImGui::SameLine();
						if (ImGui::Button("Move", ImVec2(68, 30))) AAPage = 1;
						ImGui::SameLine();
						if (ImGui::Button("Air", ImVec2(68, 30))) AAPage = 2;
						ImGui::SameLine();
						if (ImGui::Button("Slow Walk", ImVec2(68, 30))) AAPage = 3;
						ImGui::Separator();
						if (AAPage == 0)
						{
							ImGui::Text("Pitch");
							ImGui::Combo("###pitch (Stand)", &Menu.Antiaim.Stand.pitch, Pitch, ARRAYSIZE(Pitch));
							ImGui::Text("AA Mode");
							ImGui::Combo("##Desync_Mode", &Menu.Antiaim.Stand.betamode, betamode, ARRAYSIZE(betamode));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("! Static is under development along with fakelag");
							ImGui::SliderFloat("Yaw Add", &Menu.Antiaim.Stand.realyaw, -180.f, 180.f);
							ImGui::Text("AA Direction");
							ImGui::Combo("##DirectionType.Stand", &Menu.Antiaim.Stand.DirectionType, direction, ARRAYSIZE(direction));
							if (Menu.Antiaim.Stand.DirectionType == 1) {
								ImGui::SliderFloat("Switch Angle", &Menu.Antiaim.Stand.switchangle, 0.f, 180.f);
								ImGui::Text("Switch Key");
								ImGui::Combo("###switchkey.Stand", &Menu.Antiaim.switchkey, KeyStrings, ARRAYSIZE(KeyStrings));
							}
							ImGui::Separator();
							ImGui::Combo("FakeLag###fake_lag_mode_stand", &Menu.Antiaim.Stand.FakelagMode, FakelagMode, ARRAYSIZE(FakelagMode));
							ImGui::SliderInt("Factor ##amount_fakelag_stand", &Menu.Antiaim.Stand.FakelagAmount, 1, 14);
							ImGui::SliderInt("Tick Variance##amount_fakelag_stand", &Menu.Antiaim.Air.FakelagVariance, 0, 14);
							ImGui::Checkbox("Fake Lag while Shooting", &Menu.Antiaim.Stand.FakelagShoot);
						}	
						else if (AAPage == 1)
						{
							ImGui::Text("Pitch");
							ImGui::Combo("###pitch (Move)", &Menu.Antiaim.Move.pitch, Pitch, ARRAYSIZE(Pitch));
							ImGui::Text("AA Mode");
							ImGui::Combo("##Desync_Mode", &Menu.Antiaim.Move.betamode, betamode, ARRAYSIZE(betamode));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("! Static is under development along with fakelag");
							ImGui::SliderFloat("Yaw Add", &Menu.Antiaim.Move.realyaw, -180.f, 180.f);
							ImGui::Text("AA Direction");
							ImGui::Combo("##DirectionType.Move", &Menu.Antiaim.Move.DirectionType, direction, ARRAYSIZE(direction));
							if (Menu.Antiaim.Move.DirectionType == 1) {
								ImGui::SliderFloat("Switch Angle", &Menu.Antiaim.Move.switchangle, 0.f, 180.f);
								ImGui::Text("Switch Key");
								ImGui::Combo("###switchkey.Stand", &Menu.Antiaim.switchkey, KeyStrings, ARRAYSIZE(KeyStrings));
							}
							ImGui::Separator();
							ImGui::Combo("FakeLag###fake_lag_mode_move", &Menu.Antiaim.Move.FakelagMode, FakelagMode, ARRAYSIZE(FakelagMode));
							ImGui::SliderInt("Factor ##amount_fakelag_Move", &Menu.Antiaim.Move.FakelagAmount, 1, 14);
							ImGui::SliderInt("Tick Variance ##amount_fakelag_move", &Menu.Antiaim.Move.FakelagVariance, 0, 14);
							ImGui::Checkbox("Fake Lag while Shooting", &Menu.Antiaim.Move.FakelagShoot);
						}
						else if (AAPage == 2)
						{
							ImGui::Text("Pitch");
							ImGui::Combo("###pitch (Air)", &Menu.Antiaim.Air.pitch, Pitch, ARRAYSIZE(Pitch));
							ImGui::Text("AA Mode");
							ImGui::Combo("##Desync_Mode", &Menu.Antiaim.Air.betamode, betamode, ARRAYSIZE(betamode));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("! Static is under development along with fakelag");
							ImGui::SliderFloat("Yaw Add", &Menu.Antiaim.Air.realyaw, -180.f, 180.f);
							ImGui::Text("AA Direction");
							ImGui::Combo("##DirectionType.Air", &Menu.Antiaim.Air.DirectionType, direction, ARRAYSIZE(direction));
							if (Menu.Antiaim.Air.DirectionType == 1) {
								ImGui::SliderFloat("Switch Angle", &Menu.Antiaim.Air.switchangle, 0.f, 180.f);
								ImGui::Text("Switch Key");
								ImGui::Combo("###switchkey.Stand", &Menu.Antiaim.switchkey, KeyStrings, ARRAYSIZE(KeyStrings));
							}
							ImGui::Separator();
							ImGui::Combo("FakeLag###fake_lag_mode_air", &Menu.Antiaim.Air.FakelagMode, FakelagMode, ARRAYSIZE(FakelagMode));
							ImGui::SliderInt("Factor ##amount_fakelag_Air", &Menu.Antiaim.Air.FakelagAmount, 1, 14);
							ImGui::SliderInt("Tick Variance ##amount_fakelag_air", &Menu.Antiaim.Air.FakelagVariance, 0, 14);
							ImGui::Checkbox("Fake Lag while Shooting", &Menu.Antiaim.Air.FakelagShoot);
						}
						else if (AAPage == 3)
						{
							ImGui::Text("Pitch");
							ImGui::Combo("###pitch (Slow)", &Menu.Antiaim.Slow.pitch, Pitch, ARRAYSIZE(Pitch));
							ImGui::Text("AA Mode");
							ImGui::Combo("##Desync_Mode", &Menu.Antiaim.Slow.betamode, betamode, ARRAYSIZE(betamode));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("! Static is under development along with fakelag");
							ImGui::SliderFloat("Yaw Add", &Menu.Antiaim.Slow.realyaw, -180.f, 180.f);
							ImGui::Text("AA Direction");
							ImGui::Combo("##DirectionType.Slow", &Menu.Antiaim.Slow.DirectionType, direction, ARRAYSIZE(direction));
							if (Menu.Antiaim.Slow.DirectionType == 1) {
								ImGui::SliderFloat("Switch Angle", &Menu.Antiaim.Slow.switchangle, 0.f, 180.f);
								ImGui::Text("Switch Key");
								ImGui::Combo("###switchkey.Stand", &Menu.Antiaim.switchkey, KeyStrings, ARRAYSIZE(KeyStrings));
							}
							ImGui::Separator();
							ImGui::Combo("FakeLag###fake_lag_mode_stand", &Menu.Antiaim.Slow.FakelagMode, FakelagMode, ARRAYSIZE(FakelagMode));
							ImGui::SliderInt("Factor ##amount_fakelag_Slow", &Menu.Antiaim.Slow.FakelagAmount, 1, 14);
							ImGui::SliderInt("Tick Variance ##amount_fakelag_Air", &Menu.Antiaim.Slow.FakelagVariance, 0, 14);
							ImGui::Checkbox("Fake Lag while Shooting", &Menu.Antiaim.Slow.FakelagShoot);
						}
					} ImGui::EndChild();
				}
				if (iTab == 1)
				{
					ImGui::BeginChild("Rifles", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Enable", &Menu.LegitBot.bEnable);
						ImGui::Checkbox("Auto Fire", &Menu.LegitBot.autofire);
						ImGui::Checkbox("Friendly Fire", &Menu.LegitBot.friendlyfire);
						ImGui::Checkbox("Smoke Check", &Menu.LegitBot.aimsmokecheck);
						ImGui::Text("Aim");
						ImGui::Combo("###aimKey", &Menu.LegitBot.aimkey, KeyStrings, IM_ARRAYSIZE(KeyStrings));
					
					}
					ImGui::EndChild();
					ImGui::SameLine();

					ImGui::BeginChild("Legit 2", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Triggerbot", &Menu.LegitBot.triggerboton);
						ImGui::Text("Trigger Key");
						ImGui::Combo("###triggerKey", &Menu.LegitBot.triggerbotkey, KeyStrings, IM_ARRAYSIZE(KeyStrings));
						ImGui::Checkbox("Triggerbot RCS", &Menu.LegitBot.triggerrcs);
						ImGui::Checkbox("Triggerbot Team", &Menu.LegitBot.triggerteam);
						ImGui::Checkbox("Trigger Smoke Check", &Menu.LegitBot.triggersmokecheck);

						ImGui::Text("Trigger Hitboxes");
						ImGui::Separator();
						ImGui::Selectable("Head", &Menu.LegitBot.headhitscan, 0, ImVec2(240, 15));
						ImGui::Selectable("Chest", &Menu.LegitBot.chesthitscan, 0, ImVec2(120, 15));
						ImGui::SameLine();
						ImGui::Selectable("Stomach", &Menu.LegitBot.stomachhitscan, 0, ImVec2(125, 15));
						ImGui::Selectable("Arms", &Menu.LegitBot.armshitscan, 0, ImVec2(120, 15));
						ImGui::SameLine();
						ImGui::Selectable("Legs", &Menu.LegitBot.legshitscan, 0, ImVec2(120, 15));
					}
					ImGui::EndChild();
					ImGui::BeginChild("Legit 3", ImVec2(300, 320), true);
					{
						ImGui::Text("Default / Rifles");
						ImGui::Separator();
						ImGui::Checkbox("pSilent##def", &Menu.LegitBot.def_pSilent);
						ImGui::Text("Hitbox");
						ImGui::Combo("###def_hitboxes", &Menu.LegitBot.def_hitbox, Hitboxes, IM_ARRAYSIZE(Hitboxes));
						ImGui::Text("Aim Speed");
						ImGui::SliderFloat("###def_aimspeed", &Menu.LegitBot.def_aimspeed, 0.f, 100.f);
						ImGui::Text("Fov");
						ImGui::SliderFloat("###def_fov", &Menu.LegitBot.def_fov, 0.f, 30.f);
						ImGui::Text("RCS");
						ImGui::SliderFloat("###def_rcs", &Menu.LegitBot.def_rcs, 0.f, 2.f);
						ImGui::Separator();
						ImGui::Text("Sniper");
						ImGui::Separator();
						ImGui::Checkbox("pSilent##sni", &Menu.LegitBot.sni_pSilent);
						ImGui::Text("Hitbox");
						ImGui::Combo("###sni_hitboxes", &Menu.LegitBot.sni_hitbox, Hitboxes, IM_ARRAYSIZE(Hitboxes));
						ImGui::Text("Aim Speed");
						ImGui::SliderFloat("###sni_aimspeed", &Menu.LegitBot.sni_aimspeed, 0.f, 100.f);
						ImGui::Text("Fov");
						ImGui::SliderFloat("###sni_fov", &Menu.LegitBot.sni_fov, 0.f, 30.f);
						ImGui::Text("RCS");
						ImGui::SliderFloat("###sni_rcs", &Menu.LegitBot.sni_rcs, 0.f, 2.f);
					}
					ImGui::EndChild();
					ImGui::SameLine();
					ImGui::BeginChild("Legit 4", ImVec2(300, 320), true);
					{
						ImGui::Text("Pistol");
						ImGui::Separator();
						ImGui::Checkbox("pSilent##pis", &Menu.LegitBot.pis_pSilent);
						ImGui::Text("Hitbox");
						ImGui::Combo("###pis_hitboxes", &Menu.LegitBot.pis_hitbox, Hitboxes, IM_ARRAYSIZE(Hitboxes));
						ImGui::Text("Aim Speed");
						ImGui::SliderFloat("###pis_aimspeed", &Menu.LegitBot.pis_aimspeed, 0.f, 100.f);
						ImGui::Text("Fov");
						ImGui::SliderFloat("###pis_fov", &Menu.LegitBot.pis_fov, 0.f, 30.f);
						ImGui::Text("RCS");
						ImGui::SliderFloat("###pis_rcs", &Menu.LegitBot.pis_rcs, 0.f, 2.f);
						ImGui::Separator();
						ImGui::Text("SMG");
						ImGui::Separator();
						ImGui::Checkbox("pSilent##smg", &Menu.LegitBot.smg_pSilent);
						ImGui::Text("Hitbox");
						ImGui::Combo("###smg_hitboxes", &Menu.LegitBot.smg_hitbox, Hitboxes, IM_ARRAYSIZE(Hitboxes));
						ImGui::Text("Aim Speed");
						ImGui::SliderFloat("###smg_aimspeed", &Menu.LegitBot.smg_aimspeed, 0.f, 100.f);
						ImGui::Text("Fov");
						ImGui::SliderFloat("###smg_fov", &Menu.LegitBot.smg_fov, 0.f, 30.f);
						ImGui::Text("RCS");
						ImGui::SliderFloat("###smg_rcs", &Menu.LegitBot.smg_rcs, 0.f, 2.f);
					}
					ImGui::EndChild();
				}
				else if (iTab == 2)
				{
					ImGui::BeginChild("Visual", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Enable###esp_active", &Menu.Visuals.EspEnable);
						ImGui::Checkbox("Box", &Menu.Visuals.BoundingBox);
						ImGui::SameLine();
						ImGui::ColorEdit3("###boxcolor", Menu.Colors.BoundingBox);
						ImGui::Checkbox("Name", &Menu.Visuals.Name);
						ImGui::SameLine();
						ImGui::ColorEdit3("###namecc", Menu.Colors.Name);
						ImGui::Checkbox("Health", &Menu.Visuals.Health);
						ImGui::Checkbox("Weapon", &Menu.Visuals.Weapon);
						ImGui::SameLine();
						ImGui::ColorEdit3("###weaponc", Menu.Colors.wpn);
						ImGui::Checkbox("Ammo", &Menu.Visuals.Ammo);
						ImGui::SameLine();
						ImGui::ColorEdit3("###ammoclr", Menu.Colors.ammo);
						ImGui::Checkbox("Flags", &Menu.Visuals.Flags);
						if (Menu.Visuals.Flags) 
						{
							ImGui::Selectable("Armor", &Menu.Visuals.Armor);
							ImGui::Selectable("Fake Duck", &Menu.Visuals.FakeDuck);
							ImGui::Selectable("Scoped", &Menu.Visuals.Scoped);
						}
						ImGui::Checkbox("Glow", &Menu.Visuals.Glow);
						ImGui::SameLine();
						ImGui::ColorEdit4("###glowclr ", Menu.Colors.Glow, 1 << 7);
						ImGui::Checkbox("Draw Impacts", &Menu.Visuals.drawimpacts);
						ImGui::Checkbox("Hitmarker", &Menu.Visuals.Hitmarker);
						ImGui::Combo("Hit Shape", &Menu.Visuals.hitmarkertype, hitmarkertype, IM_ARRAYSIZE(hitmarkertype));
						ImGui::Combo("Hit Sound", &Menu.Visuals.htSound, hitsound, IM_ARRAYSIZE(hitsound));
						ImGui::SliderInt("Size###hittt", &Menu.Visuals.hitmarkerSize, 2, 10, "%.0f%%");
						ImGui::Checkbox("Footstep Esp", &Menu.Visuals.footstepesp);
						ImGui::SameLine();
						ImGui::ColorEdit3("###FootstepEsp", Menu.Colors.FootstepEsp);
						ImGui::Checkbox("Skeleton", &Menu.Visuals.Bones);
						ImGui::SameLine();
						ImGui::ColorEdit3("###Skeletonsclr", Menu.Colors.Skeletons);
						ImGui::Checkbox("Out of FOV arrows", &Menu.Visuals.Radar);
						ImGui::SameLine();
						ImGui::ColorEdit4("###radclr", Menu.Colors.Radar);
					}
					ImGui::EndChild();
					ImGui::SameLine();

					ImGui::BeginChild("Visual 2", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Force Crosshair", &Menu.Visuals.ForceCrosshair);
						ImGui::Checkbox("Grenade trajectory", &Menu.Visuals.GrenadePrediction);
						ImGui::SameLine();
						ImGui::ColorEdit3("###prokecileclr ", Menu.Colors.GrenadePrediction);
					}
					ImGui::EndChild();
					ImGui::BeginChild("Visual 3", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Enable###chamz", &Menu.Visuals.ChamsEnable);

						ImGui::Combo("Type", &Menu.Visuals.ChamsStyle, ModelsMode, IM_ARRAYSIZE(ModelsMode));

						ImGui::Checkbox("Player", &Menu.Visuals.ChamsPlayer);
						ImGui::SameLine();
						ImGui::ColorEdit4("###playerclr", Menu.Colors.PlayerChams);

						ImGui::Checkbox("Player XQZ", &Menu.Visuals.ChamsPlayerWall);
						ImGui::SameLine();
						ImGui::ColorEdit4("###playerxqzclr", Menu.Colors.PlayerChamsWall);

						ImGui::Checkbox("Show backtrack", &Menu.Visuals.ShowBacktrack);
						ImGui::SameLine();
						ImGui::ColorEdit4("###backtrackchams ", Menu.Colors.ChamsHistory);

						ImGui::Checkbox("Local Player##ofodofod", &Menu.Visuals.ChamsL);
						ImGui::SameLine();
						ImGui::ColorEdit4("###localclr111rrr", Menu.Colors.PlayerChamsl);

						ImGui::Checkbox("Local Fake Shadow##ofodofod", &Menu.Visuals.fakelagcham);
						ImGui::SameLine();
						ImGui::ColorEdit4("###nonames", Menu.Colors.nocheat);

						ImGui::Checkbox("Local Glow", &Menu.Visuals.LGlow);
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Usage with Fake Shadow causes visual bugs");
						ImGui::SameLine();
						ImGui::ColorEdit4("###lglowclr ", Menu.Colors.LGlow, 1 << 7);
						if (Menu.Visuals.LGlow)
							ImGui::Checkbox("Pulsating Glow", &Menu.Visuals.PulseLGlow);

						ImGui::Checkbox("Blend model on scope", &Menu.Visuals.blendonscope);
						ImGui::Text("Manual AA Indicator");
						ImGui::Combo("###Manual AA", &Menu.Visuals.manualaatype, manualtype, IM_ARRAYSIZE(manualtype));
						ImGui::SameLine();
						ImGui::ColorEdit3("###manualcolor", Menu.Colors.manualaacolor);
						ImGui::Checkbox("Desync Indicator", &Menu.Antiaim.Indicator);
						ImGui::Checkbox("Hand Chams", &Menu.Misc.WireHand);
						ImGui::SameLine();
						ImGui::ColorEdit4("###wireframehandscol", Menu.Colors.styleshands);
					}
					ImGui::EndChild();
					ImGui::SameLine();
					ImGui::BeginChild("Visual 4", ImVec2(300, 320), true);
					{
						ImGui::Combo("Skybox", &Menu.Visuals.Skybox, Skyboxmode, IM_ARRAYSIZE(Skyboxmode));
						ImGui::Checkbox("Blend props", &Menu.Colors.Props);

						ImGui::Checkbox("Remove smoke", &Menu.Visuals.Nosmoke);
						ImGui::Checkbox("Remove flash", &Menu.Visuals.NoFlash);
						ImGui::Checkbox("Remove visual recoil", &Menu.Visuals.Novisrevoil);
						ImGui::Checkbox("Remove scope", &Menu.Visuals.Noscope);
						ImGui::Checkbox("Remove zoom", &Menu.Misc.static_scope);
						ImGui::Checkbox("Remove post processing", &Menu.Visuals.RemoveParticles);
						ImGui::Checkbox("World Mod", &Menu.Visuals.worldcolor);
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("For a nightmode effect set the color to brown");
						ImGui::SameLine();
						ImGui::ColorEdit3("###worldcolor", Menu.Colors.worldcolor_col);
						if (ImGui::Button("Update World", ImVec2(240, 15))) {
							visuals::worldmod();
						}
						ImGui::Checkbox("Enable Bullet tracer", &Menu.Visuals.BulletTracers);
						ImGui::SameLine();
						ImGui::ColorEdit3("Color", Menu.Colors.Bulletracer);

						ImGui::Text("Thirdperson");
						ImGui::Combo("Key", &Menu.Misc.TPKey, KeyStrings, IM_ARRAYSIZE(KeyStrings));
					}
					ImGui::EndChild();
				}
				if (iTab == 3)
				{
					ImGui::BeginChild("Misc", ImVec2(300, 320), true);
					{
						ImGui::Text("Player Fov");
						ImGui::SliderInt("###Player FOV", &Menu.Misc.PlayerFOV, -50, 50);
						ImGui::Checkbox("View Model Changer", &Menu.Misc.viewmodelchange);
						if (Menu.Misc.viewmodelchange)
						{
							ImGui::Text("Player Viewmodel");
							ImGui::SliderInt("###Player Viewmodel", &Menu.Misc.PlayerViewmodel, 0, 150);
							ImGui::Text("Offset X");
							ImGui::SliderInt("###x", &Menu.Misc.viewmodelx, 0, 10);
							ImGui::Text("Offset Y");
							ImGui::SliderInt("###y", &Menu.Misc.viewmodely, 0, 10);
							ImGui::Text("Offset Z");
							ImGui::SliderInt("###z", &Menu.Misc.viewmodelz, 0, 20);
						}
						ImGui::Checkbox("Preserve Killfeed", &Menu.Misc.PreserveKillfeed);
						ImGui::Checkbox("Aspect Ratio Changer", &Menu.Misc.aspectratioenable);
						if (Menu.Misc.aspectratioenable)
							ImGui::SliderInt("Aspect Ratio", &Menu.Misc.aspectratio, 0, 199);
					}
					ImGui::EndChild();
					ImGui::SameLine();

					ImGui::BeginChild("Misc 2", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Anti untrusted", &Menu.Misc.AntiUT);
					}
					ImGui::EndChild();
					ImGui::BeginChild("Misc 3", ImVec2(300, 320), true);
					{
						ImGui::Checkbox("Bunny hop", &Menu.Misc.AutoJump);
						ImGui::Checkbox("Auto strafe", &Menu.Misc.AutoStrafe);
						ImGui::Text("Circle Strafe");
						ImGui::Combo("###Circle Strafe", &Menu.Misc.circlestrafekey, KeyStrings, ARRAYSIZE(KeyStrings));
						ImGui::Checkbox("Slow Walk", &Menu.Misc.Slowwalkenable);
						ImGui::Combo("###Slow Walk", &Menu.Misc.Slowwalkkey, KeyStrings, ARRAYSIZE(KeyStrings));
						ImGui::Text("Slow Walk Speed");
						ImGui::SliderFloat("###Slowwalkspeed", &Menu.Misc.Slowwalkspeed, 0, 100);
						ImGui::Checkbox("Knife bot", &Menu.Misc.KnifeBot);
						ImGui::Checkbox("Zeus bot", &Menu.Misc.ZeusBot);
						ImGui::Text("Edge Jump");
						ImGui::Combo("###Edge Jump", &Menu.Misc.edgejumpkey, KeyStrings, ARRAYSIZE(KeyStrings));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Jump at the edge of a surface");
						if (Menu.Misc.edgejumpkey > 0)
						{
							ImGui::Checkbox("strafe in air", &Menu.Misc.strafeedgejump);
						}
						static const char* clantagcbb[] =
						{
							"Off",
							"sippin' piss",
							"custom"
						};
						ImGui::Text("Clantag");
						ImGui::Combo("Clantag", &Menu.Misc.Clantag, clantagcbb, ARRAYSIZE(clantagcbb));
						if (Menu.Misc.Clantag == 2)
						{
							ImGui::InputText("##Custom Tag", Menu.Misc.customclantag, 128);
						}
						ImGui::Checkbox("Infinite Duck", &Menu.Misc.infiniteduck);
						//ImGui::Text("Fake Duck");
						//ImGui::Combo("###Fake Duck Key", &Menu.Misc.fakeduckkey, KeyStrings, ARRAYSIZE(KeyStrings));
						ImGui::Checkbox("Fake latency", &Menu.Ragebot.FakeLatency);
						if (Menu.Ragebot.FakeLatency) {
							ImGui::SliderInt("value", &Menu.Ragebot.FakeLatencyAmount, 0, 1000);
						}
					}
					ImGui::EndChild();
					ImGui::SameLine();
					ImGui::BeginChild("Misc 4", ImVec2(300, 320), true);
					{
						ImGui::InputText("CFG Name", Menu.Misc.configname, 128);
						static int sel;
						std::string config;
						std::vector<std::string> configs = Menu.GetConfigs();
						if (configs.size() > 0) {
							ImGui::ComboBoxArray("configs", &sel, configs);
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
							config = configs[Menu.Misc.ConfigSelection];
						}
						Menu.Misc.ConfigSelection = sel;
						if (ImGui::Button("new cfg", ImVec2(284, 50)))
						{
							std::string ConfigFileName = Menu.Misc.configname;
							if (ConfigFileName.size() < 1)
							{
								ConfigFileName = "null";
							}
							Menu.CreateConfig(ConfigFileName);
						}

						if (configs.size() > 0) {
							if (ImGui::Button("load cfg", ImVec2(140, 50)))
							{
								Menu.Load(config);
							}
						}
						ImGui::SameLine();

						if (configs.size() >= 1) {
							if (ImGui::Button("save cfg", ImVec2(140, 50)))
							{
								Menu.Save(config);
							}
						}
						ImGui::SameLine();
					}
					ImGui::EndChild();
				}
				if (iTab == 4)
				{
					ImGui::BeginChild("SkinChanger", ImVec2(600, 70), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
					{
						if (ImGui::Button("Force update", ImVec2(585, 55)))
						{
							if (g_pEngine->IsInGame() && g_pEngine->IsConnected() && csgo::LocalPlayer && csgo::LocalPlayer->isAlive())
								FullUpdate();
						}
					}
					ImGui::EndChild();
					ImGui::BeginChild("SkinChanger 2", ImVec2(600, 570), true);
					{
						ImGui::Checkbox("Enable SkinChanger", &Menu.Skinchanger.skinenabled);
						ImGui::Checkbox("Enable GloveChanger", &Menu.Skinchanger.glovesenabled);
						ImGui::Separator();
						static int skins = 0;
						if (ImGui::Button("Pistol", ImVec2(80, 40)))
							skins = 0;
						ImGui::SameLine();
						if (ImGui::Button("Rifle", ImVec2(80, 40)))
							skins = 1;
						ImGui::SameLine();
						if (ImGui::Button("Sniper", ImVec2(80, 40)))
							skins = 4;
						ImGui::SameLine();
						if (ImGui::Button("SMG", ImVec2(80, 40)))
							skins = 2;
						ImGui::SameLine();
						if (ImGui::Button("Heavy", ImVec2(80, 40)))
							skins = 3;
						ImGui::SameLine();
						if (ImGui::Button("Knife", ImVec2(80, 40)))
							skins = 5;
						ImGui::SameLine();
						if (ImGui::Button("Gloves", ImVec2(80, 40)))
							skins = 6;
						ImGui::Separator();
						if (skins == 0)
						{
							ImGui::Combo(("Glock-18"), &Menu.Skinchanger.GlockSkin, glock, ARRAYSIZE(glock));
							ImGui::Combo(("USP-S"), &Menu.Skinchanger.USPSkin, usp, ARRAYSIZE(usp));
							ImGui::Combo(("Deagle"), &Menu.Skinchanger.DeagleSkin, deagle, ARRAYSIZE(deagle));
							ImGui::Combo(("Five-Seven"), &Menu.Skinchanger.FiveSkin, five, ARRAYSIZE(five));
							ImGui::Combo(("Revolver"), &Menu.Skinchanger.RevolverSkin, revolver, ARRAYSIZE(revolver));
							ImGui::Combo(("TEC-9"), &Menu.Skinchanger.tec9Skin, tec9, ARRAYSIZE(tec9));
							ImGui::Combo(("Dual-Barettas"), &Menu.Skinchanger.DualSkin, dual, ARRAYSIZE(dual));
							ImGui::Combo(("Cz75-Auto"), &Menu.Skinchanger.Cz75Skin, cz75, ARRAYSIZE(cz75));
							ImGui::Combo(("P250"), &Menu.Skinchanger.P250Skin, p250, ARRAYSIZE(p250));
							ImGui::Combo(("P2000"), &Menu.Skinchanger.P2000Skin, p2000, ARRAYSIZE(p2000));
						}

						if (skins == 1)
						{
							ImGui::Combo(("AK-47"), &Menu.Skinchanger.AK47Skin, ak47, ARRAYSIZE(ak47));
							ImGui::Combo(("M4A1-S"), &Menu.Skinchanger.M4A1SSkin, m4a1s, ARRAYSIZE(m4a1s));
							ImGui::Combo(("M4A4"), &Menu.Skinchanger.M4A4Skin, m4a4, ARRAYSIZE(m4a4));
							ImGui::Combo(("AUG"), &Menu.Skinchanger.AUGSkin, aug, ARRAYSIZE(aug));
							ImGui::Combo(("FAMAS"), &Menu.Skinchanger.FAMASSkin, famas, ARRAYSIZE(famas));
							ImGui::Combo(("Galil AR"), &Menu.Skinchanger.GalilSkin, galil, ARRAYSIZE(galil));
							ImGui::Combo(("Sg553"), &Menu.Skinchanger.Sg553Skin, sg553, ARRAYSIZE(sg553));
						}

						if (skins == 4)
						{
							ImGui::Combo(("Scar-20"), &Menu.Skinchanger.SCAR20Skin, scar20, ARRAYSIZE(scar20));
							ImGui::Combo(("G3SG1"), &Menu.Skinchanger.G3sg1Skin, g3sg1, ARRAYSIZE(g3sg1));
							ImGui::Combo(("AWP"), &Menu.Skinchanger.AWPSkin, awp, ARRAYSIZE(awp));
							ImGui::Combo(("SSG08"), &Menu.Skinchanger.SSG08Skin, ssg08, ARRAYSIZE(ssg08));
						}

						if (skins == 2)
						{
							ImGui::Combo(("MP9"), &Menu.Skinchanger.Mp9Skin, mp9, ARRAYSIZE(mp9));
							ImGui::Combo(("PP-Bizon"), &Menu.Skinchanger.BizonSkin, bizon, ARRAYSIZE(bizon));
							ImGui::Combo(("MAC-10"), &Menu.Skinchanger.Mac10Skin, mac10, ARRAYSIZE(mac10));
							ImGui::Combo(("UMP45"), &Menu.Skinchanger.UMP45Skin, ump45, ARRAYSIZE(ump45));
						}

						if (skins == 3)
						{
							ImGui::Combo(("Nova"), &Menu.Skinchanger.NovaSkin, nova, ARRAYSIZE(nova));
							ImGui::Combo(("Sawed-Off"), &Menu.Skinchanger.SawedSkin, sawed, ARRAYSIZE(sawed));
							ImGui::Combo(("Mag-7"), &Menu.Skinchanger.MagSkin, mag, ARRAYSIZE(mag));
							ImGui::Combo(("XM1014"), &Menu.Skinchanger.XmSkin, xm, ARRAYSIZE(xm));
							ImGui::Combo(("M249"), &Menu.Skinchanger.M249Skin, m249, ARRAYSIZE(m249));
							ImGui::Combo(("Negev"), &Menu.Skinchanger.NegevSkin, negev, ARRAYSIZE(negev));
						}
						if (skins == 5)
						{
							ImGui::Combo(("Knife Model"), &Menu.Skinchanger.Knife, KnifeModel, ARRAYSIZE(KnifeModel));
							ImGui::Combo(("Knife Skin"), &Menu.Skinchanger.KnifeSkin, knifeskins, ARRAYSIZE(knifeskins));
						}

						if (skins == 6)
						{
							ImGui::Combo(("Glove Model"), &Menu.Skinchanger.gloves, gloves, ARRAYSIZE(gloves));
							ImGui::Combo(("Glove Skin"), &Menu.Skinchanger.skingloves, gloveskin, ARRAYSIZE(gloveskin));
						}
					}
					ImGui::EndChild();
					//ImGui::Text("Coming Soon...");
				}
				if (iTab == 5)
				{
					static int currentPlayer = -1;

					if (!g_pEngine->IsInGame())
						currentPlayer = -1;
					ImGui::BeginChild("plist", ImVec2(300, 640), true);
					{
						ImGui::Text("Player");
						if (g_pEngine->IsInGame())
						{
							for (int i = 1; i < 65; i++)
							{
								if (i == g_pEngine->GetLocalPlayer())
									continue;

								CBaseEntity* Target = g_pEntitylist->GetClientEntity(i);
								if (!Target)
									continue;

								if (Target->GetTeamNum() == csgo::LocalPlayer->GetTeamNum())
									continue;

								player_info_t entityInformation;

								if (!g_pEngine->GetPlayerInfo(i, &entityInformation))
									continue;

								if (entityInformation.m_szPlayerName == "GOTV" && !Target->GetActiveWeaponIndex())
									continue;

								ImGui::Separator();
								char buf[255]; sprintf_s(buf, u8"%s", entityInformation.m_szPlayerName);
								if (ImGui::Selectable(buf, i == currentPlayer, ImGuiSelectableFlags_SpanAllColumns))
									currentPlayer = i;
							}
						}
					}
					ImGui::EndChild();
					ImGui::SameLine();
					ImGui::BeginChild("plist2", ImVec2(300, 640), true);
					{
						player_info_t entityInformation;
						g_pEngine->GetPlayerInfo(currentPlayer, &entityInformation);
						ImGui::Text("what u do 2 them");
						if (currentPlayer != -1)
						{
							bool shouldResolve = std::find(CPlayerList::Players.begin(), CPlayerList::Players.end(), entityInformation.m_nUserID) != CPlayerList::Players.end();
							if (ImGui::Checkbox(("dont resolve"), &shouldResolve))
							{
								if (shouldResolve)
									CPlayerList::Players.push_back(entityInformation.m_nUserID);
								else
									CPlayerList::Players.erase(std::find(CPlayerList::Players.begin(), CPlayerList::Players.end(), entityInformation.m_nUserID));
							}
							static const char* pitch[] =
							{
								"down",
								"up",
								"zero",
								"emotion"
							};
							cPlayersList Players;
							ImGui::Combo(("PitchFix"), &Menu.Players2.AAA[currentPlayer].PAngle, pitch, ARRAYSIZE(pitch));
						}

						if (ImGui::Button(("Clear NoResolve"), ImVec2(93.f, 20.f)))
							CPlayerList::Players.clear();
						if (ImGui::Button(("Clear Fixes"), ImVec2(93.f, 20.f))) {
							if (g_pEngine->IsInGame())
							{
								for (int i = 1; i < 65; i++)
								{
									Menu.Players2.AAA[i].PAngle = 0;
								}
							}
						}

					}
					ImGui::EndChild();

				}
			}ImGui::EndChild();

		}ImGui::End();
	}
}


HRESULT __stdcall Hooks::D3D9_EndScene(IDirect3DDevice9* pDevice)
{
	HRESULT result = d3d9VMT->GetOriginalMethod<EndSceneFn>(42)(pDevice);
	
	if (!csgo::Init)
		GUI_Init(pDevice);

	static void* dwReturnAddress = _ReturnAddress();

	if (dwReturnAddress == _ReturnAddress())
	{
		SaveState(pDevice);

		spread_crosshair(pDevice);
		draw_menu();
		ImGui::Render();

		RestoreState(pDevice);
	}
	return result;
}

HRESULT __stdcall Hooks::hkdReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresParam)
{
	if (!csgo::Init)
		return oResetScene(pDevice, pPresParam);

	ImGui_ImplDX9_InvalidateDeviceObjects();

	auto hr = oResetScene(pDevice, pPresParam);

	if (hr >= 0)
		ImGui_ImplDX9_CreateDeviceObjects();

	return hr;
}