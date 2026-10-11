#include <array>
#include "config.h"
#include "deck_con.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "image_manager.h"
#include "sound_manager.h"
#include "game.h"
#include "builtin_browser.h"
#include "duelclient.h"
#include "ygoda_deck_code.h"

#ifdef WIN32
#include <windows.h>
#include <winuser.h>
#endif

namespace ygo {

static int parse_filter(const wchar_t* pstr, unsigned int* type) {
	if(*pstr == L'=') {
		*type = 1;
		return BufferIO::GetVal(pstr + 1);
	} else if(*pstr >= L'0' && *pstr <= L'9') {
		*type = 1;
		return BufferIO::GetVal(pstr);
	} else if(*pstr == L'>') {
		if(*(pstr + 1) == L'=') {
			*type = 2;
			return BufferIO::GetVal(pstr + 2);
		} else {
			*type = 3;
			return BufferIO::GetVal(pstr + 1);
		}
	} else if(*pstr == L'<') {
		if(*(pstr + 1) == L'=') {
			*type = 4;
			return BufferIO::GetVal(pstr + 2);
		} else {
			*type = 5;
			return BufferIO::GetVal(pstr + 1);
		}
	} else if(*pstr == L'?') {
		*type = 6;
		return 0;
	}
	*type = 0;
	return 0;
}


DeckBuilder::DeckBuilder() {
	std::random_device rd;
	std::array<uint32_t, 8> seed{};
	for (auto& x : seed)
		x = rd();
	std::seed_seq seq(seed.begin(), seed.end());
	rnd.seed(seq);
}
void DeckBuilder::Initialize() {
	mainGame->is_building = true;
	mainGame->is_siding = false;
	mainGame->ClearCardInfo();
	mainGame->wInfos->setVisible(true);
	mainGame->wCardImg->setVisible(true);
	mainGame->wDeckEdit->setVisible(true);
	mainGame->wFilter->setVisible(true);
	mainGame->wSort->setVisible(true);
	mainGame->btnLeaveGame->setVisible(true);
	mainGame->btnLeaveGame->setText(dataManager.GetSysString(1306));
	mainGame->btnSideOK->setVisible(false);
	mainGame->btnSideShuffle->setVisible(false);
	mainGame->btnSideSort->setVisible(false);
	mainGame->btnSideReload->setVisible(false);
	filterList = &deckManager._lfList[0].content;
	mainGame->cbDBLFList->setSelected(0);
	ClearSearch();
	mouse_pos.set(0, 0);
	hovered_code = 0;
	hovered_pos = 0;
	hovered_seq = -1;
	is_lastcard = 0;
	is_draging = false;
	is_starting_dragging = false;
	mainGame->RefreshDeck(mainGame->cbDBDecks);
	prev_category = mainGame->current_category;
	prev_deck = mainGame->cbDBDecks->getSelected();
	prev_operation = 0;
	prev_sel = -1;
	is_modified = false;
	mainGame->device->setEventReceiver(this);
}
void DeckBuilder::Terminate() {
	mainGame->is_building = false;
	mainGame->ClearCardInfo();
	mainGame->wDeckEdit->setVisible(false);
	mainGame->wCategories->setVisible(false);
	mainGame->wFilter->setVisible(false);
	mainGame->wSort->setVisible(false);
	mainGame->wCardImg->setVisible(false);
	mainGame->wInfos->setVisible(false);
	mainGame->btnLeaveGame->setVisible(false);
	mainGame->ResizeChatInputWindow();
	mainGame->PopupElement(mainGame->wMainMenu);
	mainGame->device->setEventReceiver(&mainGame->menuHandler);
	mainGame->wACMessage->setVisible(false);
	mainGame->ClearTextures();
	mainGame->showingcode = 0;
	mainGame->scrFilter->setVisible(false);
	int sel = mainGame->cbDBDecks->getSelected();
	if(sel >= 0)
		BufferIO::CopyWStr(mainGame->cbDBDecks->getItem(sel), mainGame->gameConf.lastdeck, 64);
	if(exit_on_return)
		mainGame->device->closeDevice();
}
void DeckBuilder::ApplyCategory(int catesel) {
	mainGame->current_category = catesel;
	if(catesel >= 4)
		current_category_name = mainGame->lstCategories->getListItem(catesel);
	else
		current_category_name.clear();
	mainGame->RefreshDeck(mainGame->cbDBDecks);
	mainGame->RefreshDeckList();
	mainGame->cbDBDecks->setSelected(0);
	mainGame->lstDecks->setSelected(0);
	prev_category = catesel;
	prev_deck = 0;
	is_modified = false;
	ApplyDeck(0);
	
}

void DeckBuilder::ApplyDeck(int idx) {
	if(idx < 0)
		return;
	const wchar_t* dname = mainGame->lstDecks->getListItem(idx);
	if(!dname)
		return;
	int cate = mainGame->current_category;
	mainGame->cbDBDecks->setSelected(idx);
	const wchar_t* cname = L"";
	// custom categories live in ./deck/<name>; NONE (unclassified) uses the ./deck root
	if(cate >= 4) {
		cname = current_category_name.c_str();
	}
	deckManager.LoadDeck(cate, cname, dname);
	prev_deck = idx;
	is_modified = false;
	
}

bool DeckBuilder::NewDeck(const wchar_t* name) {
	Deck d{};
	int cate = mainGame->current_category;
	const wchar_t* cname = L"";
	// custom categories live in ./deck/<name>; NONE (unclassified) uses the ./deck root
	if(cate >= 4 && !current_category_name.empty()) {
		cname = current_category_name.c_str();
	}
	return deckManager.SaveDeck(d, cate, cname, name);
}


void DeckBuilder::ImportDeckCodeFromClipboard() {
				// Show the deck code input window
				mainGame->wDeckCodeQuery->setText(L"导入卡组码");
				mainGame->SetStaticText(mainGame->stDeckCodeMessage, 270, mainGame->guiFont, L"请输入卡组码：");
				mainGame->ebDeckCode->setText(L"");
				mainGame->ShowElement(mainGame->wDeckCodeQuery);
				mainGame->env->setFocus(mainGame->ebDeckCode);
				return;
}

bool DeckBuilder::ImportDeckCodeFromInput(const std::wstring& deck_code_input) {
				if(deck_code_input.empty()) {
					// Input is empty
					mainGame->stACMessage->setText(L"输入不能为空");
					mainGame->PopupElement(mainGame->wACMessage, 20);
					return false;
				}

				// Convert to UTF-8 for parsing
				std::string utf8_content;
				int len = WideCharToMultiByte(CP_UTF8, 0, deck_code_input.c_str(), -1, NULL, 0, NULL, NULL);
				if(len > 1) {
					utf8_content.resize(len - 1);
					WideCharToMultiByte(CP_UTF8, 0, deck_code_input.c_str(), -1, &utf8_content[0], len, NULL, NULL);
				}

				// Try to decode the deck code
				std::vector<int> main, ex, side;
				if(DeckCode_Decode(utf8_content, main, ex, side)) {
					// Successfully decoded, now extract deck name from URI if possible
					std::wstring deck_name = L"";
					
					std::wstring uri_content = deck_code_input;
					if(uri_content.find(L"http://") != std::wstring::npos || uri_content.find(L"https://") != std::wstring::npos) {
						// Extract name parameter from URL
						size_t name_pos = uri_content.find(L"name=");
						if(name_pos != std::wstring::npos) {
							name_pos += 5; // length of "name="
							size_t next_param = uri_content.find(L"&", name_pos);
							if(next_param != std::wstring::npos) {
								deck_name = uri_content.substr(name_pos, next_param - name_pos);
							} else {
								deck_name = uri_content.substr(name_pos);
							}
							
							std::string name_utf8;
							auto HexVal = [](wchar_t c) -> int {
								if(c >= L'0' && c <= L'9') return (int)(c - L'0');
								if(c >= L'a' && c <= L'f') return (int)(c - L'a') + 10;
								if(c >= L'A' && c <= L'F') return (int)(c - L'A') + 10;
								return -1;
							};
							for(size_t i = 0; i < deck_name.length(); ++i) {
								wchar_t ch = deck_name[i];
								if(ch == L'%' && i + 2 < deck_name.length()) {
									int hi = HexVal(deck_name[i + 1]);
									int lo = HexVal(deck_name[i + 2]);
									if(hi >= 0 && lo >= 0) {
										name_utf8.push_back((char)((hi << 4) | lo));
										i += 2;
										continue;
									}
									name_utf8.push_back('%');
								} else if(ch == L'+') {
									name_utf8.push_back(' ');
								} else if(ch < 0x80) {
									name_utf8.push_back((char)ch);
								} else {
									char tmp[8]{};
									int n = WideCharToMultiByte(CP_UTF8, 0, &deck_name[i], 1, tmp, sizeof(tmp) - 1, NULL, NULL);
									if(n > 0) name_utf8.append(tmp, n);
								}
							}
							std::wstring decoded_name;
							if(!name_utf8.empty()) {
								int wn = MultiByteToWideChar(CP_UTF8, 0, name_utf8.data(), (int)name_utf8.size(), NULL, 0);
								if(wn > 0) {
									decoded_name.resize(wn);
									MultiByteToWideChar(CP_UTF8, 0, name_utf8.data(), (int)name_utf8.size(), &decoded_name[0], wn);
								}
							}
							if(!decoded_name.empty())
								deck_name = decoded_name;

						}
					}
					
					// If no name extracted, generate one
					if(deck_name.empty() || deck_name == L"") {
						time_t now = time(0);
						tm* ltm = localtime(&now);
						wchar_t gen_name[64];
						myswprintf(gen_name, L"导入_%04d%02d%02d_%02d%02d%02d", 
							1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
							ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
						deck_name = gen_name;
					}
					
					// Sanitize deck name (remove invalid file name characters)
					for(size_t i = 0; i < deck_name.length(); i++) {
						if(deck_name[i] == L'<' || deck_name[i] == L'>' || deck_name[i] == L':' ||
						   deck_name[i] == L'"' || deck_name[i] == L'/' || deck_name[i] == L'\\' ||
						   deck_name[i] == L'|' || deck_name[i] == L'?' || deck_name[i] == L'*') {
							deck_name[i] = L'_';
						}
					}
					
					int cate = mainGame->current_category;
					const wchar_t* cname = L"";
					if(cate >= 4) {
						cname = current_category_name.c_str();
					} else if(cate != ygo::DECK_CATEGORY_NONE) {
						cate = ygo::DECK_CATEGORY_NONE;   // PACK/BOT/SEP -> ./deck root
					}
					wchar_t catepath[256]{};
					DeckManager::GetCategoryPath(catepath, cate, cname);
					if(!FileSystem::IsDirExists(catepath) && !FileSystem::MakeDir(catepath)) {
						mainGame->stACMessage->setText(L"导入失败：无法创建分类目录");
						mainGame->PopupElement(mainGame->wACMessage, 20);
						return false;
					}
					DeckArray arr;
					arr.main.assign(main.begin(), main.end());
					arr.extra.assign(ex.begin(), ex.end());
					arr.side.assign(side.begin(), side.end());
					wchar_t filepath[256]{};
					myswprintf(filepath, L"%ls/%ls.ydk", catepath, deck_name.c_str());
					if(DeckManager::SaveDeckArray(arr, filepath)) {
						wchar_t msg[256];
						myswprintf(msg, L"导入成功：%ls", deck_name.c_str());
						mainGame->stACMessage->setText(msg);
						mainGame->PopupElement(mainGame->wACMessage, 20);
						mainGame->RefreshDeckManage();
						return true;
					} else {
						mainGame->stACMessage->setText(L"导入失败：写入卡组文件失败");
						mainGame->PopupElement(mainGame->wACMessage, 20);
					}
				} else {
					// Failed to decode
					mainGame->stACMessage->setText(L"未识别到卡组码");
					mainGame->PopupElement(mainGame->wACMessage, 20);
				}
				return false;
			
}

bool DeckBuilder::OnEvent(const irr::SEvent& event) {
	if(event.EventType == irr::EET_GUI_EVENT
	   && event.GUIEvent.EventType == irr::gui::EGET_BUTTON_CLICKED
	   && event.GUIEvent.Caller
	   && event.GUIEvent.Caller->getID() == BUTTON_IMPORT_DECK_CODE) {
		soundManager.PlaySoundEffect(SOUND_BUTTON);
		ImportDeckCodeFromClipboard();
		return true;
	}
	if(event.EventType == irr::EET_GUI_EVENT
	   && event.GUIEvent.EventType == irr::gui::EGET_BUTTON_CLICKED
	   && event.GUIEvent.Caller) {
		s32 bid = event.GUIEvent.Caller->getID();
		if(bid == BUTTON_BR_BACK || bid == BUTTON_BR_FWD || bid == BUTTON_BR_RELOAD
		   || bid == BUTTON_BR_HOME || bid == BUTTON_BR_CLOSE) {
			soundManager.PlaySoundEffect(SOUND_BUTTON);
			switch(bid) {
			case BUTTON_BR_BACK:   BuiltinBrowser::Instance().GoBack(); break;
			case BUTTON_BR_FWD:    BuiltinBrowser::Instance().GoForward(); break;
			case BUTTON_BR_RELOAD: BuiltinBrowser::Instance().Reload(); break;
			case BUTTON_BR_HOME:   mainGame->OpenWebPage(L"https://mycard.world/ygopro/arena/#/cards"); break;
			case BUTTON_BR_CLOSE:  mainGame->CloseBrowser(); break;
			}
			return true;
		}
	}
	if(event.EventType == irr::EET_GUI_EVENT) {
		// Allow GUI button click events to pass through, only apply OnCommonEvent to other GUI events
		if(event.GUIEvent.EventType == irr::gui::EGET_BUTTON_CLICKED) {
			// Do nothing, let it continue to the switch statement
		} else {
			// Apply OnCommonEvent for other GUI events
			if(mainGame->dField.OnCommonEvent(event))
				return false;
		}
	} else {
		// Apply OnCommonEvent for non-GUI events
		if(mainGame->dField.OnCommonEvent(event))
			return false;
	}
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		s32 id = event.GUIEvent.Caller->getID();
		if(mainGame->wCategories->isVisible() && id != BUTTON_CATEGORY_OK)
			break;
		if(mainGame->wQuery->isVisible() && id != BUTTON_YES && id != BUTTON_NO)
			break;
		if(mainGame->wLinkMarks->isVisible() && id != BUTTON_MARKERS_OK)
			break;
		if(mainGame->wRenameDeck->isVisible() && id != BUTTON_RENAME_DECK_SAVE && id != BUTTON_RENAME_DECK_CANCEL)
			break;
		if(mainGame->wSearchWindow->isVisible() && id != BUTTON_SEARCH_AGREE && id != BUTTON_SEARCH_CANCEL)
			break;
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_BUTTON_CLICKED: {
			soundManager.PlaySoundEffect(SOUND_BUTTON);
			switch(id) {
			case BUTTON_MANAGE_DECK: {
				if(is_modified && !mainGame->chkIgnoreDeckChanges->isChecked()) {
					mainGame->gMutex.lock();
					mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, dataManager.GetSysString(1356));
					mainGame->PopupElement(mainGame->wQuery);
					mainGame->gMutex.unlock();
					prev_operation = id;
					break;
				}
				mainGame->ShowDeckManage();
				break;
			}
			case BUTTON_IMPORT_DECK_CODE: {
				ImportDeckCodeFromClipboard();
				break;
			}
			case BUTTON_DECK_CODE_OK: {
				// Get the text from the input box
				const wchar_t* input_text = mainGame->ebDeckCode->getText();
				if(input_text && wcslen(input_text) > 0) {
					// Call the new import function with the input text
					if(ImportDeckCodeFromInput(std::wstring(input_text))) {
						// Success, hide the window
						mainGame->HideElement(mainGame->wDeckCodeQuery);
					}
				} else {
					// Empty input
					mainGame->stACMessage->setText(L"输入不能为空");
					mainGame->PopupElement(mainGame->wACMessage, 20);
				}
				break;
			}
			case BUTTON_DECK_CODE_CANCEL: {
				mainGame->HideElement(mainGame->wDeckCodeQuery);
				break;
			}
			case BUTTON_COPY_DECK_CODE: {
#ifdef WIN32
				// Get the deck code from the edit box
				const wchar_t* deck_code_w = mainGame->ebDeckCode->getText();
				if(deck_code_w && wcslen(deck_code_w) > 0) {
					// Open clipboard
					if(OpenClipboard(NULL)) {
						EmptyClipboard();
						
						// Calculate required buffer size
						size_t len = wcslen(deck_code_w);
						HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, (len + 1) * sizeof(wchar_t));
						if(hClipboardData) {
							wchar_t* pchData = (wchar_t*)GlobalLock(hClipboardData);
							wcscpy(pchData, deck_code_w);
							GlobalUnlock(hClipboardData);
							SetClipboardData(CF_UNICODETEXT, hClipboardData);
						}
						CloseClipboard();
						
						// Show success message
						mainGame->stACMessage->setText(L"卡组码已复制");
						mainGame->PopupElement(mainGame->wACMessage, 20);
					}
				}
#endif
				break;
			}
			case BUTTON_NEW_DECK_DM: {
				dm_mode = 3;
				mainGame->wDMQuery->setText(dataManager.GetSysString(1722));
				mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont, L"");
				mainGame->ebDMName->setText(L"");
				mainGame->ShowElement(mainGame->wDMQuery);
				mainGame->env->setFocus(mainGame->ebDMName);
				break;
			}
			case BUTTON_DM_CLOSE: {
				mainGame->HideElement(mainGame->wDeckManage);
				break;
			}
			case BUTTON_NEW_CATEGORY: {
				mainGame->deckBuilder.dm_mode = 1;
				mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont, L"");
				mainGame->ebDMName->setText(L"");
				mainGame->wDMQuery->setText(dataManager.GetSysString(1723));
				mainGame->ShowElement(mainGame->wDMQuery);
				mainGame->env->setFocus(mainGame->ebDMName);
				break;
			}
			case BUTTON_RENAME_CATEGORY: {
				int sel = mainGame->lstCategories->getSelected();
				if(sel < 4) break;
				mainGame->deckBuilder.dm_mode = 2;
				mainGame->deckBuilder.dm_oldname = mainGame->lstCategories->getListItem(sel);
				mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont, L"");
				mainGame->ebDMName->setText(mainGame->deckBuilder.dm_oldname.c_str());
				mainGame->wDMQuery->setText(dataManager.GetSysString(1724));
				mainGame->ShowElement(mainGame->wDMQuery);
				mainGame->env->setFocus(mainGame->ebDMName);
				break;
			}
			case BUTTON_DELETE_CATEGORY: {
				int sel = mainGame->lstCategories->getSelected();
				if(sel < 4) break;
				const wchar_t* cname = mainGame->lstCategories->getListItem(sel);
				if(deckManager.DeleteCategory(cname)) {
					mainGame->RefreshDeckManage();
				} else {
					mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont, dataManager.GetSysString(1718));
					mainGame->ShowElement(mainGame->wDMQuery);
					mainGame->ebDMName->setText(L"");
					mainGame->ebDMName->setVisible(false);
					mainGame->btnDMOK->setVisible(false);
				}
				break;
			}
			case BUTTON_DM_OK: {
				wchar_t name[256]{};
				BufferIO::CopyWStr(mainGame->ebDMName->getText(), name, 255);
				int cur_mode = mainGame->deckBuilder.dm_mode;
				if(name[0]) {
					if(cur_mode == 1)
						deckManager.CreateCategory(name);
					else if(cur_mode == 2)
						deckManager.RenameCategory(mainGame->deckBuilder.dm_oldname.c_str(), name);
					else if(cur_mode == 3)
						mainGame->deckBuilder.NewDeck(name);
					else if(cur_mode == 4) {
						// Rename deck from manage window
						int dsel = mainGame->lstDecks->getSelected();
						if(dsel >= 0) {
							int catesel = mainGame->lstCategories->getSelected();
							const wchar_t* cname = (catesel >= 4)
								? mainGame->lstCategories->getListItem(catesel) : L"";
							deckManager.RenameDeck(catesel, cname,
								mainGame->lstDecks->getListItem(dsel), name);
						}
					} else if(cur_mode == 5 || cur_mode == 6) {
						// Move (5) or Copy (6) deck to target category by name
						int dsel = mainGame->lstDecks->getSelected();
						if(dsel >= 0) {
							int src_catesel = mainGame->lstCategories->getSelected();
							const wchar_t* src_cname = (src_catesel >= 4)
								? mainGame->lstCategories->getListItem(src_catesel) : L"";
							// Find target category index by matching name
							int dst_catesel = -1;
							for(int i = 0; i < (int)mainGame->lstCategories->getItemCount(); i++) {
								if(!mywcsncasecmp(mainGame->lstCategories->getListItem(i), name, 256)) {
									dst_catesel = i;
									break;
								}
							}
							if(dst_catesel >= 0 && dst_catesel != src_catesel) {
								const wchar_t* dst_cname = (dst_catesel >= 4)
									? mainGame->lstCategories->getListItem(dst_catesel) : L"";
								const wchar_t* dkname = mainGame->lstDecks->getListItem(dsel);
								// Save current deck so we can restore it after the temp load
								Deck saved_deck = deckManager.current_deck;
								// LoadDeck(filepath) loads into current_deck — use full path
								wchar_t src_path[256], src_file[256];
								DeckManager::GetCategoryPath(src_path, src_catesel, src_cname);
								myswprintf(src_file, L"%ls/%ls.ydk", src_path, dkname);
								if(deckManager.LoadDeck(src_file)) {
									// Save loaded deck to destination category
									deckManager.SaveDeck(deckManager.current_deck, dst_catesel, dst_cname, dkname);
									if(cur_mode == 5) {
										// Move: delete source after successful copy
										deckManager.DeleteDeck(src_catesel, src_cname, dkname);
									}
								}
								// Restore current deck
								deckManager.current_deck = saved_deck;
							} else if(dst_catesel < 0) {
								// Target category name not found
								mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont,
									dataManager.GetSysString(1716));
							}
						}
					}
				}
				if(cur_mode == 1) {
					int n = mainGame->lstCategories->getItemCount();
					if(n > 0)
						mainGame->lstCategories->setSelected(n - 1);
				}
				mainGame->deckBuilder.dm_mode = 0;
				mainGame->ebDMName->setVisible(true);
				mainGame->btnDMOK->setVisible(true);
				mainGame->HideElement(mainGame->wDMQuery);
				mainGame->RefreshDeckManage();
				break;
			}
			case BUTTON_DM_CANCEL: {
				mainGame->deckBuilder.dm_mode = 0;
				mainGame->ebDMName->setVisible(true);
				mainGame->btnDMOK->setVisible(true);
				mainGame->HideElement(mainGame->wDMQuery);
				break;
			}
			case BUTTON_CLEAR_DECK: {
				mainGame->gMutex.lock();
				mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, dataManager.GetSysString(1339));
				mainGame->PopupElement(mainGame->wQuery);
				mainGame->gMutex.unlock();
				prev_operation = id;
				break;
			}
			case BUTTON_GENERATE_DECK_CODE: {
				// Get current deck's card codes
				std::vector<int> main, ex, side;
				
				// Extract main deck card codes
				for(auto& card : deckManager.current_deck.main) {
					main.push_back(card->first);
				}
				
				// Extract extra deck card codes
				for(auto& card : deckManager.current_deck.extra) {
					ex.push_back(card->first);
				}
				
				// Extract side deck card codes
				for(auto& card : deckManager.current_deck.side) {
					side.push_back(card->first);
				}
				
				// Check if deck is empty
				if(main.empty() && ex.empty() && side.empty()) {
					mainGame->stACMessage->setText(L"卡组中没有卡片");
					mainGame->PopupElement(mainGame->wACMessage, 20);
					break;
				}
				
				// Get deck name (use current deck name if available)
				const wchar_t* current_deck_name = L"";
				int current_deck_idx = mainGame->cbDBDecks->getSelected();
				if(current_deck_idx >= 0) {
					current_deck_name = mainGame->cbDBDecks->getItem(current_deck_idx);
				}
				
				// Generate the deck code (pure code only)
				std::string pure_code = DeckCode_Encode(main, ex, side, std::wstring(current_deck_name));
				
				// Build full URI with deck name, type, version and code
				std::string deck_name_utf8;
				if(current_deck_name) {
					int len = WideCharToMultiByte(CP_UTF8, 0, current_deck_name, -1, NULL, 0, NULL, NULL);
					if(len > 1) {
						deck_name_utf8.resize(len - 1);
						WideCharToMultiByte(CP_UTF8, 0, current_deck_name, -1, &deck_name_utf8[0], len, NULL, NULL);
					}
				}
				
				// URL encode the deck name (basic implementation)
				std::string encoded_name;
				for(char c : deck_name_utf8) {
					if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
						encoded_name += c;
					} else {
						char buf[4];
						sprintf(buf, "%%%.2X", (unsigned char)c);
						encoded_name += buf;
					}
				}
				
				// Build the full URI
				std::string full_uri = "http://deck.ourygo.top?";
				if(!encoded_name.empty()) {
					full_uri += "name=" + encoded_name + "&";
				}
				full_uri += "ygotype=deck&v=1&d=" + pure_code;
				
				// Show the deck code in a dialog with copy button
				mainGame->wDeckCodeQuery->setText(L"生成的卡组码");
				mainGame->SetStaticText(mainGame->stDeckCodeMessage, 270, mainGame->guiFont, L"卡组码：");
				wchar_t wcode[1024]{};
				BufferIO::DecodeUTF8(full_uri.c_str(), wcode);
				mainGame->ebDeckCode->setText(wcode);
				mainGame->ShowElement(mainGame->wDeckCodeQuery);
				mainGame->env->setFocus(mainGame->ebDeckCode);
				break;
			}
			case BUTTON_SORT_DECK: {
				std::sort(deckManager.current_deck.main.begin(), deckManager.current_deck.main.end(), ClientCard::deck_sort_lv);
				std::sort(deckManager.current_deck.extra.begin(), deckManager.current_deck.extra.end(), ClientCard::deck_sort_lv);
				std::sort(deckManager.current_deck.side.begin(), deckManager.current_deck.side.end(), ClientCard::deck_sort_lv);
				break;
			}
			case BUTTON_SHUFFLE_DECK: {
				std::shuffle(deckManager.current_deck.main.begin(), deckManager.current_deck.main.end(), rnd);
				break;
			}
			case BUTTON_SAVE_DECK: {
				int sel = mainGame->cbDBDecks->getSelected();
				if(sel >= 0) {
					int cate = mainGame->current_category;
					const wchar_t* cname = L"";
					// custom categories live in ./deck/<name>; NONE (unclassified) uses the ./deck root
					if(cate >= 4 && !mainGame->deckBuilder.current_category_name.empty()) {
						cname = mainGame->deckBuilder.current_category_name.c_str();
					}
					if(deckManager.SaveDeck(deckManager.current_deck, cate, cname, mainGame->cbDBDecks->getItem(sel))) {
						mainGame->stACMessage->setText(dataManager.GetSysString(1335));
						mainGame->PopupElement(mainGame->wACMessage, 20);
						is_modified = false;
					}
				}
				break;
			}
			case BUTTON_SAVE_DECK_AS: {
				const wchar_t* dname = mainGame->ebDeckname->getText();
				if(*dname == 0)
					break;
				int sel = -1;
				for(size_t i = 0; i < mainGame->cbDBDecks->getItemCount(); ++i) {
					if(!wcscmp(dname, mainGame->cbDBDecks->getItem(i))) {
						sel = i;
						break;
					}
				}
				if(sel >= 0)
					mainGame->cbDBDecks->setSelected(sel);
				else {
					mainGame->cbDBDecks->addItem(dname);
					mainGame->cbDBDecks->setSelected(mainGame->cbDBDecks->getItemCount() - 1);
				}
				// Use category-aware SaveDeck function
				int cate = mainGame->current_category;
				const wchar_t* cname = L"";
				// custom categories live in ./deck/<name>; NONE (unclassified) uses the ./deck root
				if(cate >= 4 && !mainGame->deckBuilder.current_category_name.empty()) {
					cname = mainGame->deckBuilder.current_category_name.c_str();
				}
				if(deckManager.SaveDeck(deckManager.current_deck, cate, cname, dname)) {
					mainGame->stACMessage->setText(dataManager.GetSysString(1335));
					mainGame->PopupElement(mainGame->wACMessage, 20);
					is_modified = false;
				}
				break;
			}
			case BUTTON_RENAME_DECK: {
				int sel = mainGame->cbDBDecks->getSelected();
				if(sel == -1)
					break;
				mainGame->gMutex.lock();
				mainGame->wRenameDeck->setText(dataManager.GetSysString(1376));
				mainGame->ebREName->setText(mainGame->cbDBDecks->getItem(sel));
				mainGame->PopupElement(mainGame->wRenameDeck);
				mainGame->gMutex.unlock();
				prev_operation = id;
				prev_sel = sel;
				break;
			}
			case BUTTON_RENAME_DECK_SAVE: {
				mainGame->HideElement(mainGame->wRenameDeck);
				if(prev_operation == BUTTON_RENAME_DECK) {
					wchar_t newname[256];
					BufferIO::CopyWStr(mainGame->ebREName->getText(), newname, 256);
					if(mywcsncasecmp(newname + wcslen(newname) - 4, L"", 4)) {
						myswprintf(newname, L"%ls", mainGame->ebREName->getText());
					}
					int cate = mainGame->current_category;
					const wchar_t* cname = L"";
					// custom categories live in ./deck/<name>; NONE (unclassified) uses the ./deck root
					if(cate >= 4 && !mainGame->deckBuilder.current_category_name.empty()) {
						cname = mainGame->deckBuilder.current_category_name.c_str();
					}
					if(deckManager.RenameDeck(cate, cname, mainGame->cbDBDecks->getItem(prev_sel), newname)) {
						mainGame->cbDBDecks->removeItem(prev_sel);
						mainGame->cbDBDecks->addItem(newname);
						mainGame->cbDBDecks->setSelected(mainGame->cbDBDecks->getItemCount() - 1);
						mainGame->stASMessage->setText(dataManager.GetSysString(1375));
					        mainGame->PopupElement(mainGame->wACMessage, 20);
						is_modified = false;
					} else {
						mainGame->env->addMessageBox(L"", dataManager.GetSysString(1365));
					}
				}
				prev_operation = 0;
				prev_sel = -1;
				break;
			}
			case BUTTON_RENAME_DECK_CANCEL: {
				mainGame->HideElement(mainGame->wRenameDeck);
				prev_operation = 0;
				prev_sel = -1;
				break;
			}
			case BUTTON_DELETE_DECK: {
				int sel = mainGame->cbDBDecks->getSelected();
				if(sel == -1)
					break;
				mainGame->gMutex.lock();
				wchar_t textBuffer[256];
				myswprintf(textBuffer, L"%ls\n%ls", mainGame->cbDBDecks->getItem(sel), dataManager.GetSysString(1337));
				mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, textBuffer);
				mainGame->PopupElement(mainGame->wQuery);
				mainGame->gMutex.unlock();
				prev_operation = id;
				prev_sel = sel;
				break;
			}
			case BUTTON_LEAVE_GAME: {
				if(is_modified && !mainGame->chkIgnoreDeckChanges->isChecked()) {
					mainGame->gMutex.lock();
					mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, dataManager.GetSysString(1356));
					mainGame->PopupElement(mainGame->wQuery);
					mainGame->gMutex.unlock();
					prev_operation = id;
					break;
				}
				Terminate();
				break;
			}
			case BUTTON_EFFECT_FILTER: {
				mainGame->PopupElement(mainGame->wCategories);
				break;
			}
			case BUTTON_START_FILTER: {
				StartFilter();
				if(!mainGame->gameConf.separate_clear_button)
					ClearFilter();
				break;
			}
			case BUTTON_CLEAR_FILTER: {
				ClearSearch();
				break;
			}
			case BUTTON_CATEGORY_OK: {
				filter_effect = 0;
				long long filter = 0x1;
				for(int i = 0; i < 32; ++i, filter <<= 1)
					if(mainGame->chkCategory[i]->isChecked())
						filter_effect |= filter;
				mainGame->btnEffectFilter->setPressed(filter_effect > 0);
				mainGame->HideElement(mainGame->wCategories);
				InstantSearch();
				break;
			}
			case BUTTON_SIDE_OK: {
				if(deckManager.current_deck.main.size() != pre_mainc || deckManager.current_deck.extra.size() != pre_extrac
				        || deckManager.current_deck.side.size() != pre_sidec) {
					soundManager.PlaySoundEffect(SOUND_INFO);
					mainGame->env->addMessageBox(L"", dataManager.GetSysString(1410));
					break;
				}
				mainGame->ClearCardInfo();
				unsigned char deckbuf[1024];
				auto pdeck = deckbuf;
				BufferIO::WriteInt32(pdeck, deckManager.current_deck.main.size() + deckManager.current_deck.extra.size());
				BufferIO::WriteInt32(pdeck, deckManager.current_deck.side.size());
				for(size_t i = 0; i < deckManager.current_deck.main.size(); ++i)
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.main[i]->first);
				for(size_t i = 0; i < deckManager.current_deck.extra.size(); ++i)
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.extra[i]->first);
				for(size_t i = 0; i < deckManager.current_deck.side.size(); ++i)
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.side[i]->first);
				DuelClient::SendBufferToServer(CTOS_UPDATE_DECK, deckbuf, pdeck - deckbuf);
				break;
			}
			case BUTTON_SIDE_RELOAD: {
				// Use category-aware LoadDeck function
				int category_index = mainGame->current_category;
				const wchar_t* category_name = L"";
				if(category_index >= 4) {
					// For custom categories, get the name
					category_name = mainGame->deckBuilder.current_category_name.c_str();
				}
				deckManager.LoadDeck(category_index, category_name, mainGame->cbDeckSelect->getItem(mainGame->cbDeckSelect->getSelected()));
				break;
			}
			case BUTTON_MSG_OK: {
				mainGame->HideElement(mainGame->wMessage);
				mainGame->actionSignal.Set();
				break;
			}
			case BUTTON_YES: {
				mainGame->HideElement(mainGame->wQuery);
				if(!mainGame->is_building || mainGame->is_siding)
					break;
				if(prev_operation == BUTTON_CLEAR_DECK) {
					deckManager.current_deck.main.clear();
					deckManager.current_deck.extra.clear();
					deckManager.current_deck.side.clear();
				} else if(prev_operation == BUTTON_DELETE_DECK) {
					int sel = prev_sel;
					int cate = mainGame->current_category;
					const wchar_t* cname = L"";
					// custom categories live in ./deck/<name>; NONE (unclassified) uses the ./deck root
					if(cate >= 4 && !mainGame->deckBuilder.current_category_name.empty()) {
						cname = mainGame->deckBuilder.current_category_name.c_str();
					}
					if(deckManager.DeleteDeck(cate, cname, mainGame->cbDBDecks->getItem(sel))) {
						mainGame->cbDBDecks->removeItem(sel);
						int count = mainGame->cbDBDecks->getItemCount();
						if(sel >= count)
							sel = count - 1;
						mainGame->cbDBDecks->setSelected(sel);
						if(sel != -1) {
							// Reload the deck in the same category
							deckManager.LoadDeck(cate, cname, mainGame->cbDBDecks->getItem(sel));
						}
						mainGame->stACMessage->setText(dataManager.GetSysString(1338));
						mainGame->PopupElement(mainGame->wACMessage, 20);
						prev_deck = sel;
						is_modified = false;
					}
					prev_sel = -1;
				} else if(prev_operation == BUTTON_LEAVE_GAME) {
					Terminate();
				} else if(prev_operation == COMBOBOX_DBDECKS) {
					int sel = mainGame->cbDBDecks->getSelected();
					deckManager.LoadDeck(mainGame->cbDBDecks->getItem(sel));
					prev_deck = sel;
					is_modified = false;
				} else if(prev_operation == BUTTON_MANAGE_DECK) {
					mainGame->ShowDeckManage();
				} else if(prev_operation == LISTBOX_CATEGORIES) {
					int catesel = mainGame->lstCategories->getSelected();
					if(catesel == 3) catesel = 2;
					ApplyCategory(catesel);
				} else if(prev_operation == LISTBOX_DECKS) {
					ApplyDeck(mainGame->lstDecks->getSelected());
				}
				prev_operation = 0;
				break;
			}
			case BUTTON_NO: {
				mainGame->HideElement(mainGame->wQuery);
				if (prev_operation == COMBOBOX_DBDECKS) {
					mainGame->cbDBDecks->setSelected(prev_deck);
				}
				if(prev_operation == LISTBOX_CATEGORIES)
					mainGame->lstCategories->setSelected(prev_category);
				else if(prev_operation == LISTBOX_DECKS)
					mainGame->lstDecks->setSelected(prev_deck);
				prev_operation = 0;
				break;
			}
			case BUTTON_MARKS_FILTER: {
				mainGame->PopupElement(mainGame->wLinkMarks);
				break;
			}
			case BUTTON_MARKERS_OK: {
				filter_marks = 0;
				if (mainGame->btnMark[0]->isPressed())
					filter_marks |= 0100;
				if (mainGame->btnMark[1]->isPressed())
					filter_marks |= 0200;
				if (mainGame->btnMark[2]->isPressed())
					filter_marks |= 0400;
				if (mainGame->btnMark[3]->isPressed())
					filter_marks |= 0010;
				if (mainGame->btnMark[4]->isPressed())
					filter_marks |= 0040;
				if (mainGame->btnMark[5]->isPressed())
					filter_marks |= 0001;
				if (mainGame->btnMark[6]->isPressed())
					filter_marks |= 0002;
				if (mainGame->btnMark[7]->isPressed())
					filter_marks |= 0004;
				mainGame->HideElement(mainGame->wLinkMarks);
				mainGame->btnMarksFilter->setPressed(filter_marks > 0);
				InstantSearch();
				break;
			}
			// [Card Search] Open card name search popup
			case BUTTON_CARD_SEARCH: {
				mainGame->gMutex.lock();
				mainGame->ebSearchName->setText(L"");
				mainGame->PopupElement(mainGame->wSearchWindow);
				mainGame->env->setFocus(mainGame->ebSearchName);
				mainGame->gMutex.unlock();
				break;
			}
			// [Card Search] Confirm: online card query in builtin browser (ygocdb)
			case BUTTON_SEARCH_AGREE: {
				const wchar_t* sname = mainGame->ebSearchName->getText();
				if(sname && *sname) {
					BufferIO::CopyWStr(sname, mainGame->gameConf.searchname, 256);
					wchar_t url[1024]{};
					myswprintf(url, L"https://ygocdb.com/?search=%ls", sname);
					mainGame->OpenWebPage(url);
				}
				mainGame->HideElement(mainGame->wSearchWindow);
				break;
			}
			// [Card Search] Cancel: close popup without applying
			case BUTTON_SEARCH_CANCEL: {
				mainGame->HideElement(mainGame->wSearchWindow);
				break;
			}
			// [DM] Rename deck from manage window
			case BUTTON_RENAME_DECK_DM: {
				int sel = mainGame->lstDecks->getSelected();
				if(sel < 0) break;
				mainGame->gMutex.lock();
				mainGame->wDMQuery->setText(dataManager.GetSysString(1724));
				mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont, L"");
				mainGame->ebDMName->setText(mainGame->lstDecks->getListItem(sel));
				mainGame->ShowElement(mainGame->wDMQuery);
				mainGame->env->setFocus(mainGame->ebDMName);
				mainGame->gMutex.unlock();
				dm_mode = 4; // 4 = rename deck from manage window
				break;
			}
			// [DM] Delete deck from manage window
			case BUTTON_DELETE_DECK_DM: {
				int sel = mainGame->lstDecks->getSelected();
				if(sel < 0) break;
				int catesel = mainGame->lstCategories->getSelected();
				const wchar_t* dname = mainGame->lstDecks->getListItem(sel);
				const wchar_t* cname = (catesel >= 4)
					? mainGame->lstCategories->getListItem(catesel) : L"";
				if(deckManager.DeleteDeck(catesel, cname, dname)) {
					mainGame->RefreshDeckManage();
				} else {
					mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont,
						dataManager.GetSysString(1718));
					mainGame->ShowElement(mainGame->wDMQuery);
					mainGame->ebDMName->setVisible(false);
					mainGame->btnDMOK->setVisible(false);
				}
				break;
			}
			// [DM] Move deck: ask for target category name via wDMQuery
			case BUTTON_MOVE_DECK: {
				int sel = mainGame->lstDecks->getSelected();
				if(sel < 0) break;
				mainGame->gMutex.lock();
				mainGame->wDMQuery->setText(dataManager.GetSysString(1472));
				mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont, L"");
				mainGame->ebDMName->setText(L"");
				mainGame->ShowElement(mainGame->wDMQuery);
				mainGame->env->setFocus(mainGame->ebDMName);
				mainGame->gMutex.unlock();
				dm_mode = 5; // 5 = move deck
				break;
			}
			// [DM] Copy deck: ask for target category name via wDMQuery
			case BUTTON_COPY_DECK: {
				int sel = mainGame->lstDecks->getSelected();
				if(sel < 0) break;
				mainGame->gMutex.lock();
				mainGame->wDMQuery->setText(dataManager.GetSysString(1473));
				mainGame->SetStaticText(mainGame->stDMMessage, 270, mainGame->guiFont, L"");
				mainGame->ebDMName->setText(L"");
				mainGame->ShowElement(mainGame->wDMQuery);
				mainGame->env->setFocus(mainGame->ebDMName);
				mainGame->gMutex.unlock();
				dm_mode = 6; // 6 = copy deck
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_SCROLL_BAR_CHANGED: {
			switch(id) {
			case SCROLL_FILTER: {
				GetHoveredCard();
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_ENTER: {
			switch(id) {
			case EDITBOX_INPUTS:
			case EDITBOX_KEYWORD: {
				StartFilter();
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_CHANGED: {
			switch(id) {
			case EDITBOX_KEYWORD: {
				InstantSearch();
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_LISTBOX_CHANGED: {
			switch(id) {
			case LISTBOX_CATEGORIES: {
				if(suppress_list_events)
					break;
				if(mainGame->wDMQuery->isVisible()) {
					mainGame->lstCategories->setSelected(prev_category);
					break;
				}
				int catesel = mainGame->lstCategories->getSelected();
				if(catesel < 0)
					break;
				if(catesel == 3) {
					catesel = 2;
					mainGame->lstCategories->setSelected(catesel);
					if(prev_category == catesel)
						break;
				}
				if(is_modified && !mainGame->chkIgnoreDeckChanges->isChecked()) {
					mainGame->gMutex.lock();
					mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, dataManager.GetSysString(1356));
					mainGame->PopupElement(mainGame->wQuery);
					mainGame->gMutex.unlock();
					prev_operation = id;
					break;
				}
				ApplyCategory(catesel);
				break;
			}
			case LISTBOX_DECKS: {
				if(mainGame->wDMQuery->isVisible()) {
					mainGame->lstDecks->setSelected(prev_deck);
					break;
				}
				int decksel = mainGame->lstDecks->getSelected();
				if(decksel < 0)
					break;
				if(is_modified && !mainGame->chkIgnoreDeckChanges->isChecked()) {
					mainGame->gMutex.lock();
					mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, dataManager.GetSysString(1356));
					mainGame->PopupElement(mainGame->wQuery);
					mainGame->gMutex.unlock();
					prev_operation = id;
					break;
				}
				ApplyDeck(decksel);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_COMBO_BOX_CHANGED: {
			switch(id) {
			case COMBOBOX_DBLFLIST: {
				filterList = &deckManager._lfList[mainGame->cbDBLFList->getSelected()].content;
				break;
			}
			case COMBOBOX_DBCATEGORY: {
				if(is_modified && !mainGame->chkIgnoreDeckChanges->isChecked()) {
					mainGame->gMutex.lock();
					mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, dataManager.GetSysString(1356));
					mainGame->PopupElement(mainGame->wQuery);
					mainGame->gMutex.unlock();
					prev_operation = id;
					break;
				}
				int catesel = mainGame->current_category;
				prev_category = catesel;
				mainGame->RefreshDeck(mainGame->cbDBDecks);
				mainGame->cbDBDecks->setSelected(0);
				int csel = mainGame->cbDBDecks->getSelected();
				if(csel >= 0)
					deckManager.LoadDeck(catesel,
					                     (catesel >= 4) ? mainGame->deckBuilder.current_category_name.c_str() : L"",
					                     mainGame->cbDBDecks->getItem(csel));
				prev_deck = csel;
				is_modified = false;
				break;
			}
			case COMBOBOX_DBDECKS: {
				if(is_modified && !mainGame->chkIgnoreDeckChanges->isChecked()) {
					mainGame->gMutex.lock();
					mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->guiFont, dataManager.GetSysString(1356));
					mainGame->PopupElement(mainGame->wQuery);
					mainGame->gMutex.unlock();
					prev_operation = id;
					break;
				}
				int sel = mainGame->cbDBDecks->getSelected();
				if(sel >= 0) {
					int cate = mainGame->current_category;
					const wchar_t* cname = L"";
					// custom categories live in ./deck/<name>; NONE (unclassified) uses the ./deck root
					if(cate >= 4) {
						cname = mainGame->deckBuilder.current_category_name.c_str();
					}
					deckManager.LoadDeck(cate, cname, mainGame->cbDBDecks->getItem(sel));
				}
				prev_deck = sel;
				is_modified = false;
				break;
			}
			case COMBOBOX_MAINTYPE: {
				mainGame->cbCardType2->setSelected(0);
				mainGame->cbAttribute->setSelected(0);
				mainGame->cbRace->setSelected(0);
				mainGame->ebAttack->setText(L"");
				mainGame->ebDefense->setText(L"");
				mainGame->ebStar->setText(L"");
				mainGame->ebScale->setText(L"");
				switch(mainGame->cbCardType->getSelected()) {
				case 0: {
					mainGame->cbCardType2->setEnabled(false);
					mainGame->cbCardType2->setSelected(0);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefense->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					mainGame->ebScale->setEnabled(false);
					break;
				}
				case 1: {
					wchar_t normaltuner[32];
					wchar_t normalpen[32];
					wchar_t syntuner[32];
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(true);
					mainGame->cbAttribute->setEnabled(true);
					mainGame->ebAttack->setEnabled(true);
					mainGame->ebDefense->setEnabled(true);
					mainGame->ebStar->setEnabled(true);
					mainGame->ebScale->setEnabled(true);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1080), 0);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1054), TYPE_MONSTER + TYPE_NORMAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1055), TYPE_MONSTER + TYPE_EFFECT);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1056), TYPE_MONSTER + TYPE_FUSION);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1057), TYPE_MONSTER + TYPE_RITUAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1063), TYPE_MONSTER + TYPE_SYNCHRO);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1073), TYPE_MONSTER + TYPE_XYZ);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1074), TYPE_MONSTER + TYPE_PENDULUM);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1076), TYPE_MONSTER + TYPE_LINK);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1075), TYPE_MONSTER + TYPE_SPSUMMON);
					myswprintf(normaltuner, L"%ls|%ls", dataManager.GetSysString(1054), dataManager.GetSysString(1062));
					mainGame->cbCardType2->addItem(normaltuner, TYPE_MONSTER + TYPE_NORMAL + TYPE_TUNER);
					myswprintf(normalpen, L"%ls|%ls", dataManager.GetSysString(1054), dataManager.GetSysString(1074));
					mainGame->cbCardType2->addItem(normalpen, TYPE_MONSTER + TYPE_NORMAL + TYPE_PENDULUM);
					myswprintf(syntuner, L"%ls|%ls", dataManager.GetSysString(1063), dataManager.GetSysString(1062));
					mainGame->cbCardType2->addItem(syntuner, TYPE_MONSTER + TYPE_SYNCHRO + TYPE_TUNER);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1062), TYPE_MONSTER + TYPE_TUNER);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1061), TYPE_MONSTER + TYPE_DUAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1060), TYPE_MONSTER + TYPE_UNION);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1059), TYPE_MONSTER + TYPE_SPIRIT);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1071), TYPE_MONSTER + TYPE_FLIP);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1072), TYPE_MONSTER + TYPE_TOON);
					break;
				}
				case 2: {
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefense->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					mainGame->ebScale->setEnabled(false);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1080), 0);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1054), TYPE_SPELL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1066), TYPE_SPELL + TYPE_QUICKPLAY);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1067), TYPE_SPELL + TYPE_CONTINUOUS);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1057), TYPE_SPELL + TYPE_RITUAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1068), TYPE_SPELL + TYPE_EQUIP);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1069), TYPE_SPELL + TYPE_FIELD);
					break;
				}
				case 3: {
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefense->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					mainGame->ebScale->setEnabled(false);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1080), 0);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1054), TYPE_TRAP);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1067), TYPE_TRAP + TYPE_CONTINUOUS);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1070), TYPE_TRAP + TYPE_COUNTER);
					break;
				}
				}
				mainGame->env->setFocus(0);
				InstantSearch();
				break;
			}
			case COMBOBOX_SORTTYPE: {
				SortList();
				mainGame->env->setFocus(0);
				break;
			}
			case COMBOBOX_SECONDTYPE: {
				if (mainGame->cbCardType->getSelected() == 1) {
					if (mainGame->cbCardType2->getSelected() == 8) {
						mainGame->ebDefense->setEnabled(false);
						mainGame->ebDefense->setText(L"");
					} else {
						mainGame->ebDefense->setEnabled(true);
					}
				}
				mainGame->env->setFocus(0);
				InstantSearch();
				break;
			}
			case COMBOBOX_ATTRIBUTE:
			case COMBOBOX_RACE:
			case COMBOBOX_LIMIT:
				mainGame->env->setFocus(0);
				InstantSearch();
				break;
			}
		}
		default: break;
		}
		break;
	}
	case irr::EET_MOUSE_INPUT_EVENT: {
		switch(event.MouseInput.Event) {
		case irr::EMIE_LMOUSE_PRESSED_DOWN: {
			irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
			if(root->getElementFromPoint(mouse_pos) != root)
				break;
			if(mainGame->wCategories->isVisible() || mainGame->wQuery->isVisible())
				break;
			if(hovered_pos == 0 || hovered_seq == -1)
				break;
			click_pos = hovered_pos;
			dragx = event.MouseInput.X;
			dragy = event.MouseInput.Y;
			draging_pointer = dataManager.GetCodePointer(hovered_code);
			if(draging_pointer == dataManager.datas_end)
				break;
			if(hovered_pos == 4) {
				if(!check_limit(draging_pointer))
					break;
			}
			is_starting_dragging = true;
			break;
		}
		case irr::EMIE_LMOUSE_LEFT_UP: {
			{
				std::wstring search_text;
				if(!mainGame->is_siding && mainGame->GetCardTextSearchTextAt(mouse_pos, search_text)) {
					mainGame->ebCardName->setText(search_text.c_str());
					mainGame->env->setFocus(mainGame->ebCardName);
					StartFilter();
					break;
				}
			}
			is_starting_dragging = false;
			if(!is_draging)
				break;
			soundManager.PlaySoundEffect(SOUND_CARD_DROP);
			bool pushed = false;
			if(hovered_pos == 1)
				pushed = push_main(draging_pointer, hovered_seq);
			else if(hovered_pos == 2)
				pushed = push_extra(draging_pointer, hovered_seq + is_lastcard);
			else if(hovered_pos == 3)
				pushed = push_side(draging_pointer, hovered_seq + is_lastcard);
			else if(hovered_pos == 4 && !mainGame->is_siding)
				pushed = true;
			if(!pushed) {
				if(click_pos == 1)
					push_main(draging_pointer);
				else if(click_pos == 2)
					push_extra(draging_pointer);
				else if(click_pos == 3)
					push_side(draging_pointer);
			}
			is_draging = false;
			break;
		}
		case irr::EMIE_RMOUSE_LEFT_UP: {
			if(mainGame->is_siding) {
				if(is_draging)
					break;
				if(hovered_pos == 0 || hovered_seq == -1)
					break;
				auto pointer = dataManager.GetCodePointer(hovered_code);
				if(pointer == dataManager.datas_end)
					break;
				soundManager.PlaySoundEffect(SOUND_CARD_DROP);
				if(hovered_pos == 1) {
					if(push_side(pointer))
						pop_main(hovered_seq);
				} else if(hovered_pos == 2) {
					if(push_side(pointer))
						pop_extra(hovered_seq);
				} else {
					if(push_extra(pointer) || push_main(pointer))
						pop_side(hovered_seq);
				}
				break;
			}
			if(mainGame->wCategories->isVisible() || mainGame->wQuery->isVisible())
				break;
			if(!is_draging) {
				if(hovered_pos == 0 || hovered_seq == -1)
					break;
				soundManager.PlaySoundEffect(SOUND_CARD_DROP);
				if(hovered_pos == 1) {
					pop_main(hovered_seq);
				} else if(hovered_pos == 2) {
					pop_extra(hovered_seq);
				} else if(hovered_pos == 3) {
					pop_side(hovered_seq);
				} else {
					auto pointer = dataManager.GetCodePointer(hovered_code);
					if(pointer == dataManager.datas_end)
						break;
					if(!check_limit(pointer))
						break;
					if(!push_extra(pointer) && !push_main(pointer))
						push_side(pointer);
				}
			} else {
				soundManager.PlaySoundEffect(SOUND_CARD_PICK);
				if(click_pos == 1) {
					push_side(draging_pointer);
				} else if(click_pos == 2) {
					push_side(draging_pointer);
				} else if(click_pos == 3) {
					if(!push_extra(draging_pointer))
						push_main(draging_pointer);
				} else {
					push_side(draging_pointer);
				}
				is_draging = false;
			}
			break;
		}
		case irr::EMIE_MMOUSE_LEFT_UP: {
			if (mainGame->is_siding)
				break;
			if (mainGame->wCategories->isVisible() || mainGame->wQuery->isVisible())
				break;
			if (hovered_pos == 0 || hovered_seq == -1)
				break;
			if (is_draging)
				break;
			auto pointer = dataManager.GetCodePointer(hovered_code);
			if (pointer == dataManager.datas_end)
				break;
			if(!check_limit(pointer))
				break;
			soundManager.PlaySoundEffect(SOUND_CARD_PICK);
			if (hovered_pos == 1) {
				if(!push_main(pointer))
					push_side(pointer);
			} else if (hovered_pos == 2) {
				if(!push_extra(pointer))
					push_side(pointer);
			} else if (hovered_pos == 3) {
				if(!push_side(pointer) && !push_extra(pointer))
					push_main(pointer);
			} else {
				if(!push_extra(pointer) && !push_main(pointer))
					push_side(pointer);
			}
			break;
		}
		case irr::EMIE_MOUSE_MOVED: {
			if(is_starting_dragging) {
				is_draging = true;
				soundManager.PlaySoundEffect(SOUND_CARD_PICK);
				if(hovered_pos == 1)
					pop_main(hovered_seq);
				else if(hovered_pos == 2)
					pop_extra(hovered_seq);
				else if(hovered_pos == 3)
					pop_side(hovered_seq);
				is_starting_dragging = false;
			}
			mouse_pos.set(event.MouseInput.X, event.MouseInput.Y);
			GetHoveredCard();
			break;
		}
		case irr::EMIE_MOUSE_WHEEL: {
			if(!mainGame->scrFilter->isVisible())
				break;
			if(mainGame->env->hasFocus(mainGame->scrFilter))
				break;
			irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
			if(root->getElementFromPoint(mouse_pos) != root)
				break;
			if(event.MouseInput.Wheel < 0) {
				if(mainGame->scrFilter->getPos() < mainGame->scrFilter->getMax())
					mainGame->scrFilter->setPos(mainGame->scrFilter->getPos() + 1);
			} else {
				if(mainGame->scrFilter->getPos() > 0)
					mainGame->scrFilter->setPos(mainGame->scrFilter->getPos() - 1);
			}
			GetHoveredCard();
			break;
		}
		default: break;
		}
		break;
	}
	case irr::EET_KEY_INPUT_EVENT: {
		break;
	}
	default: break;
	}
	return false;
}
void DeckBuilder::GetHoveredCard() {
	int pre_code = hovered_code;
	hovered_pos = 0;
	hovered_code = 0;
	irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
	if(root->getElementFromPoint(mouse_pos) != root)
		return;
	position2di pos = mainGame->ResizeReverse(mouse_pos.X, mouse_pos.Y);
	int x = pos.X;
	int y = pos.Y;
	is_lastcard = 0;
	if(x >= 314 && x <= 794) {
		if(y >= 164 && y <= 435) {
			int lx = 10, px, py = (y - 164) / 68;
			hovered_pos = 1;
			if(deckManager.current_deck.main.size() > 40)
				lx = (deckManager.current_deck.main.size() - 41) / 4 + 11;
			if(x >= 750)
				px = lx - 1;
			else
				px = (x - 314) * (lx - 1) / 436;
			hovered_seq = py * lx + px;
			if(hovered_seq >= (int)deckManager.current_deck.main.size()) {
				hovered_seq = -1;
				hovered_code = 0;
			} else {
				hovered_code = deckManager.current_deck.main[hovered_seq]->first;
			}
		} else if(y >= 466 && y <= 530) {
			int lx = deckManager.current_deck.extra.size();
			hovered_pos = 2;
			if(lx < 10)
				lx = 10;
			if(x >= 750)
				hovered_seq = lx - 1;
			else
				hovered_seq = (x - 314) * (lx - 1) / 436;
			if(hovered_seq >= (int)deckManager.current_deck.extra.size()) {
				hovered_seq = -1;
				hovered_code = 0;
			} else {
				hovered_code = deckManager.current_deck.extra[hovered_seq]->first;
				if(x >= 772)
					is_lastcard = 1;
			}
		} else if (y >= 564 && y <= 628) {
			int lx = deckManager.current_deck.side.size();
			hovered_pos = 3;
			if(lx < 10)
				lx = 10;
			if(x >= 750)
				hovered_seq = lx - 1;
			else
				hovered_seq = (x - 314) * (lx - 1) / 436;
			if(hovered_seq >= (int)deckManager.current_deck.side.size()) {
				hovered_seq = -1;
				hovered_code = 0;
			} else {
				hovered_code = deckManager.current_deck.side[hovered_seq]->first;
				if(x >= 772)
					is_lastcard = 1;
			}
		}
	} else if(x >= 810 && x <= 995 && y >= 165 && y <= 626) {
		hovered_pos = 4;
		hovered_seq = (y - 165) / 66;
		int pos = mainGame->scrFilter->getPos() + hovered_seq;
		if(pos >= (int)results.size()) {
			hovered_seq = -1;
			hovered_code = 0;
		} else {
			hovered_code = results[pos]->first;
		}
	}
	if(is_draging) {
		dragx = mouse_pos.X;
		dragy = mouse_pos.Y;
	}
	if(!is_draging && pre_code != hovered_code) {
		if(hovered_code)
			mainGame->ShowCardInfo(hovered_code);
		if(pre_code)
			imageManager.RemoveTexture(pre_code);
	}
}
void DeckBuilder::StartFilter() {
	filter_type = mainGame->cbCardType->getSelected();
	filter_type2 = mainGame->cbCardType2->getItemData(mainGame->cbCardType2->getSelected());
	filter_lm = mainGame->cbLimit->getSelected();
	if(filter_type == 1) {
		filter_attrib = mainGame->cbAttribute->getItemData(mainGame->cbAttribute->getSelected());
		filter_race = mainGame->cbRace->getItemData(mainGame->cbRace->getSelected());
		filter_atk = parse_filter(mainGame->ebAttack->getText(), &filter_atktype);
		filter_def = parse_filter(mainGame->ebDefense->getText(), &filter_deftype);
		filter_lv = parse_filter(mainGame->ebStar->getText(), &filter_lvtype);
		filter_scl = parse_filter(mainGame->ebScale->getText(), &filter_scltype);
	}
	FilterCards();
}
int DeckBuilder::GetSearchResultMaxVisible() const {
	float xS = mainGame->xScale, yS = mainGame->yScale;
	float mul = (xS > yS) ? yS : xS;
	return std::max(7, (int)(((626.0f - 165.0f) * yS) / (66.0f * mul)));
}
void DeckBuilder::FilterCards() {
	results.clear();
	struct element_t {
		std::wstring keyword;
		unsigned int setcode;
		enum class type_t {
			all,
			name,
			setcode
		} type;
		bool exclude;
		element_t(): setcode(0), type(type_t::all), exclude(false) {}
	};
	const wchar_t* pstr = mainGame->ebCardName->getText();
	std::wstring str = std::wstring(pstr);
	std::vector<element_t> query_elements;
	if(mainGame->gameConf.search_multiple_keywords) {
		const wchar_t separator = mainGame->gameConf.search_multiple_keywords == 1 ? L' ' : L'+';
		const wchar_t minussign = L'-';
		const wchar_t quotation = L'\"';
		size_t element_start = 0;
		for(;;) {
			element_start = str.find_first_not_of(separator, element_start);
			if(element_start == std::wstring::npos)
				break;
			element_t element;
			if(str[element_start] == minussign) {
				element.exclude = true;
				element_start++;
			}
			if(element_start >= str.size())
				break;
			if(str[element_start] == L'$') {
				element.type = element_t::type_t::name;
				element_start++;
			} else if(str[element_start] == L'@') {
				element.type = element_t::type_t::setcode;
				element_start++;
			}
			if(element_start >= str.size())
				break;
			wchar_t delimiter = separator;
			if(str[element_start] == quotation) {
				delimiter = quotation;
				element_start++;
			}
			size_t element_end = str.find_first_of(delimiter, element_start);
			if(element_end != std::wstring::npos) {
				size_t length = element_end - element_start;
				element.keyword = str.substr(element_start, length);
			} else
				element.keyword = str.substr(element_start);
			element.setcode = dataManager.GetSetCode(element.keyword.c_str());
			query_elements.push_back(element);
			if(element_end == std::wstring::npos)
				break;
			element_start = element_end + 1;
		}
	} else {
		element_t element;
		size_t element_start = 0;
		if(str[element_start] == L'$') {
			element.type = element_t::type_t::name;
			element_start++;
		} else if(str[element_start] == L'@') {
			element.type = element_t::type_t::setcode;
			element_start++;
		}
		if(element_start < str.size()) {
			element.keyword = str.substr(element_start);
			element.setcode = dataManager.GetSetCode(element.keyword.c_str());
			query_elements.push_back(element);
		}
	}
	for(code_pointer ptr = dataManager.datas_begin; ptr != dataManager.datas_end; ++ptr) {
		const CardDataC& data = ptr->second;
		auto strpointer = dataManager.GetStringPointer(ptr->first);
		if (strpointer == dataManager.strings_end)
			continue;
		const CardString& text = strpointer->second;
		if(data.type & TYPE_TOKEN)
			continue;
		switch(filter_type) {
		case 1: {
			if(!(data.type & TYPE_MONSTER) || (data.type & filter_type2) != filter_type2)
				continue;
			if(filter_race && data.race != filter_race)
				continue;
			if(filter_attrib && data.attribute != filter_attrib)
				continue;
			if(filter_atktype) {
				if((filter_atktype == 1 && data.attack != filter_atk) || (filter_atktype == 2 && data.attack < filter_atk)
				        || (filter_atktype == 3 && data.attack <= filter_atk) || (filter_atktype == 4 && (data.attack > filter_atk || data.attack < 0))
				        || (filter_atktype == 5 && (data.attack >= filter_atk || data.attack < 0)) || (filter_atktype == 6 && data.attack != -2))
					continue;
			}
			if(filter_deftype) {
				if((filter_deftype == 1 && data.defense != filter_def) || (filter_deftype == 2 && data.defense < filter_def)
				        || (filter_deftype == 3 && data.defense <= filter_def) || (filter_deftype == 4 && (data.defense > filter_def || data.defense < 0))
				        || (filter_deftype == 5 && (data.defense >= filter_def || data.defense < 0)) || (filter_deftype == 6 && data.defense != -2)
				        || (data.type & TYPE_LINK))
					continue;
			}
			if(filter_lvtype) {
				if((filter_lvtype == 1 && data.level != filter_lv) || (filter_lvtype == 2 && data.level < filter_lv)
				        || (filter_lvtype == 3 && data.level <= filter_lv) || (filter_lvtype == 4 && data.level > filter_lv)
				        || (filter_lvtype == 5 && data.level >= filter_lv) || filter_lvtype == 6)
					continue;
			}
			if(filter_scltype) {
				if((filter_scltype == 1 && data.lscale != filter_scl) || (filter_scltype == 2 && data.lscale < filter_scl)
				        || (filter_scltype == 3 && data.lscale <= filter_scl) || (filter_scltype == 4 && (data.lscale > filter_scl || data.lscale == 0))
				        || (filter_scltype == 5 && (data.lscale >= filter_scl || data.lscale == 0)) || filter_scltype == 6
				        || !(data.type & TYPE_PENDULUM))
					continue;
			}
			break;
		}
		case 2: {
			if(!(data.type & TYPE_SPELL))
				continue;
			if(filter_type2 && data.type != filter_type2)
				continue;
			break;
		}
		case 3: {
			if(!(data.type & TYPE_TRAP))
				continue;
			if(filter_type2 && data.type != filter_type2)
				continue;
			break;
		}
		}
		if(filter_effect && !(data.category & filter_effect))
			continue;
		if(filter_marks && (data.link_marker & filter_marks)!= filter_marks)
			continue;
		if(filter_lm) {
			if(filter_lm <= 3 && (!filterList->count(ptr->first) || (*filterList).at(ptr->first) != filter_lm - 1))
				continue;
			if(filter_lm == 4 && data.ot != 1)
				continue;
			if(filter_lm == 5 && data.ot != 2)
				continue;
			if(filter_lm == 6 && data.ot != 3)
				continue;
			if(filter_lm == 7 && data.ot != 4)
				continue;
		}
		bool is_target = true;
		for (auto elements_iterator = query_elements.begin(); elements_iterator != query_elements.end(); ++elements_iterator) {
			bool match = false;
			if (elements_iterator->type == element_t::type_t::name) {
				match = CardNameContains(text.name.c_str(), elements_iterator->keyword.c_str());
			} else if (elements_iterator->type == element_t::type_t::setcode) {
				match = elements_iterator->setcode && data.is_setcode(elements_iterator->setcode);
			} else {
				int trycode = BufferIO::GetVal(elements_iterator->keyword.c_str());
				bool tryresult = dataManager.GetData(trycode, 0);
				if(!tryresult) {
					match = CardNameContains(text.name.c_str(), elements_iterator->keyword.c_str())
						|| text.text.find(elements_iterator->keyword) != std::wstring::npos
						|| mainGame->CheckRegEx(text.text, elements_iterator->keyword)
						|| (elements_iterator->setcode && data.is_setcode(elements_iterator->setcode));
				} else {
					match = data.code == trycode || data.alias == trycode;
				}
			}
			if(elements_iterator->exclude)
				match = !match;
			if(!match) {
				is_target = false;
				break;
			}
		}
		if(is_target)
			results.push_back(ptr);
		else
			continue;
	}
	myswprintf(result_string, L"%d", results.size());
	{
		int max_vis = GetSearchResultMaxVisible();
		if(results.size() > (size_t)max_vis) {
			mainGame->scrFilter->setVisible(true);
			mainGame->scrFilter->setMax((int)results.size() - max_vis);
			mainGame->scrFilter->setPos(0);
		} else {
			mainGame->scrFilter->setVisible(false);
			mainGame->scrFilter->setPos(0);
		}
	}
	SortList();
}
void DeckBuilder::InstantSearch() {
	if(mainGame->gameConf.auto_search_limit >= 0 && ((int)wcslen(mainGame->ebCardName->getText()) >= mainGame->gameConf.auto_search_limit))
		StartFilter();
}
void DeckBuilder::ClearSearch() {
	mainGame->cbCardType->setSelected(0);
	mainGame->cbCardType2->setSelected(0);
	mainGame->cbCardType2->setEnabled(false);
	mainGame->cbRace->setEnabled(false);
	mainGame->cbAttribute->setEnabled(false);
	mainGame->ebAttack->setEnabled(false);
	mainGame->ebDefense->setEnabled(false);
	mainGame->ebStar->setEnabled(false);
	mainGame->ebScale->setEnabled(false);
	mainGame->ebCardName->setText(L"");
	mainGame->scrFilter->setVisible(false);
	mainGame->scrFilter->setPos(0);
	ClearFilter();
	results.clear();
	myswprintf(result_string, L"%d", 0);
}
void DeckBuilder::ClearFilter() {
	mainGame->cbAttribute->setSelected(0);
	mainGame->cbRace->setSelected(0);
	mainGame->cbLimit->setSelected(0);
	mainGame->ebAttack->setText(L"");
	mainGame->ebDefense->setText(L"");
	mainGame->ebStar->setText(L"");
	mainGame->ebScale->setText(L"");
	filter_effect = 0;
	for(int i = 0; i < 32; ++i)
		mainGame->chkCategory[i]->setChecked(false);
	filter_marks = 0;
	for(int i = 0; i < 8; i++)
		mainGame->btnMark[i]->setPressed(false);
	mainGame->btnEffectFilter->setPressed(false);
	mainGame->btnMarksFilter->setPressed(false);
}
void DeckBuilder::SortList() {
	auto left = results.begin();
	const wchar_t* pstr = mainGame->ebCardName->getText();
	for(auto it = results.begin(); it != results.end(); ++it) {
		if(wcscmp(pstr, dataManager.GetName((*it)->first)) == 0 || mainGame->CheckRegEx(dataManager.GetName((*it)->first), pstr, true)) {
			std::iter_swap(left, it);
			++left;
		}
	}
	switch(mainGame->cbSortType->getSelected()) {
	case 0:
		std::sort(left, results.end(), ClientCard::deck_sort_lv);
		break;
	case 1:
		std::sort(left, results.end(), ClientCard::deck_sort_atk);
		break;
	case 2:
		std::sort(left, results.end(), ClientCard::deck_sort_def);
		break;
	case 3:
		std::sort(left, results.end(), ClientCard::deck_sort_name);
		break;
	}
}
static inline wchar_t NormalizeChar(wchar_t c) {
	/*
	// Convert all symbols and punctuations to space.
	if (c != 0 && c < 128 && !isalnum(c)) {
		return ' ';
	}
	*/
	// Convert latin chararacters to uppercase to ignore case.
	if (c < 128 && isalpha(c)) {
		return toupper(c);
	}
	// Remove some accentued characters that are not supported by the editbox.
	if (c >= 232 && c <= 235) {
		return 'E';
	}
	if (c >= 238 && c <= 239) {
		return 'I';
	}
	return c;
}
bool DeckBuilder::CardNameContains(const wchar_t *haystack, const wchar_t *needle)
{
	if (!needle[0]) {
		return true;
	}
	if (!haystack) {
		return false;
	}
	if(mainGame->CheckRegEx(haystack, needle))
		return true;
	int i = 0;
	int j = 0;
	while (haystack[i]) {
		wchar_t ca = NormalizeChar(haystack[i]);
		wchar_t cb = NormalizeChar(needle[j]);
		if (ca == cb) {
			j++;
			if (!needle[j]) {
				return true;
			}
		} else {
			i -= j;
			j = 0;
		}
		i++;
	}
	return false;
}
bool DeckBuilder::push_main(code_pointer pointer, int seq) {
	if(pointer->second.type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_XYZ | TYPE_LINK))
		return false;
	auto& container = deckManager.current_deck.main;
	int maxc = mainGame->is_siding ? 64 : 60;
	if((int)container.size() >= maxc)
		return false;
	if(seq >= 0 && seq < (int)container.size())
		container.insert(container.begin() + seq, pointer);
	else
		container.push_back(pointer);
	is_modified = true;
	GetHoveredCard();
	return true;
}
bool DeckBuilder::push_extra(code_pointer pointer, int seq) {
	if(!(pointer->second.type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_XYZ | TYPE_LINK)))
		return false;
	auto& container = deckManager.current_deck.extra;
	int maxc = mainGame->is_siding ? 20 : 15;
	if((int)container.size() >= maxc)
		return false;
	if(seq >= 0 && seq < (int)container.size())
		container.insert(container.begin() + seq, pointer);
	else
		container.push_back(pointer);
	is_modified = true;
	GetHoveredCard();
	return true;
}
bool DeckBuilder::push_side(code_pointer pointer, int seq) {
	auto& container = deckManager.current_deck.side;
	int maxc = mainGame->is_siding ? 20 : 15;
	if((int)container.size() >= maxc)
		return false;
	if(seq >= 0 && seq < (int)container.size())
		container.insert(container.begin() + seq, pointer);
	else
		container.push_back(pointer);
	is_modified = true;
	GetHoveredCard();
	return true;
}
void DeckBuilder::pop_main(int seq) {
	auto& container = deckManager.current_deck.main;
	container.erase(container.begin() + seq);
	is_modified = true;
	GetHoveredCard();
}
void DeckBuilder::pop_extra(int seq) {
	auto& container = deckManager.current_deck.extra;
	container.erase(container.begin() + seq);
	is_modified = true;
	GetHoveredCard();
}
void DeckBuilder::pop_side(int seq) {
	auto& container = deckManager.current_deck.side;
	container.erase(container.begin() + seq);
	is_modified = true;
	GetHoveredCard();
}
bool DeckBuilder::check_limit(code_pointer pointer) {
	unsigned int limitcode = pointer->second.alias ? pointer->second.alias : pointer->first;
	int limit = 3;
	auto flit = filterList->find(limitcode);
	if(flit != filterList->end())
		limit = flit->second;
	for(auto it = deckManager.current_deck.main.begin(); it != deckManager.current_deck.main.end(); ++it) {
		if((*it)->first == limitcode || (*it)->second.alias == limitcode)
			limit--;
	}
	for(auto it = deckManager.current_deck.extra.begin(); it != deckManager.current_deck.extra.end(); ++it) {
		if((*it)->first == limitcode || (*it)->second.alias == limitcode)
			limit--;
	}
	for(auto it = deckManager.current_deck.side.begin(); it != deckManager.current_deck.side.end(); ++it) {
		if((*it)->first == limitcode || (*it)->second.alias == limitcode)
			limit--;
	}
	return limit > 0;
}
}
