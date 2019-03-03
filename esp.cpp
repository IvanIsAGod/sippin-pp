#include "esp.h"
#include "event_log.h"
#include "freestand autowall.h"
#include "Aimbot.h"
static int dormantAlpha[65];

char* GetWeaponString(CBaseCombatWeapon *pWeapon)
{
	int ID = pWeapon->WeaponID();

	switch (ID) {
	case 4:
		return "Glock";
	case 2:
		return "Elite";
	case 36:
		return "P250";
	case 30:
		return "Tec9";
	case 1:
		return "Deagle";
	case 32:
		return "P2000";
	case 3:
		return "FiveSeven";
	case 64:
		return "Revolver";
	case 63:
		return "CZ75 Auto";
	case 61:
		return "USP";
	case 35:
		return "Nova";
	case 25:
		return "XM1014";
	case 29:
		return "Sawed Off";
	case 14:
		return "M249";
	case 28:
		return "Negev";
	case 27:
		return "Mag7";
	case 17:
		return "Mac10";
	case 33:
		return "MP7";
	case 23:
		return "MP5";
	case 24:
		return "UMP-45";
	case 19:
		return "P90";
	case 26:
		return "Bizon";
	case 34:
		return "MP9";
	case 10:
		return "Famas";
	case 16:
		return "M4A4";
	case 40:
		return "SSG08";
	case 8:
		return "Aug";
	case 9:
		return "AWP";
	case 38:
		return "SCAR20";
	case 13:
		return "Galil";
	case 7:
		return "Ak47";
	case 39:
		return "SG553";
	case 11:
		return "G3SG1";
	case 60:
		return "M4A1-S";
	case 46:
		return "INC";
	case 48:
		return "Molotov";
	case 44:
		return "Grenade";
	case 43:
		return "Flashbang";
	case 45:
		return "Smoke";
	case 47:
		return "Decoy";
	case 31:
		return "Taser";
	default:
		return "Knife";
	}
}

mstudiobbox_t* get_hitbox1(CBaseEntity* entity, int hitbox_index)
{
	if (entity->IsDormant() || entity->GetHealth() <= 0)
		return NULL;

	const auto pModel = entity->GetModel();
	if (!pModel)
		return NULL;

	auto pStudioHdr = g_pModelInfo->GetStudioModel(pModel);
	if (!pStudioHdr)
		return NULL;

	auto pSet = pStudioHdr->pHitboxSet(0);
	if (!pSet)
		return NULL;

	if (hitbox_index >= pSet->numhitboxes || hitbox_index < 0)
		return NULL;

	return pSet->pHitbox(hitbox_index);
}

bool IsOnScreen(Vector origin, Vector& screen)
{
	if (!GameUtils::WorldToScreen(origin, screen)) return false;
	int iScreenWidth, iScreenHeight;
	g_pEngine->GetScreenSize(iScreenWidth, iScreenHeight);
	bool xOk = iScreenWidth > screen.x > 0, yOk = iScreenHeight > screen.y > 0;
	return xOk && yOk;
}

bool IsFakeDucking(CBaseEntity* entity)
{
	int storedTick = 0;
	int crouchedTicks[64];
	float duckamount = entity->m_flDuckAmount();
	if (!duckamount) return false;
	float duckspeed = entity->m_flDuckSpeed();
	if (!duckspeed) return false;

	if (storedTick != g_pGlobals->tickcount)
	{
		crouchedTicks[entity->GetIndex()]++;
		storedTick = g_pGlobals->tickcount;
	}
	if (duckspeed == 8 && duckamount <= 0.9 && duckamount > 0.01 && (*entity->GetFlags() & FL_ONGROUND) && (crouchedTicks[entity->GetIndex()] >= 5))
		return true;
	else 
		return false;
}

void player(CBaseEntity* entity)
{
	int i = entity->GetIndex();

	player_info_t info;
	g_pEngine->GetPlayerInfo(i, &info);

	auto weapon = reinterpret_cast<CBaseCombatWeapon*>(g_pEntitylist->GetClientEntity(entity->GetActiveWeaponIndex()));

	if (!weapon)
		return;

	Vector min, max;
	entity->GetRenderBounds(min, max);
	Vector pos, pos3D, top, top3D;
	pos3D = entity->GetOrigin() - Vector(0, 0, 10);
	top3D = pos3D + Vector(0, 0, max.z + 10);

	if (GameUtils::WorldToScreen(pos3D, pos) && GameUtils::WorldToScreen(top3D, top))
	{
		int height = (pos.y - top.y);
		int width = height / 2;

		if (Menu.Visuals.BoundingBox)
		{
			Color clr = entity->IsDormant() ? Color::Grey() : Color(Menu.Colors.BoundingBox);

			draw::DrawEmptyRect(pos.x - width / 2, top.y, (pos.x - width / 2) + width, top.y + height, clr);
			draw::DrawEmptyRect((pos.x - width / 2) + 1, top.y + 1, (pos.x - width / 2) + width - 1, top.y + height - 1, Color(10, 10, 10, dormantAlpha[i] / 2));
			draw::DrawEmptyRect((pos.x - width / 2) - 1, top.y - 1, (pos.x - width / 2) + width + 1, top.y + height + 1, Color(10, 10, 10, dormantAlpha[i] / 2));
		}

		if (Menu.Visuals.Name)
		{
			Color clr = entity->IsDormant() ? Color::Grey() : Color(Menu.Colors.Name, dormantAlpha[i]);

			setlocale(LC_ALL, "ru");

			draw::DrawF(pos.x, top.y - 7, F::esp, true, true, clr, info.m_szPlayerName);
		}

		if (Menu.Visuals.Weapon)
		{
			Color clr = entity->IsDormant() ? Color::Grey() : Color(Menu.Colors.wpn, dormantAlpha[i]);

			auto pos_y = Menu.Visuals.Ammo ? pos.y + 13 : pos.y + 8;

			std::string weapon_name = GetWeaponString(weapon);

			draw::DrawF(pos.x, pos_y, F::esp2, true, true, clr, weapon_name.c_str());
		}

		if (Menu.Visuals.Ammo)
		{
			Color clr = entity->IsDormant() ? Color::Grey() : Color(Menu.Colors.ammo, dormantAlpha[i]);

			int height = (pos.y - top.y);

			float offset = (height / 4.f) + 5;
			UINT hp = height - (UINT)((height * 3) / 100);

			auto animLayer = entity->GetAnimOverlay(1);

			auto activity = entity->GetSequenceActivity(animLayer->m_nSequence);

			int iClip = weapon->GetLoadedAmmo();
			int iClipMax = weapon->GetCSWpnData()->max_clip;

			float box_w = (float)fabs(height / 2);
			float width;
			if (activity == 967 && animLayer->m_flWeight != 0.f)
			{
				float cycle = animLayer->m_flCycle;
				width = (((box_w * cycle) / 1.f));
			}
			else
				width = (((box_w * iClip) / iClipMax));

			draw::DrawFilledRect((pos.x - box_w / 2), top.y + height + 3, (pos.x - box_w / 2) + box_w + 2, top.y + height + 7, Color(10, 10, 10, dormantAlpha[i] / 2));
			draw::DrawFilledRect((pos.x - box_w / 2) + 1, top.y + height + 4, (pos.x - box_w / 2) + width + 1, top.y + height + 6, clr);
		}

		if (Menu.Visuals.Health)
		{
			int enemy_hp = entity->GetHealth();
			int hp_red = 255 - (enemy_hp * 2.55);
			int hp_green = enemy_hp * 2.55;
			Color health_color = entity->IsDormant() ? Color::Grey() : Color(hp_red, hp_green, 1, dormantAlpha[i]);

			float offset = (height / 4.f) + 5;
			UINT hp = height - (UINT)((height * enemy_hp) / entity->GetMaxHealth());

			draw::DrawEmptyRect((pos.x - width / 2) - 6, top.y - 1, (pos.x - width / 2) - 3, top.y + height + 1, Color(10, 10, 10, dormantAlpha[i] / 2));
			draw::DrawLine((pos.x - width / 2) - 4, top.y + hp, (pos.x - width / 2) - 4, top.y + height, health_color);
			draw::DrawLine((pos.x - width / 2) - 5, top.y + hp, (pos.x - width / 2) - 5, top.y + height, health_color);
		}

		if (Menu.Visuals.Flags)
		{
			std::vector<std::pair<std::string, Color>> info;

			Color info_clr = entity->IsDormant() ? Color::Grey() : Color(Menu.Colors.f, dormantAlpha[i]);

			if (Menu.Visuals.Scoped && entity->IsScoped())
				info.push_back(std::pair<std::string, Color>("scoped", info_clr));
			if (Menu.Visuals.Armor && entity->GetArmor() > 0)
				info.push_back(std::pair<std::string, Color>(entity->GetArmorName(), info_clr));
			if (Menu.Visuals.FakeDuck && IsFakeDucking(entity))
				info.push_back(std::pair<std::string, Color>("Fake Duck", info_clr));
			if (Menu.Ragebot.ResolverType == 1)
				info.push_back(std::pair<std::string, Color>(csgo::resolvermode, info_clr));

			for (auto text : info)
			{
				int i = 0;
				draw::DrawF((pos.x + width / 2) + 5, top.y + i, F::esp2, false, false, text.second, text.first);
				i += 8;
			}
		}

		if (Menu.Visuals.Bones)
		{
			studiohdr_t* pStudioHdr = g_pModelInfo->GetStudioModel(entity->GetModel());

			if (!pStudioHdr)
				return;

			Vector vParent, vChild, sParent, sChild;

			for (int j = 0; j < pStudioHdr->numbones; j++)
			{
				mstudiobone_t* pBone = pStudioHdr->pBone(j);

				if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
				{
					vChild = entity->GetBonePos(j);
					vParent = entity->GetBonePos(pBone->parent);

					if (GameUtils::WorldToScreen(vParent, sParent) && GameUtils::WorldToScreen(vChild, sChild))
					{
						draw::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(Menu.Colors.Skeletons, dormantAlpha[i]));
					}
				}
			}
		}
	}
}

void DrawBarrel(CBaseEntity* entity)
{

	auto get_hitbox_pos = [](CBaseEntity* entity, int hitbox_id)
	{
		auto hitbox = get_hitbox1(entity, hitbox_id);
		if (!hitbox)
			return Vector(0, 0, 0);

		auto bone_matrix = entity->GetBoneMatrix(hitbox->bone);

		Vector bbmin, bbmax;
		Math::VectorTransform(hitbox->bbmin, bone_matrix, bbmin);
		Math::VectorTransform(hitbox->bbmax, bone_matrix, bbmax);

		return (bbmin + bbmax) * 0.5f;
	};


	auto local_player = csgo::LocalPlayer;

	if (!local_player)
		return;

	if (entity->IsDormant())
		return;

	if (!Menu.Visuals.Radar)
		return;

	Vector screenPos, client_viewangles;
	if (IsOnScreen(get_hitbox_pos(entity, (int)CSGOHitboxID::PELVIS), screenPos)) return;

	int screen_width = 0, screen_height = 0;
	float radius = 300;

	g_pEngine->GetViewAngles(client_viewangles);

	g_pEngine->GetScreenSize(screen_width, screen_height);

	Vector local_position = local_player->GetOrigin() + local_player->GetViewOffset();

	const auto screen_center = Vector(screen_width / 2.f, screen_height / 2.f, 0);
	const auto rot = DEG2RAD(client_viewangles.y - GameUtils::CalculateAngle(local_position, get_hitbox_pos(entity, (int)CSGOHitboxID::PELVIS)).y - 90);

	std::vector<Vertex_t> vertices;
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius)));
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot + DEG2RAD(2)) * (radius - 35), screen_center.y + sinf(rot + DEG2RAD(2)) * (radius - 35))));
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot - DEG2RAD(2)) * (radius - 35), screen_center.y + sinf(rot - DEG2RAD(2)) * (radius - 35))));

	Color col;
	draw::TexturedPolygon(3, vertices, Color(Menu.Colors.Radar, 150));
}

Vector get_hitbox_pos(CBaseEntity* entity, int hitbox_id)
{
	auto getHitbox = [](CBaseEntity* entity, int hitboxIndex) -> mstudiobbox_t*
	{
		if (entity->IsDormant() || entity->GetHealth() <= 0) return NULL;

		const auto pModel = entity->GetModel();
		if (!pModel) return NULL;

		auto pStudioHdr = g_pModelInfo->GetStudioModel(pModel);
		if (!pStudioHdr) return NULL;

		auto pSet = pStudioHdr->pHitboxSet(0);
		if (!pSet) return NULL;

		if (hitboxIndex >= pSet->numhitboxes || hitboxIndex < 0) return NULL;

		return pSet->GetHitbox(hitboxIndex);
	};

	auto hitbox = getHitbox(entity, hitbox_id);
	if (!hitbox) return Vector(0, 0, 0);

	auto bone_matrix = entity->GetBoneMatrix(hitbox->bone);

	Vector bbmin, bbmax;
	Math::VectorTransform(hitbox->bbmin, bone_matrix, bbmin);
	Math::VectorTransform(hitbox->bbmax, bone_matrix, bbmax);

	return (bbmin + bbmax) * 0.5f;
}
void DrawFovArrows(CBaseEntity* entity)
{
	auto local_player = csgo::LocalPlayer;
	if (!local_player) return;
	if (entity->IsDormant()) return;

	Vector screenPos, client_viewangles;
	int screen_width = 0, screen_height = 0;
	float radius = 300.f;

	if (IsOnScreen(get_hitbox_pos(entity, (int)CSGOHitboxID::HEAD), screenPos)) return;

	g_pEngine->GetViewAngles(client_viewangles);

	g_pEngine->GetScreenSize(screen_width, screen_height);

	const auto screen_center = Vector(screen_width / 2.f, screen_height / 2.f, 0);
	const auto rot = DEG2RAD(client_viewangles.y - GameUtils::CalcAngle(local_player->GetEyePosition(), get_hitbox_pos(entity, (int)CSGOHitboxID::HEAD)).y - 90);

	std::vector<Vertex_t> vertices;

	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius)));
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot + DEG2RAD(2)) * (radius - 16), screen_center.y + sinf(rot + DEG2RAD(2)) * (radius - 16))));
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot - DEG2RAD(2)) * (radius - 16), screen_center.y + sinf(rot - DEG2RAD(2)) * (radius - 16))));

	draw::TexturedPolygon(3, vertices, Color(Menu.Colors.Radar, 150));
}

void local_esp()
{
	if (csgo::LocalPlayer && csgo::LocalPlayer->isAlive())
	{
		if (Menu.Visuals.Noscope && csgo::LocalPlayer->IsScoped())
		{
			draw::DrawLine(csgo::Screen.width / 2, 0, csgo::Screen.width / 2, csgo::Screen.height, Color::Black());
			draw::DrawLine(0, csgo::Screen.height / 2, csgo::Screen.width, csgo::Screen.height / 2, Color::Black());
		}
		if (Menu.Antiaim.Indicator)
		{
			float fake_strength = csgo::LocalPlayer->poseparameterptr()[11] * 58;
			if (fake_strength >= 30.f)
				draw::Textf(10, csgo::Screen.height - 80, Color(0, 102, 255), F::indicators, "Desync");
			else if (fake_strength >= 20.f)
				draw::Textf(10, csgo::Screen.height - 80, Color(10, 240, 10), F::indicators, "Desync");
			else if (fake_strength >= 10.f)
				draw::Textf(10, csgo::Screen.height - 80, Color(240, 200, 0), F::indicators, "Desync");
			else
				draw::Textf(10, csgo::Screen.height - 80, Color(255, 0, 0), F::indicators, "Desync");
		}
		if (Menu.Antiaim.DirectonType == 1)
		{
			int ssw, ssh;
			g_pEngine->GetScreenSize(ssw, ssh);
			int cntrscr_x = ssw / 2;
			int cntrscr_y = ssh / 2;

			if (csgo::isleft) {
				if (Menu.Visuals.manualaatype == 0) {
					draw::Text(cntrscr_x - 35, cntrscr_y, Color(Menu.Colors.manualaacolor, 255), F::indicators, "<");
					draw::Text(cntrscr_x + 35, cntrscr_y, Color(200, 200, 200, 175), F::indicators, ">");
				}
				else if (Menu.Visuals.manualaatype = 1) draw::Text(10, ssh - 560, Color(Menu.Colors.manualaacolor, 255), F::indicators, "LEFT");
			}
			else {
				if (Menu.Visuals.manualaatype == 0) {
					draw::Text(cntrscr_x - 35, cntrscr_y, Color(200, 200, 200, 175), F::indicators, "<");
					draw::Text(cntrscr_x + 35, cntrscr_y, Color(Menu.Colors.manualaacolor, 255), F::indicators, ">");
				}
				else if (Menu.Visuals.manualaatype == 1) draw::Text(10, ssh - 560, Color(Menu.Colors.manualaacolor, 255), F::indicators, "RIGHT");
			}
		}
	}
}

void visuals::worldmod() {
	if (g_pEngine->IsConnected() && g_pEngine->IsInGame() && csgo::LocalPlayer) {

		static ConVar*r_DrawSpecificStaticProp;
		if (!r_DrawSpecificStaticProp)
			r_DrawSpecificStaticProp = g_pCvar->FindVar("r_DrawSpecificStaticProp");
		r_DrawSpecificStaticProp->SetValue(0);
		for (MaterialHandle_t i = g_pMaterialSystem->FirstMaterial(); i != g_pMaterialSystem->InvalidMaterial(); i = g_pMaterialSystem->NextMaterial(i))
		{
			IMaterial* pMaterial = g_pMaterialSystem->GetMaterial(i);
			if (!pMaterial)
				continue;
			if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
			{
				if (Menu.Visuals.worldcolor)
					pMaterial->ColorModulate(Menu.Colors.worldcolor_col[0], Menu.Colors.worldcolor_col[1], Menu.Colors.worldcolor_col[2]);
				else
					pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
			}
		}
	}
}

void visuals::Do()
{
	if (Menu.Visuals.EspEnable)
	{
		if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
		{
			if (csgo::LocalPlayer && csgo::LocalPlayer->isAlive())
			{
				local_esp();
				Eventlog::Draw();

				for (int i = 0; i < g_pGlobals->maxClients; ++i)
				{
					auto entity = g_pEntitylist->GetClientEntity(i);
					if (!entity || !entity->IsPlayer() || entity == csgo::LocalPlayer || !entity->isAlive()
						|| entity->GetTeamNum() == csgo::LocalPlayer->GetTeamNum())
						continue;

					if (entity->IsDormant() && dormantAlpha[entity->GetIndex()] != 0.f)
					{
						if (dormantAlpha[entity->GetIndex()] < 0.f)
							dormantAlpha[entity->GetIndex()] = 0.f;
						else
							dormantAlpha[entity->GetIndex()] -= 0.3f;
					}
					else if (dormantAlpha[entity->GetIndex()] != 255 && !entity->IsDormant())
						dormantAlpha[entity->GetIndex()] = 255;

					if (dormantAlpha[entity->GetIndex()] > 0)
					{
						player(entity);
					}
					//DrawBarrel(entity);
					if (Menu.Visuals.Radar)
						DrawFovArrows(entity);
				}
			}
		}
	}
}