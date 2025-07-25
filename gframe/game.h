#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "client_field.h"
#include "deck_con.h"
#include "menu_handler.h"
#include "CGUISkinSystem/CGUISkinSystem.h"
#include <unordered_map>
#include <vector>
#include <list>
class CGUISkinSystem;
#define DEFAULT_DUEL_RULE 5
namespace ygo {

template<size_t N>
bool IsExtension(const wchar_t* filename, const wchar_t(&extension)[N]) {
	auto flen = std::wcslen(filename);
	constexpr size_t elen = N - 1;
	if (!elen || flen < elen)
		return false;
	return !mywcsncasecmp(filename + (flen - elen), extension, elen);
}

template<size_t N>
bool IsExtension(const char* filename, const char(&extension)[N]) {
	auto flen = std::strlen(filename);
	constexpr size_t elen = N - 1;
	if (!elen || flen < elen)
		return false;
	return !mystrncasecmp(filename + (flen - elen), extension, elen);
}

struct Config {
	bool use_d3d;
	bool use_image_scale;
	unsigned short antialias;
	unsigned short serverport;
	unsigned char textfontsize;
	wchar_t lasthost[100];
	wchar_t lastport[10];
	wchar_t nickname[20];
	wchar_t gamename[20];
	wchar_t lastdeck[64];
	wchar_t textfont[256];
	wchar_t numfont[256];
	wchar_t roompass[20];
	wchar_t locale[64];
	wchar_t soundtheme[64];
	wchar_t searchname[256];
	//settings
	int chkMAutoPos;
	int chkSTAutoPos;
	int chkRandomPos;
	int chkAutoChain;
	int chkWaitChain;
	int chkIgnore1;
	int chkIgnore2;
	int default_rule;
	int chkHideSetname;
	int chkHideHintButton;
	int control_mode;
	int draw_field_spell;
	int separate_clear_button;
	int auto_search_limit;
	int search_multiple_keywords;
	int search_regex;
	int chkIgnoreDeckChanges;
	int defaultOT;
	int enable_bot_mode;
	int quick_animation;
	bool window_maximized;
	int window_width;
	int window_height;
	bool resize_popup_menu;
	int auto_save_replay;
	int ask_mset;
	bool enable_sound;
	bool enable_music;
	double sound_volume;
	double music_volume;
	int music_mode;
	int random;
	int chkEnablePScale;
	int skin_index;
};

struct DuelInfo {
	bool isStarted{ false };
	bool isInDuel{ false };
	bool isFinished{false};
	bool isReplay{ false };
	bool isReplaySkiping{ false };
	bool isFirst{ false };
	bool isTag{ false };
	bool isSingleMode{ false };
	bool is_shuffling{ false };
	bool tag_player[2]{};
	bool isReplaySwapped{ false };
	int lp[2]{};
	int start_lp{ 0 };
	int duel_rule{ 0 };
	int turn{ 0 };
	short curMsg{ 0 };
	wchar_t hostname[20]{};
	wchar_t clientname[20]{};
	wchar_t hostname_tag[20]{};
	wchar_t clientname_tag[20]{};
	wchar_t strLP[2][16]{};
	wchar_t* vic_string{ nullptr };
	unsigned char player_type{ 0 };
	unsigned char time_player{ 0 };
	unsigned short time_limit{ 0 };
	unsigned short time_left[2]{ 0 };
	int card_count[2];
	int total_attack[2];
	wchar_t str_time_left[2][16];
	video::SColor time_color[2];
	wchar_t str_card_count[2][16];
	wchar_t str_total_attack[2][16];
	video::SColor card_count_color[2];
	video::SColor total_attack_color[2];
	std::vector<unsigned int> announce_cache;
	void Clear();
};

struct BotInfo {
	wchar_t name[256];
	wchar_t command[256];
	wchar_t desc[256];
	bool support_master_rule_3;
	bool support_new_master_rule;
	bool support_master_rule_2020;
};

struct FadingUnit {
	bool signalAction;
	bool isFadein;
	int fadingFrame;
	int autoFadeoutFrame;
	irr::gui::IGUIElement* guiFading;
	irr::core::recti fadingSize;
	irr::core::vector2di fadingUL;
	irr::core::vector2di fadingLR;
	irr::core::vector2di fadingDiff;
};

class Game {

public:
	bool Initialize();
	void MainLoop();
	void RefreshTimeDisplay();
	void BuildProjectionMatrix(irr::core::matrix4& mProjection, f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar);
	void InitStaticText(irr::gui::IGUIStaticText* pControl, u32 cWidth, u32 cHeight, irr::gui::CGUITTFont* font, const wchar_t* text);
	std::wstring SetStaticText(irr::gui::IGUIStaticText* pControl, u32 cWidth, irr::gui::CGUITTFont* font, const wchar_t* text, u32 pos = 0);
	void LoadExpansions();
	void ReLoadExpansions();
	void RefreshDeck(irr::gui::IGUIComboBox* cbDeck);
	void RefreshReplay();
	void RefreshSingleplay();
	void RefreshBot();
	void RefreshLocales();
	void RefreshSoundTheme();
	void RefreshFont();
	void RefreshHDS();
	void RefreshCRS();
	void RefreshBGS();
	void DrawSelectionLine(irr::video::S3DVertex* vec, bool strip, int width, float* cv);
	void DrawSelectionLine(irr::gui::IGUIElement* element, int width, irr::video::SColor color);
	void DrawBackGround();
	void DrawLinkedZones(ClientCard* pcard);
	void DrawSpellLinkedZones(ClientCard* pcard);
	void CheckMutual(ClientCard* pcard, int mark);
	void DrawCards();
	void DrawCard(ClientCard* pcard);
	void DrawShadowText(irr::gui::CGUITTFont* font, const core::stringw& text, const core::rect<s32>& position, const core::rect<s32>& padding, video::SColor color = 0xffffffff, video::SColor shadowcolor = 0xff000000, bool hcenter = false, bool vcenter = false, const core::rect<s32>* clip = 0);
	void DrawMisc();
	void DrawStatus(ClientCard* pcard, int x1, int y1, int x2, int y2);
	void DrawGUI();
	void DrawSpec();
	void DrawBackImage(irr::video::ITexture* texture);
	void ShowElement(irr::gui::IGUIElement* element, int autoframe = 0);
	void HideElement(irr::gui::IGUIElement* element, bool set_action = false);
	void PopupElement(irr::gui::IGUIElement* element, int hideframe = 0);
	void WaitFrameSignal(int frame);
	void DrawThumb(code_pointer cp, position2di pos, const std::unordered_map<int,int>* lflist, bool drag = false);
	void DrawDeckBd();
	void LoadConfig();
	void SaveConfig();
	void ShowCardInfo(int code, bool resize = false);
	void ClearCardInfo(int player = 0);
	void AddLog(const wchar_t* msg, int param = 0);
	void AddChatMsg(const wchar_t* msg, int player, bool play_sound = false);
	void ClearChatMsg();
	void AddDebugMsg(const char* msgbuf);
	void ErrorLog(const char* msgbuf);
	void initUtils();
	void ClearTextures();
	void CloseDuelWindow();

	int LocalPlayer(int player) const;
	int OppositePlayer(int player);
	int ChatLocalPlayer(int player);
	const wchar_t* LocalName(int local_player);
	const char* GetLocaleDir(const char* dir);
	const wchar_t* GetLocaleDirWide(const char* dir);
	const char* GetsoundthemeDir(const char* dir);
	const wchar_t* GetsoundthemeDirWide(const char* dir);
	bool CheckRegEx(const std::wstring& text, const std::wstring& exp, bool exact = false);

	bool HasFocus(EGUI_ELEMENT_TYPE type) const {
		irr::gui::IGUIElement* focus = env->getFocus();
		return focus && focus->hasType(type);
	}

	void TrimText(irr::gui::IGUIElement* editbox) const {
		irr::core::stringw text(editbox->getText());
		text.trim();
		editbox->setText(text.c_str());
	}

	void OnResize();
	void ResizeChatInputWindow();
	recti Resize(s32 x, s32 y, s32 x2, s32 y2);
	recti Resize(s32 x, s32 y, s32 x2, s32 y2, s32 dx, s32 dy, s32 dx2, s32 dy2);
	position2di Resize(s32 x, s32 y);
	position2di ResizeReverse(s32 x, s32 y);
	recti ResizePhaseHint(s32 x, s32 y, s32 x2, s32 y2, s32 width);
	recti ResizeWin(s32 x, s32 y, s32 x2, s32 y2, bool chat = false);
	recti ResizeCard(s32 x, s32 y, s32 x2, s32 y2);
	recti ResizeCardHint(s32 x, s32 y, s32 x2, s32 y2);
	position2di ResizeCardHint(s32 x, s32 y);
	recti ResizeCardMid(s32 x, s32 y, s32 x2, s32 y2, s32 midx, s32 midy);
	position2di ResizeCardMid(s32 x, s32 y, s32 midx, s32 midy);
	recti ResizeFit(s32 x, s32 y, s32 x2, s32 y2);

	void SetWindowsIcon();
	void SetWindowsScale(float scale);
	void FlashWindow();
	void takeScreenshot();
	void SetCursor(ECURSOR_ICON icon);

	std::mutex gMutex;
	Signal frameSignal;
	Signal actionSignal;
	Signal replaySignal;
	Signal singleSignal;
	Signal closeSignal;
	Signal closeDoneSignal;
	Config gameConf;
	DuelInfo dInfo;

	std::list<FadingUnit> fadingList;
	std::vector<int> logParam;
	std::wstring chatMsg[8];
	std::vector<BotInfo> botInfo;

	int hideChatTimer;
	bool hideChat;
	int chatTiming[8]{};
	int chatType[8]{};
	unsigned short linePatternD3D;
	unsigned short linePatternGL;
	int waitFrame;
	int signalFrame;
	int actionParam;
	int showingcode;
	const wchar_t* showingtext;
	int showcard;
	int showcardcode;
	int showcarddif;
	int showcardp;
	int is_attacking;
	int attack_sv;
	irr::core::vector3df atk_r;
	irr::core::vector3df atk_t;
	float atkdy;
	int lpframe;
	int lpd;
	int lpplayer;
	int lpccolor;
	wchar_t* lpcstring;
	bool always_chain;
	bool ignore_chain;
	bool chain_when_avail;

	bool is_building;
	bool is_siding;

	irr::core::dimension2d<irr::u32> window_size;
	float xScale;
	float yScale;

	CGUISkinSystem *skinSystem;
	wchar_t locale_buf[256];
	wchar_t orig_dir[64];
	char locale_buf_utf8[256];
	
	ClientField dField;
	DeckBuilder deckBuilder;
	MenuHandler menuHandler;
	irr::IrrlichtDevice* device;
	irr::video::IVideoDriver* driver;
	irr::scene::ISceneManager* smgr;
	irr::scene::ICameraSceneNode* camera;

#ifdef _WIN32
	HWND hWnd;
#endif

	//GUI
	irr::gui::IGUIEnvironment* env;
	irr::gui::CGUITTFont* guiFont;
	irr::gui::CGUITTFont* textFont;
	irr::gui::CGUITTFont* numFont;
	irr::gui::CGUITTFont* adFont;
	irr::gui::CGUITTFont* lpcFont;
	std::map<irr::gui::CGUIImageButton*, int> imageLoading;
	//card image
	irr::gui::IGUIStaticText* wCardImg;
	irr::gui::IGUIImage* imgCard;
	//hint text
	irr::gui::IGUIStaticText* stHintMsg;
	irr::gui::IGUIStaticText* stTip;
	irr::gui::IGUIStaticText* stCardListTip;
	//infos
	irr::gui::IGUITabControl* wInfos;
	irr::gui::IGUIStaticText* stName;
	irr::gui::IGUIStaticText* stInfo;
	irr::gui::IGUIStaticText* stDataInfo;
	irr::gui::IGUIStaticText* stSetName;
	irr::gui::IGUIStaticText* stText;
	irr::gui::IGUIScrollBar* scrCardText;
	irr::gui::IGUIListBox* lstLog;
	irr::gui::IGUIButton* btnClearLog;
	irr::gui::IGUIButton* btnSaveLog;
	irr::gui::IGUIWindow* tabHelper;
	irr::gui::IGUIElement* elmTabHelperLast;
	irr::gui::IGUIScrollBar* scrTabHelper;
	irr::gui::IGUICheckBox* chkMAutoPos;
	irr::gui::IGUICheckBox* chkSTAutoPos;
	irr::gui::IGUICheckBox* chkRandomPos;
	irr::gui::IGUICheckBox* chkAutoChain;
	irr::gui::IGUICheckBox* chkWaitChain;
	irr::gui::IGUICheckBox* chkQuickAnimation;
	irr::gui::IGUICheckBox* chkAutoSaveReplay;
	irr::gui::IGUICheckBox* chkAskMSet;
	irr::gui::IGUIWindow* tabSystem;
	irr::gui::IGUIElement* elmTabSystemLast;
	irr::gui::IGUIScrollBar* scrTabSystem;
	irr::gui::IGUICheckBox* chkHideSetname;
	irr::gui::IGUICheckBox* chkHideHintButton;
	irr::gui::IGUICheckBox* chkIgnoreDeckChanges;
	irr::gui::IGUICheckBox* chkEnableSound;
	irr::gui::IGUICheckBox* chkEnableMusic;
	irr::gui::IGUIScrollBar* scrSoundVolume;
	irr::gui::IGUIScrollBar* scrMusicVolume;
	irr::gui::IGUICheckBox* chkMusicMode;
	irr::gui::IGUIButton* btnWinResizeS;
	irr::gui::IGUIButton* btnWinResizeM;
	irr::gui::IGUIButton* btnWinResizeL;
	irr::gui::IGUIButton* btnWinResizeXL;
	irr::gui::IGUICheckBox* chkEnablePScale;
	irr::gui::IGUIWindow* wSearchWindow;
	irr::gui::IGUIButton* btnCardSearch;
	irr::gui::IGUIButton* btnSearchAgree;
	irr::gui::IGUIButton* btnSearchCancel;
	irr::gui::IGUIEditBox* ebSearchName;
	//main menu
	irr::gui::IGUIWindow* wMainMenu;
	irr::gui::IGUIButton* btnLanMode;
	irr::gui::IGUIButton* btnSingleMode;
	irr::gui::IGUIButton* btnReplayMode;
	irr::gui::IGUIButton* btnTestMode;
	irr::gui::IGUIButton* btnDeckEdit;
	irr::gui::IGUIButton* btnModeExit;
	irr::gui::IGUIButton* btnOther;
	irr::gui::IGUIButton* btnSYS;
	//other
	irr::gui::IGUIWindow* wOther;
	irr::gui::IGUIButton* btnSEM;
	irr::gui::IGUIButton* btnTakeout1;
	irr::gui::IGUIButton* btnTakeout2;
	irr::gui::IGUIButton* btnOtherExit;
	irr::gui::IGUIButton* btnLantern;
	irr::gui::IGUIButton* btnVI;
	irr::gui::IGUIButton* btnFOX;
	irr::gui::IGUIButton* btnWBO;
	irr::gui::IGUIButton* btnDC;
	irr::gui::IGUIButton* btnDLD;
	irr::gui::IGUIButton* btnTG;
	irr::gui::IGUIButton* btnYST;
	irr::gui::IGUIButton* btnMJ;
	irr::gui::IGUIButton* btnDV;
	irr::gui::IGUIButton* btnDIY;
	//system setting
	irr::gui::IGUIWindow* wSystem;
	irr::gui::IGUICheckBox* chkMRandom;
	irr::gui::IGUICheckBox* chkSkin;
	irr::gui::IGUICheckBox* chkD3D;
	irr::gui::IGUICheckBox* chkAutoSearch;
	irr::gui::IGUICheckBox* chkMultiKeywords;
	irr::gui::IGUICheckBox* chkRegex;
	irr::gui::IGUIComboBox* cbLocale;
	irr::gui::IGUIComboBox* cbsoundtheme;
	irr::gui::IGUIComboBox* cbFont;
	irr::gui::IGUIButton* btnHeadS;
	irr::gui::IGUIButton* btnCoverS;
	irr::gui::IGUIButton* btnBgS;
	irr::gui::IGUIButton* btnSystemExit;
	//head select
	irr::gui::IGUIWindow* wHDS;
	irr::gui::IGUIComboBox* cbHDS;
	irr::gui::IGUIImage* imgHead;
	irr::gui::IGUIButton* btnHDSOK;
	irr::gui::IGUIButton* btnHDSExit;
	//cover select
	irr::gui::IGUIWindow* wCRS;
	irr::gui::IGUIComboBox* cbCRS;
	irr::gui::IGUIImage* imgCover;
	irr::gui::IGUIButton* btnCoverOK1;
	irr::gui::IGUIButton* btnCoverOK2;
	irr::gui::IGUIButton* btnCoverOK3;
	irr::gui::IGUIButton* btnCoverOK4;
	irr::gui::IGUIButton* btnCRSExit;
	//background select
	irr::gui::IGUIWindow* wBGS;
	irr::gui::IGUIComboBox* cbBGS;
	irr::gui::IGUIImage* imgBG;
	irr::gui::IGUIButton* btnBGOK1;
	irr::gui::IGUIButton* btnBGOK2;
	irr::gui::IGUIButton* btnBGOK3;
	irr::gui::IGUIButton* btnBGSExit;
	//lan
	irr::gui::IGUIWindow* wLanWindow;
	irr::gui::IGUIEditBox* ebNickName;
	irr::gui::IGUIListBox* lstHostList;
	irr::gui::IGUIButton* btnLanRefresh;
	irr::gui::IGUIButton* btnSC;
	irr::gui::IGUIButton* btnRM;
	irr::gui::IGUIEditBox* ebJoinHost;
	irr::gui::IGUIEditBox* ebJoinPort;
	irr::gui::IGUIEditBox* ebJoinPass;
	irr::gui::IGUIButton* btnJoinHost;
	irr::gui::IGUIButton* btnJoinCancel;
	irr::gui::IGUIButton* btnCreateHost;
	//server choose
	irr::gui::IGUIWindow* wSC;
	irr::gui::IGUIButton* btn233;
	irr::gui::IGUIButton* btn23333;
	irr::gui::IGUIButton* btn7210;
	irr::gui::IGUIButton* btn222;
	irr::gui::IGUIButton* btn765;
	irr::gui::IGUIButton* btnJP;
	irr::gui::IGUIButton* btnNK;
	irr::gui::IGUIButton* btnSP;
	irr::gui::IGUIButton* btn888;
	irr::gui::IGUIButton* btnSCExit;
	//room code
	irr::gui::IGUIWindow* wRM;
	irr::gui::IGUIButton* btnRM1;
	irr::gui::IGUIButton* btnRM2;
	irr::gui::IGUIButton* btnRM3;
	irr::gui::IGUIButton* btnRM4;
	irr::gui::IGUIButton* btnRM5;
	irr::gui::IGUIButton* btnRM6;
	irr::gui::IGUIButton* btnRM7;
	irr::gui::IGUIButton* btnRM8;
	irr::gui::IGUIButton* btnRM9;
	irr::gui::IGUIButton* btnRM10;
	irr::gui::IGUIButton* btnRM11;
	irr::gui::IGUIButton* btnRM12;
	irr::gui::IGUIButton* btnRM13;
	irr::gui::IGUIButton* btnRM14;
	irr::gui::IGUIButton* btnRMCM;
	irr::gui::IGUIButton* btnRMWN;
	irr::gui::IGUIButton* btnRMAI;
	irr::gui::IGUIButton* btnRMA;
	irr::gui::IGUIButton* btnRMExit;
	//create host
	irr::gui::IGUIWindow* wCreateHost;
	irr::gui::IGUIComboBox* cbLFlist;
	irr::gui::IGUIComboBox* cbMatchMode;
	irr::gui::IGUIComboBox* cbRule;
	irr::gui::IGUIEditBox* ebTimeLimit;
	irr::gui::IGUIEditBox* ebStartLP;
	irr::gui::IGUIEditBox* ebStartHand;
	irr::gui::IGUIEditBox* ebDrawCount;
	irr::gui::IGUIEditBox* ebServerName;
	irr::gui::IGUIEditBox* ebServerPass;
	irr::gui::IGUIComboBox* cbDuelRule;
	irr::gui::IGUICheckBox* chkNoCheckDeck;
	irr::gui::IGUICheckBox* chkNoShuffleDeck;
	irr::gui::IGUIButton* btnHostConfirm;
	irr::gui::IGUIButton* btnHostCancel;
	//host panel
	irr::gui::IGUIWindow* wHostPrepare;
	irr::gui::IGUIButton* btnHostPrepDuelist;
	irr::gui::IGUIButton* btnHostPrepOB;
	irr::gui::IGUIStaticText* stHostPrepDuelist[4];
	irr::gui::IGUICheckBox* chkHostPrepReady[4];
	irr::gui::IGUIButton* btnHostPrepKick[4];
	irr::gui::IGUIComboBox* cbDeckSelect;
	irr::gui::IGUIStaticText* stHostPrepRule;
	irr::gui::IGUIStaticText* stHostPrepOB;
	irr::gui::IGUIButton* btnHostPrepReady;
	irr::gui::IGUIButton* btnHostPrepNotReady;
	irr::gui::IGUIButton* btnHostPrepStart;
	irr::gui::IGUIButton* btnHostPrepCancel;
	//replay
	irr::gui::IGUIWindow* wReplay;
	irr::gui::IGUIListBox* lstReplayList;
	irr::gui::IGUIStaticText* stReplayInfo;
	irr::gui::IGUIButton* btnLoadReplay;
	irr::gui::IGUIButton* btnDeleteReplay;
	irr::gui::IGUIButton* btnRenameReplay;
	irr::gui::IGUIButton* btnReplayCancel;
	irr::gui::IGUIButton* btnExportDeck;
	irr::gui::IGUIEditBox* ebRepStartTurn;
	//single play
	irr::gui::IGUIWindow* wSinglePlay;
	irr::gui::IGUIListBox* lstBotList;
	irr::gui::IGUIStaticText* stBotInfo;
	irr::gui::IGUIButton* btnStartBot;
	irr::gui::IGUIButton* btnBotCancel;
	irr::gui::IGUIComboBox* cbBotRule;
	irr::gui::IGUICheckBox* chkBotHand;
	irr::gui::IGUICheckBox* chkBotNoCheckDeck;
	irr::gui::IGUICheckBox* chkBotNoShuffleDeck;
	irr::gui::IGUIListBox* lstSinglePlayList;
	irr::gui::IGUIStaticText* stSinglePlayInfo;
	irr::gui::IGUIButton* btnLoadSinglePlay;
	irr::gui::IGUIButton* btnSinglePlayCancel;
	//hand
	irr::gui::IGUIWindow* wHand;
	irr::gui::IGUIButton* btnHand[3];
	//
	irr::gui::IGUIWindow* wFTSelect;
	irr::gui::IGUIButton* btnFirst;
	irr::gui::IGUIButton* btnSecond;
	//message
	irr::gui::IGUIWindow* wMessage;
	irr::gui::IGUIStaticText* stMessage;
	irr::gui::IGUIButton* btnMsgOK;
	//auto close message
	irr::gui::IGUIWindow* wACMessage;
	irr::gui::IGUIStaticText* stACMessage;
	//auto center message
	irr::gui::IGUIWindow* wASMessage;
	irr::gui::IGUIStaticText* stASMessage;
	//yes/no
	irr::gui::IGUIWindow* wQuery;
	irr::gui::IGUIStaticText* stQMessage;
	irr::gui::IGUIButton* btnYes;
	irr::gui::IGUIButton* btnNo;
	//surrender yes/no
	irr::gui::IGUIWindow* wSurrender;
	irr::gui::IGUIStaticText* stSurrenderMessage;
	irr::gui::IGUIButton* btnSurrenderYes;
	irr::gui::IGUIButton* btnSurrenderNo;
	//options
	irr::gui::IGUIWindow* wOptions;
	irr::gui::IGUIStaticText* stOptions;
	irr::gui::IGUIButton* btnOptionp;
	irr::gui::IGUIButton* btnOptionn;
	irr::gui::IGUIButton* btnOptionOK;
	irr::gui::IGUIButton* btnOption[5];
	irr::gui::IGUIScrollBar* scrOption;
	//pos selection
	irr::gui::IGUIWindow* wPosSelect;
	irr::gui::CGUIImageButton* btnPSAU;
	irr::gui::CGUIImageButton* btnPSAD;
	irr::gui::CGUIImageButton* btnPSDU;
	irr::gui::CGUIImageButton* btnPSDD;
	//card selection
	irr::gui::IGUIWindow* wCardSelect;
	irr::gui::CGUIImageButton* btnCardSelect[5];
	irr::gui::IGUIStaticText *stCardPos[5];
	irr::gui::IGUIScrollBar *scrCardList;
	irr::gui::IGUIButton* btnSelectOK;
	//card display
	irr::gui::IGUIWindow* wCardDisplay;
	irr::gui::CGUIImageButton* btnCardDisplay[5];
	irr::gui::IGUIStaticText *stDisplayPos[5];
	irr::gui::IGUIScrollBar *scrDisplayList;
	irr::gui::IGUIButton* btnDisplayOK;
	//announce number
	irr::gui::IGUIWindow* wANNumber;
	irr::gui::IGUIComboBox* cbANNumber;
	irr::gui::IGUIButton* btnANNumberOK;
	//announce card
	irr::gui::IGUIWindow* wANCard;
	irr::gui::IGUIEditBox* ebANCard;
	irr::gui::IGUIListBox* lstANCard;
	irr::gui::IGUIButton* btnANCardOK;
	//announce attribute
	irr::gui::IGUIWindow* wANAttribute;
	irr::gui::IGUICheckBox* chkAttribute[7];
	//announce race
	irr::gui::IGUIWindow* wANRace;
	irr::gui::IGUICheckBox* chkRace[RACES_COUNT];
	//cmd menu
	irr::gui::IGUIWindow* wCmdMenu;
	irr::gui::IGUIButton* btnActivate;
	irr::gui::IGUIButton* btnSummon;
	irr::gui::IGUIButton* btnSPSummon;
	irr::gui::IGUIButton* btnMSet;
	irr::gui::IGUIButton* btnSSet;
	irr::gui::IGUIButton* btnRepos;
	irr::gui::IGUIButton* btnAttack;
	irr::gui::IGUIButton* btnShowList;
	irr::gui::IGUIButton* btnOperation;
	irr::gui::IGUIButton* btnReset;
	irr::gui::IGUIButton* btnShuffle;
	//chat window
	irr::gui::IGUIWindow* wChat;
	irr::gui::IGUIListBox* lstChatLog;
	irr::gui::IGUIEditBox* ebChatInput;
	irr::gui::IGUICheckBox* chkIgnore1;
	irr::gui::IGUICheckBox* chkIgnore2;
	//phase button
	irr::gui::IGUIStaticText* wPhase;
	irr::gui::IGUIButton* btnPhaseStatus;
	irr::gui::IGUIButton* btnBP;
	irr::gui::IGUIButton* btnM2;
	irr::gui::IGUIButton* btnEP;
	//deck edit
	irr::gui::IGUIStaticText* wDeckEdit;
	irr::gui::IGUIComboBox* cbDBLFList;
	irr::gui::IGUIComboBox* cbDBDecks;
	irr::gui::IGUIButton* btnClearDeck;
	irr::gui::IGUIButton* btnSortDeck;
	irr::gui::IGUIButton* btnShuffleDeck;
	irr::gui::IGUIButton* btnSaveDeck;
	irr::gui::IGUIButton* btnDeleteDeck;
	irr::gui::IGUIButton* btnSaveDeckAs;
	irr::gui::IGUIButton* btnSideOK;
	irr::gui::IGUIButton* btnSideShuffle;
	irr::gui::IGUIButton* btnSideSort;
	irr::gui::IGUIButton* btnSideReload;
	irr::gui::IGUIEditBox* ebDeckname;
	irr::gui::IGUIStaticText* stBanlist;
	irr::gui::IGUIStaticText* stDeck;
	irr::gui::IGUIStaticText* stCategory;
	irr::gui::IGUIStaticText* stLimit;
	irr::gui::IGUIStaticText* stAttribute;
	irr::gui::IGUIStaticText* stRace;
	irr::gui::IGUIStaticText* stAttack;
	irr::gui::IGUIStaticText* stDefense;
	irr::gui::IGUIStaticText* stStar;
	irr::gui::IGUIStaticText* stSearch;
	irr::gui::IGUIStaticText* stScale;
	irr::gui::IGUIButton* btnRenameDeck;
	//deck rename
	irr::gui::IGUIWindow* wRenameDeck;
	irr::gui::IGUIEditBox* ebREName;
	irr::gui::IGUIButton* btnREYes;
	irr::gui::IGUIButton* btnRENo;
	//filter
	irr::gui::IGUIStaticText* wFilter;
	irr::gui::IGUIScrollBar* scrFilter;
	irr::gui::IGUIComboBox* cbCardType;
	irr::gui::IGUIComboBox* cbCardType2;
	irr::gui::IGUIComboBox* cbRace;
	irr::gui::IGUIComboBox* cbAttribute;
	irr::gui::IGUIComboBox* cbLimit;
	irr::gui::IGUIEditBox* ebStar;
	irr::gui::IGUIEditBox* ebScale;
	irr::gui::IGUIEditBox* ebAttack;
	irr::gui::IGUIEditBox* ebDefense;
	irr::gui::IGUIEditBox* ebCardName;
	irr::gui::IGUIButton* btnEffectFilter;
	irr::gui::IGUIButton* btnStartFilter;
	irr::gui::IGUIButton* btnClearFilter;
	irr::gui::IGUIWindow* wCategories;
	irr::gui::IGUICheckBox* chkCategory[32];
	irr::gui::IGUIButton* btnCategoryOK;
	irr::gui::IGUIButton* btnMarksFilter;
	irr::gui::IGUIWindow* wLinkMarks;
	irr::gui::IGUIButton* btnMark[8];
	irr::gui::IGUIButton* btnMarksOK;
	//sort type
	irr::gui::IGUIStaticText* wSort;
	irr::gui::IGUIComboBox* cbSortType;
	//replay save
	irr::gui::IGUIWindow* wReplaySave;
	irr::gui::IGUIEditBox* ebRSName;
	irr::gui::IGUIButton* btnRSYes;
	irr::gui::IGUIButton* btnRSNo;
	//replay control
	irr::gui::IGUIStaticText* wReplayControl;
	irr::gui::IGUIButton* btnReplayStart;
	irr::gui::IGUIButton* btnReplayPause;
	irr::gui::IGUIButton* btnReplayStep;
	irr::gui::IGUIButton* btnReplayUndo;
	irr::gui::IGUIButton* btnReplayExit;
	irr::gui::IGUIButton* btnReplaySwap;
	//surrender/leave
	irr::gui::IGUIButton* btnLeaveGame;
	//swap
	irr::gui::IGUIButton* btnSpectatorSwap;
	//chain control
	irr::gui::IGUIButton* btnChainIgnore;
	irr::gui::IGUIButton* btnChainAlways;
	irr::gui::IGUIButton* btnChainWhenAvail;
	//cancel or finish
	irr::gui::IGUIButton* btnCancelOrFinish;
	//fortune
	irr::gui::IGUIWindow* wDV;
	irr::gui::IGUIImage* imgDV;
	irr::gui::IGUIButton* btnDVClose;
};

extern Game* mainGame;

}

#define SIZE_QUERY_BUFFER	0x4000

#define CARD_IMG_WIDTH		177
#define CARD_IMG_HEIGHT		254
#define CARD_THUMB_WIDTH	44
#define CARD_THUMB_HEIGHT	64

#define UEVENT_EXIT			0x1
#define UEVENT_TOWINDOW		0x2

#define COMMAND_ACTIVATE	0x0001
#define COMMAND_SUMMON		0x0002
#define COMMAND_SPSUMMON	0x0004
#define COMMAND_MSET		0x0008
#define COMMAND_SSET		0x0010
#define COMMAND_REPOS		0x0020
#define COMMAND_ATTACK		0x0040
#define COMMAND_LIST		0x0080
#define COMMAND_OPERATION	0x0100
#define COMMAND_RESET		0x0200

#define POSITION_HINT		0x8000

#define BUTTON_LAN_MODE				100
#define BUTTON_SINGLE_MODE			101
#define BUTTON_REPLAY_MODE			102
#define BUTTON_TEST_MODE			103
#define BUTTON_DECK_EDIT			104
#define BUTTON_MODE_EXIT			105
#define LISTBOX_LAN_HOST			110
#define BUTTON_JOIN_HOST			111
#define BUTTON_JOIN_CANCEL			112
#define BUTTON_CREATE_HOST			113
#define BUTTON_HOST_CONFIRM			114
#define BUTTON_HOST_CANCEL			115
#define BUTTON_LAN_REFRESH			116
#define BUTTON_HP_DUELIST			120
#define BUTTON_HP_OBSERVER			121
#define BUTTON_HP_START				122
#define BUTTON_HP_CANCEL			123
#define BUTTON_HP_KICK				124
#define CHECKBOX_HP_READY			125
#define BUTTON_HP_READY				126
#define BUTTON_HP_NOTREADY			127
#define LISTBOX_REPLAY_LIST			130
#define BUTTON_LOAD_REPLAY			131
#define BUTTON_CANCEL_REPLAY		132
#define BUTTON_DELETE_REPLAY		133
#define BUTTON_RENAME_REPLAY		134
#define BUTTON_EXPORT_DECK			135
#define EDITBOX_CHAT				140
#define BUTTON_MSG_OK				200
#define BUTTON_YES					201
#define BUTTON_NO					202
#define BUTTON_HAND1				205
#define BUTTON_HAND2				206
#define BUTTON_HAND3				207
#define BUTTON_FIRST				208
#define BUTTON_SECOND				209
#define BUTTON_POS_AU				210
#define BUTTON_POS_AD				211
#define BUTTON_POS_DU				212
#define BUTTON_POS_DD				213
#define BUTTON_OPTION_PREV			220
#define BUTTON_OPTION_NEXT			221
#define BUTTON_OPTION_OK			222
#define BUTTON_OPTION_0				223
#define BUTTON_OPTION_1				224
#define BUTTON_OPTION_2				225
#define BUTTON_OPTION_3				226
#define BUTTON_OPTION_4				227
#define SCROLL_OPTION_SELECT		228
#define BUTTON_CARD_0				230
#define BUTTON_CARD_1				231
#define BUTTON_CARD_2				232
#define BUTTON_CARD_3				233
#define BUTTON_CARD_4				234
#define SCROLL_CARD_SELECT			235
#define BUTTON_CARD_SEL_OK			236
#define TEXT_CARD_LIST_TIP			237
#define BUTTON_CMD_ACTIVATE			240
#define BUTTON_CMD_SUMMON			241
#define BUTTON_CMD_SPSUMMON			242
#define BUTTON_CMD_MSET				243
#define BUTTON_CMD_SSET				244
#define BUTTON_CMD_REPOS			245
#define BUTTON_CMD_ATTACK			246
#define BUTTON_CMD_SHOWLIST			247
#define BUTTON_CMD_SHUFFLE			248
#define BUTTON_CMD_RESET			249
#define BUTTON_ANNUMBER_OK			250
#define BUTTON_ANCARD_OK			251
#define EDITBOX_ANCARD				252
#define LISTBOX_ANCARD				253
#define CHECK_ATTRIBUTE				254
#define CHECK_RACE					255
#define BUTTON_BP					260
#define BUTTON_M2					261
#define BUTTON_EP					262
#define BUTTON_LEAVE_GAME			263
#define BUTTON_CHAIN_IGNORE			264
#define BUTTON_CHAIN_ALWAYS			265
#define BUTTON_CHAIN_WHENAVAIL		266
#define BUTTON_CANCEL_OR_FINISH		267
#define BUTTON_PHASE				268
#define BUTTON_CLEAR_LOG			270
#define LISTBOX_LOG					271
#define SCROLL_CARDTEXT				280
#define BUTTON_DISPLAY_0			290
#define BUTTON_DISPLAY_1			291
#define BUTTON_DISPLAY_2			292
#define BUTTON_DISPLAY_3			293
#define BUTTON_DISPLAY_4			294
#define SCROLL_CARD_DISPLAY			295
#define BUTTON_CARD_DISP_OK			296
#define BUTTON_SURRENDER_YES		297
#define BUTTON_SURRENDER_NO			298
#define BUTTON_CATEGORY_OK			300
#define COMBOBOX_DBLFLIST			301
#define COMBOBOX_DBDECKS			302
#define BUTTON_CLEAR_DECK			303
#define BUTTON_SAVE_DECK			304
#define BUTTON_SAVE_DECK_AS			305
#define BUTTON_DELETE_DECK			306
#define BUTTON_SIDE_RELOAD			307
#define BUTTON_SORT_DECK			308
#define BUTTON_SIDE_OK				309
#define BUTTON_SHUFFLE_DECK			310
#define COMBOBOX_MAINTYPE			311
#define COMBOBOX_SECONDTYPE			312
#define BUTTON_EFFECT_FILTER		313
#define BUTTON_START_FILTER			314
#define SCROLL_FILTER				315
#define EDITBOX_KEYWORD				316
#define BUTTON_CLEAR_FILTER			317
#define COMBOBOX_ATTRIBUTE			318
#define COMBOBOX_RACE				319
#define BUTTON_REPLAY_START			320
#define BUTTON_REPLAY_PAUSE			321
#define BUTTON_REPLAY_STEP			322
#define BUTTON_REPLAY_UNDO			323
#define BUTTON_REPLAY_EXIT			324
#define BUTTON_REPLAY_SWAP			325
#define BUTTON_REPLAY_SAVE			330
#define BUTTON_REPLAY_CANCEL		331
#define EDITBOX_INPUTS				332
#define BUTTON_BOT_START			340
#define LISTBOX_BOT_LIST			341
#define COMBOBOX_BOT_RULE			342
#define LISTBOX_SINGLEPLAY_LIST		343
#define BUTTON_LOAD_SINGLEPLAY		344
#define BUTTON_CANCEL_SINGLEPLAY	345
#define SCROLL_TAB_HELPER			350
#define SCROLL_TAB_SYSTEM			351
#define CHECKBOX_MULTI_KEYWORDS		352
#define CHECKBOX_REGEX				353
#define CHECKBOX_AUTO_SEARCH		360

#define CHECKBOX_ENABLE_SOUND		361
#define CHECKBOX_ENABLE_MUSIC		362
#define SCROLL_VOLUME				363
#define CHECKBOX_QUICK_ANIMATION	364
#define CHECKBOX_RDM				365
#define CHECKBOX_SKIN				366
#define CHECKBOX_D3D				367
#define CHECKBOX_DISABLE_CHAT		368

#define COMBOBOX_SORTTYPE			370
#define COMBOBOX_LIMIT				371
#define COMBOBOX_FONT				372
#define COMBOBOX_LOCALE				373
#define COMBOBOX_SOUNDTHEME			374
#define CHECKBOX_ASK_MSET			375

#define BUTTON_MARKS_FILTER			380
#define BUTTON_MARKERS_OK			381

#define BUTTON_CARD_SEARCH			382
#define BUTTON_SEARCH_AGREE			383
#define BUTTON_SEARCH_CANCEL		384

#define BUTTON_OTHER			        385
#define BUTTON_RENAME_DECK				386
#define BUTTON_RENAME_DECK_SAVE			387
#define BUTTON_RENAME_DECK_CANCEL		388

#define BUTTON_SEM						389
#define BUTTON_TAKEOUT1					390
#define BUTTON_TAKEOUT2					391
#define BUTTON_LANTERN					392
#define BUTTON_OTHER_EXIT				393
#define BUTTON_VI						403
#define BUTTON_FOX						404
#define BUTTON_WBO						405
#define BUTTON_TG						429
#define BUTTON_DC						430
#define BUTTON_DLD						431
#define BUTTON_YST						432
#define BUTTON_MJ						433
#define BUTTON_DV						434
#define BUTTON_DV_CLOSE					435
#define BUTTON_DIYHEAD					436


#define BUTTON_SERVER_CHOOSE			394
#define BUTTON_233						395
#define BUTTON_23333					396
#define BUTTON_7210						397
#define BUTTON_222						398
#define BUTTON_765						399
#define BUTTON_JP						400
#define BUTTON_NK						401
#define BUTTON_SC_EXIT					402
#define BUTTON_SP						521
#define BUTTON_888						522

#define BUTTON_ROOM_CODE				501
#define BUTTON_RM1						502
#define BUTTON_RM2						503
#define BUTTON_RM3						504
#define BUTTON_RM4						505
#define BUTTON_RM5						506
#define BUTTON_RM6						507
#define BUTTON_RM7						508
#define BUTTON_RM8						509
#define BUTTON_RM9						510
#define BUTTON_RM10						511
#define BUTTON_RM11						512
#define BUTTON_RM12						513
#define BUTTON_RM13						514
#define BUTTON_RM14						515
#define BUTTON_RMCM						516
#define BUTTON_RMWN						517
#define BUTTON_RMAI						518
#define BUTTON_RMA						519
#define BUTTON_RM_EXIT					520

#define BUTTON_SYS						406
#define BUTTON_SYS_EXIT					407

#define BUTTON_WINDOW_RESIZE_S		408
#define BUTTON_WINDOW_RESIZE_M		409
#define BUTTON_WINDOW_RESIZE_L		410
#define BUTTON_WINDOW_RESIZE_XL		411
#define BUTTON_HDS		412
#define BUTTON_CRS		413
#define BUTTON_BGS		414

//head select
#define BUTTON_HDS_OK		415
#define BUTTON_HDS_EXIT		416
#define COMBOBOX_HDS		417

//cover select
#define BUTTON_CRS_OK1		418
#define BUTTON_CRS_OK2		419
#define BUTTON_CRS_OK3		420
#define BUTTON_CRS_OK4		421
#define BUTTON_CRS_EXIT		422
#define COMBOBOX_CRS		423

//bg select
#define BUTTON_BGS_OK1		424
#define BUTTON_BGS_OK2		425
#define BUTTON_BGS_OK3		426
#define BUTTON_BGS_EXIT		427
#define COMBOBOX_BGS		428

#define TEXTURE_DUEL				0
#define TEXTURE_DECK				1
#define TEXTURE_MENU				2
#define TEXTURE_COVER_S				3
#define TEXTURE_COVER_O				4
#define TEXTURE_COVER_S2			5
#define TEXTURE_COVER_O2			6
#define TEXTURE_ATTACK				7
#define TEXTURE_ACTIVATE			8
#define TEXTURE_AVATAR_S			9
#define TEXTURE_DV					11

#define DEFAULT_DUEL_RULE			5
//STOC_GAME_MSG messages
#define MSG_WAITING				3
#define MSG_START				4
#define MSG_UPDATE_DATA			6	// flag=0: clear
#define MSG_UPDATE_CARD			7	// flag=QUERY_CODE, code=0: clear
#define MSG_REQUEST_DECK		8
#define MSG_REFRESH_DECK		34
#define MSG_CARD_SELECTED		80
#define MSG_UNEQUIP				95
#define MSG_BE_CHAIN_TARGET		121
#define MSG_CREATE_RELATION		122
#define MSG_RELEASE_RELATION	123

#define AVAIL_OCG					0x1
#define AVAIL_TCG					0x2
#define AVAIL_CUSTOM				0x4
#define AVAIL_SC					0x8
#define AVAIL_OCGTCG				(AVAIL_OCG|AVAIL_TCG)
#define CARD_ARTWORK_VERSIONS_OFFSET	10
#define MAX_LAYER_COUNT	6
extern bool delay_swap;
extern int swap_player;
#endif // GAME_H
