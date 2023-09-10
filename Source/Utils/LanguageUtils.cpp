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
	english[LanguageID::GUI_MISC_UNHOOK] = "Unhook";
	english[LanguageID::GUI_CONFIG_INCREMENTALLOAD] = "Incremental Load";
	english[LanguageID::GUI_CONFIG_NAME] = "config name";
	english[LanguageID::GUI_CONFIG_OPENDIRECTORY] = "Open config directory";
	english[LanguageID::GUI_CONFIG_CREATE] = "Create config";
	english[LanguageID::GUI_CONFIG_RESET] = "Reset config";
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
	chinese[LanguageID::GUI_MISC_UNHOOK] = "卸载";
	chinese[LanguageID::GUI_CONFIG_INCREMENTALLOAD] = "增强型加载";
	chinese[LanguageID::GUI_CONFIG_NAME] = "参数名称";
	chinese[LanguageID::GUI_CONFIG_OPENDIRECTORY] = "打开参数目录";
	chinese[LanguageID::GUI_CONFIG_CREATE] = "创建参数";
	chinese[LanguageID::GUI_CONFIG_RESET] = "重置参数";
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