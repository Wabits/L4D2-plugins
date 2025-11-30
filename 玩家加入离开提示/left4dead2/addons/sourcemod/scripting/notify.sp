#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <colors>
#include <baidugeo>

#undef REQUIRE_EXTENSIONS
#tryinclude <left4dhooks>
#define REQUIRE_EXTENSIONS

#define IsConnecting "ambient/alarms/klaxon1.wav"
#define IsConnected "buttons/button11.wav"
#define IsDisconnect "buttons/button4.wav"
#define IsLeftSafeArea "level/countdown.wav"
#define IsStart "level/loud/bell_break.wav"

ConVar cv_SafeArea, cv_Isconnecting, cv_Country, cv_ApiKey;

bool showNotify[MAXPLAYERS + 1], g_GeoIPReady[MAXPLAYERS + 1];
char g_PlayerCountry[MAXPLAYERS + 1][64], g_PlayerProvince[MAXPLAYERS + 1][64], g_PlayerCity[MAXPLAYERS + 1][64];

public Plugin myinfo = {
	name = "[L4D2]提示信息音效",
	description = "玩家加入/离开提示",
	author = "落樱",
	version = "2.5.0"
};

public void OnPluginStart()
{
	cv_Isconnecting = CreateConVar("l4d2_connecting_notify", "1", "连接中提示", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	cv_Country = CreateConVar("l4d2_country_notify", "1", "加入服务器后国家提示", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	cv_SafeArea = CreateConVar("l4d2_leftsafe_sound", "1", "离开安全屋提示音", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	cv_ApiKey = CreateConVar("l4d2_baidugeo_key", "", "百度地图API Key", FCVAR_PROTECTED);

	HookEvent("player_disconnect", Event_PlayerDisconnect, EventHookMode_Pre);

	cv_ApiKey.AddChangeHook(OnApiKeyChanged);
	
	AutoExecConfig(true, "notify");
	
	char key[64];
	cv_ApiKey.GetString(key, sizeof(key));
	if (strlen(key) > 0) {
		BaiduGeo_SetAPIKey(key);
	}
}

public void OnApiKeyChanged(ConVar convar, const char[] oldValue, const char[] newValue)
{
	BaiduGeo_SetAPIKey(newValue);
}

public void OnConfigsExecuted()
{
	ConVar cv = FindConVar("l4d2_baidugeo_key");
	if (cv != null)
	{
		char key[64];
		cv.GetString(key, sizeof(key));
		BaiduGeo_SetAPIKey(key);
	}
	BaiduGeo_SetTimeout(5.0);
}

public void OnMapStart()
{
	PrecacheSound(IsConnecting);
	PrecacheSound(IsConnected);
	PrecacheSound(IsDisconnect);
	PrecacheSound(IsLeftSafeArea);
	PrecacheSound(IsStart);
}

#if defined _l4dh_included
public Action L4D_OnFirstSurvivorLeftSafeArea(int client)
{
	if (cv_SafeArea.BoolValue) {
		PlaySound(IsLeftSafeArea);
		CreateTimer(3.0, SoundTimer, _, TIMER_FLAG_NO_MAPCHANGE);
	}
	return Plugin_Continue;
}

public Action SoundTimer(Handle timer)
{
	PlaySound(IsStart);
	return Plugin_Continue;
}
#endif

public void OnClientConnected(int client)
{
	if (IsFakeClient(client))
		return;

	g_GeoIPReady[client] = false;
	Format(g_PlayerCountry[client], sizeof(g_PlayerCountry[]), "未知");
	Format(g_PlayerProvince[client], sizeof(g_PlayerProvince[]), "");
	Format(g_PlayerCity[client], sizeof(g_PlayerCity[]), "");

	char ip[32];
	GetClientIP(client, ip, sizeof(ip));
	BaiduGeo_QueryPlayerLocation(GetClientUserId(client), ip);

	if (cv_Isconnecting.BoolValue) {
		CPrintToChatAll("{olive}▸ {green}玩家 {olive}> {lightgreen}%N {olive}正在连接...", client);
		PlaySound(IsConnecting);
	}
}

public void BaiduGeo_OnLocationReceived(int userid, const char[] ip, const char[] country, const char[] province, const char[] city)
{
	int client = GetClientOfUserId(userid);
	if (client == 0 || !IsClientConnected(client))
		return;

	LogMessage("BaiduGeo Success for IP %s: Country=%s, Province=%s, City=%s", ip, country, province, city);

	if (strlen(country) > 0) {
		Format(g_PlayerCountry[client], sizeof(g_PlayerCountry[]), "%s", country);
		Format(g_PlayerProvince[client], sizeof(g_PlayerProvince[]), "%s", province);
		Format(g_PlayerCity[client], sizeof(g_PlayerCity[]), "%s", city);
	} else {
		Format(g_PlayerCountry[client], sizeof(g_PlayerCountry[]), "未知");
		Format(g_PlayerProvince[client], sizeof(g_PlayerProvince[]), "");
		Format(g_PlayerCity[client], sizeof(g_PlayerCity[]), "");
	}

	g_GeoIPReady[client] = true;
	if (!showNotify[client] && IsClientInGame(client))
		CreateTimer(1.0, Timer_ShowJoinMessage, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
}

public void BaiduGeo_OnLocationError(int userid, const char[] ip, int errorCode, const char[] errorMsg)
{
	int client = GetClientOfUserId(userid);
	if (client == 0 || !IsClientConnected(client))
		return;

	Format(g_PlayerCountry[client], sizeof(g_PlayerCountry[]), "未知");
	Format(g_PlayerProvince[client], sizeof(g_PlayerProvince[]), "");
	Format(g_PlayerCity[client], sizeof(g_PlayerCity[]), "");
	g_GeoIPReady[client] = true;

	if (!showNotify[client] && IsClientInGame(client))
		CreateTimer(1.0, Timer_ShowJoinMessage, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
}

public void OnClientPutInServer(int client)
{
	if (IsFakeClient(client))
		return;

	if (!showNotify[client] && g_GeoIPReady[client])
		CreateTimer(1.0, Timer_ShowJoinMessage, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
}

public Action Timer_ShowJoinMessage(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (client > 0 && IsClientInGame(client) && !showNotify[client])
		ShowPlayerJoinMessage(client);
	return Plugin_Continue;
}

void ShowPlayerJoinMessage(int client)
{
	if (showNotify[client] || !IsClientInGame(client))
		return;

	char playerName[MAX_NAME_LENGTH];
	GetClientName(client, playerName, sizeof(playerName));

	CPrintToChatAll("{olive}▸ {green}玩家 {olive}> {lightgreen}%s {olive}已加入服务器", playerName);

	if (cv_Country.BoolValue) {
		char location[128];
		char locationParts[64];
		
		locationParts[0] = '\0';
		
		if (strlen(g_PlayerProvince[client]) > 0) {
			Format(locationParts, sizeof(locationParts), "%s", g_PlayerProvince[client]);
		}
		
		if (strlen(g_PlayerCity[client]) > 0) {
			if (strlen(locationParts) > 0 && !StrEqual(g_PlayerCity[client], g_PlayerProvince[client])) {
				Format(locationParts, sizeof(locationParts), "%s-%s", locationParts, g_PlayerCity[client]);
			} else if (strlen(locationParts) == 0) {
				Format(locationParts, sizeof(locationParts), "%s", g_PlayerCity[client]);
			}
		}
		
		if (strlen(locationParts) > 0) {
			Format(location, sizeof(location), "%s {olive}● {lightgreen}%s", g_PlayerCountry[client], locationParts);
		} else {
			Format(location, sizeof(location), "%s", g_PlayerCountry[client]);
		}
		
		CPrintToChatAll("{olive}▸ {green}地区 {olive}> {lightgreen}%s", location);
	}

	DataPack pack = new DataPack();
	pack.WriteCell(GetClientUserId(client));
	CreateTimer(3.0, Timer_PlayJoinSound, pack, TIMER_FLAG_NO_MAPCHANGE | TIMER_DATA_HNDL_CLOSE);
	showNotify[client] = true;
}

public Action Timer_PlayJoinSound(Handle timer, DataPack pack)
{
	pack.Reset();
	int userid = pack.ReadCell();
	int client = GetClientOfUserId(userid);

	PlaySound(IsConnected);

	if (client > 0 && IsClientInGame(client) && !IsFakeClient(client))
		EmitSoundToClient(client, IsConnected, SOUND_FROM_PLAYER, SNDCHAN_STATIC, SNDLEVEL_NORMAL, SND_NOFLAGS, 1.0, SNDPITCH_NORMAL);

	return Plugin_Continue;
}

public Action Event_PlayerDisconnect(Event event, const char[] name, bool dontBroadcast)
{
	event.BroadcastDisabled = true;
	int client = GetClientOfUserId(GetEventInt(event, "userid"));

	if (!(1 <= client <= MaxClients) || !IsClientInGame(client) || IsFakeClient(client))
		return Plugin_Handled;

	char reason[64], message[64];
	GetEventString(event, "reason", reason, sizeof(reason));

	if (StrContains(reason, "connection rejected", false) != -1)
		Format(message, sizeof(message), "连接被拒绝");
	else if (StrContains(reason, "timed out", false) != -1)
		Format(message, sizeof(message), "超时");
	else if (StrContains(reason, "by console", false) != -1)
		Format(message, sizeof(message), "控制台退出");
	else if (StrContains(reason, "by user", false) != -1)
		Format(message, sizeof(message), "主动断开连接");
	else if (StrContains(reason, "ping is too high", false) != -1)
		Format(message, sizeof(message), "ping太高了");
	else if (StrContains(reason, "No Steam logon", false) != -1)
		Format(message, sizeof(message), "steam验证失败/游戏闪退");
	else if (StrContains(reason, "Steam account is being used in another", false) != -1)
		Format(message, sizeof(message), "steam账号被顶");
	else if (StrContains(reason, "Steam Connection lost", false) != -1)
		Format(message, sizeof(message), "steam断线");
	else if (StrContains(reason, "This Steam account does not own this game", false) != -1)
		Format(message, sizeof(message), "家庭共享账号");
	else if (StrContains(reason, "Validation Rejected", false) != -1)
		Format(message, sizeof(message), "验证失败");
	else if (StrContains(reason, "Certificate Length", false) != -1)
		Format(message, sizeof(message), "certificate length");
	else if (StrContains(reason, "Pure server", false) != -1)
		Format(message, sizeof(message), "纯净服务器");
	else
		message = reason;

	CPrintToChatAll("{olive}▸ {green}玩家 {olive}> {lightgreen}%N {olive}已离开 {default}| {green}%s", client, message);
	PlaySound(IsDisconnect);
	showNotify[client] = false;
	return Plugin_Handled;
}

void PlaySound(const char[] sample)
{
	for (int i = 1; i <= MaxClients; i++) {
		if (IsClientInGame(i) && !IsFakeClient(i))
			EmitSoundToClient(i, sample, SOUND_FROM_PLAYER, SNDCHAN_STATIC, SNDLEVEL_NORMAL, SND_NOFLAGS, SNDVOL_NORMAL, SNDPITCH_NORMAL);
	}
}
