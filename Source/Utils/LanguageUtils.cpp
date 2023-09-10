#include "LanguageUtils.h"

#include <map>

#include "../Config.h"

static std::map<LanguageID, std::string> english;
static std::map<LanguageID, std::string> chinese;
static std::map<LanguageID, std::string> russian;
static std::map<LanguageID, std::string> japanese;

void initEnglish()
{

	english[LanguageID::GUI_GLOBAL_ENABLED] = "Enabled";

	english[LanguageID::GUI_TAB_AIMBOT] = "Aimbot";
	english[LanguageID::GUI_TAB_ANTIAIM] = "Anti Aim";
	english[LanguageID::GUI_TAB_TRIGGERBOT] = "Triggerbot";
	english[LanguageID::GUI_TAB_BACKTRACK] = "Backtrack";
	english[LanguageID::GUI_TAB_GLOW] = "Glow";
	english[LanguageID::GUI_TAB_CHAMS] = "Chams";
	english[LanguageID::GUI_TAB_ESP] = "ESP";
	english[LanguageID::GUI_TAB_VISUALS] = "Visuals";
	english[LanguageID::GUI_TAB_PROFILECHANGER] = "Profile Changer";
	english[LanguageID::GUI_TAB_INVENTORYCHANGER] = "Inventory Changer";
	english[LanguageID::GUI_TAB_SOUND] = "Sound";
	english[LanguageID::GUI_TAB_STYLE] = "Style";
	english[LanguageID::GUI_TAB_MISC] = "Misc";
	english[LanguageID::GUI_TAB_CONFIG] = "Config";
	english[LanguageID::GUI_INVENTORYCHANGER_ADDITEM] = "Add items...";
	english[LanguageID::GUI_INVENTORYCHANGER_FORCEUPDATE] = "Force Update";
	english[LanguageID::GUI_INVENTORYCHANGER_BACK] = "Back";
	english[LanguageID::GUI_INVENTORYCHANGER_SEARCH] = "Search weapon skins, stickers, knives, gloves, music kits...";
	english[LanguageID::GUI_INVENTORYCHANGER_ADDALL] = "Add all in list";
	english[LanguageID::GUI_INVENTORYCHANGER_ADD] = "Add";
	english[LanguageID::GUI_MISC_UNHOOK] = "Unhook";
	english[LanguageID::GUI_CONFIG_INCREMENTALLOAD] = "Incremental Load";
	english[LanguageID::GUI_CONFIG_NAME] = "config name";
	english[LanguageID::GUI_CONFIG_OPENDIRECTORY] = "Open config directory";
	english[LanguageID::GUI_CONFIG_CREATE] = "Create config";
	english[LanguageID::GUI_CONFIG_RESET] = "Reset config";
	english[LanguageID::GUI_CONFIG_LOAD] = "Load selected";
	english[LanguageID::GUI_CONFIG_SAVE] = "Save selected";
	english[LanguageID::GUI_CONFIG_DELETE] = "Delete selected";
}

void initChinese()
{

	chinese[LanguageID::GUI_GLOBAL_ENABLED] = "开启";

	chinese[LanguageID::GUI_TAB_AIMBOT] = "自动瞄准";
	chinese[LanguageID::GUI_TAB_ANTIAIM] = "反瞄准";
	chinese[LanguageID::GUI_TAB_TRIGGERBOT] = "自动扳机";
	chinese[LanguageID::GUI_TAB_BACKTRACK] = "回溯";
	chinese[LanguageID::GUI_TAB_GLOW] = "人物发光";
	chinese[LanguageID::GUI_TAB_CHAMS] = "人物上色";
	chinese[LanguageID::GUI_TAB_ESP] = "透视";
	chinese[LanguageID::GUI_TAB_VISUALS] = "视觉类";
	chinese[LanguageID::GUI_TAB_PROFILECHANGER] = "段位修改器";
	chinese[LanguageID::GUI_TAB_INVENTORYCHANGER] = "库存修改器";
	chinese[LanguageID::GUI_TAB_SOUND] = "听觉类";
	chinese[LanguageID::GUI_TAB_STYLE] = "主题";
	chinese[LanguageID::GUI_TAB_MISC] = "杂项";
	chinese[LanguageID::GUI_TAB_CONFIG] = "配置";
	chinese[LanguageID::GUI_INVENTORYCHANGER_ADDITEM] = "添加物品...";
	chinese[LanguageID::GUI_INVENTORYCHANGER_FORCEUPDATE] = "强制更新";
	chinese[LanguageID::GUI_INVENTORYCHANGER_BACK] = "返回";
	chinese[LanguageID::GUI_INVENTORYCHANGER_SEARCH] = "搜索武器皮肤, 印花, 刀, 手套, 音乐盒等...";
	chinese[LanguageID::GUI_INVENTORYCHANGER_ADDALL] = "添加所有物品";
	chinese[LanguageID::GUI_INVENTORYCHANGER_ADD] = "添加";
	chinese[LanguageID::GUI_MISC_UNHOOK] = "卸载";
	chinese[LanguageID::GUI_CONFIG_INCREMENTALLOAD] = "增强型加载";
	chinese[LanguageID::GUI_CONFIG_NAME] = "参数名称";
	chinese[LanguageID::GUI_CONFIG_OPENDIRECTORY] = "打开参数目录";
	chinese[LanguageID::GUI_CONFIG_CREATE] = "创建参数";
	chinese[LanguageID::GUI_CONFIG_RESET] = "重置参数";
	chinese[LanguageID::GUI_CONFIG_LOAD] = "加载选中";
	chinese[LanguageID::GUI_CONFIG_SAVE] = "保存选中";
	chinese[LanguageID::GUI_CONFIG_DELETE] = "删除选中";
}

void Language::init() noexcept
{
	initEnglish();
	initChinese();
}

const char* Language::getText(LanguageID textID, LanguageType language) noexcept
{
	if (language == LanguageType::Chinese)
		return chinese.at(textID).c_str();
	return english.at(textID).c_str(); // Default
}

const char* Language::getText(LanguageID textID) noexcept
{
	return getText(textID, static_cast<LanguageType>(config->style.menuLanguage));
}