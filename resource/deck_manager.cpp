#include "deck_manager.h"
#include "game.h"
#include "myfilesystem.h"
#include "network.h"
#include "base64.h"

namespace ygo {

char DeckManager::deckBuffer[0x10000]{};
DeckManager deckManager;

void DeckManager::LoadLFListSingle(const char* path, bool insert) {
	FILE* fp = myfopen(path, "r");
	if (!fp) return;
	_LoadLFListFromLineProvider([&](char* buf, size_t sz) {
		return std::fgets(buf, sz, fp) != nullptr;
	}, insert);
	std::fclose(fp);
}
void DeckManager::LoadLFListSingle(const wchar_t* path, bool insert) {
	FILE* fp = mywfopen(path, "r");
	if (!fp) return;
	_LoadLFListFromLineProvider([&](char* buf, size_t sz) {
		return std::fgets(buf, sz, fp) != nullptr;
	}, insert);
	std::fclose(fp);
}
void DeckManager::LoadLFListSingle(irr::io::IReadFile* reader, bool insert) {
	std::string linebuf;
	char ch{};
	_LoadLFListFromLineProvider([&](char* buf, size_t sz) {
		while (reader->read(&ch, 1)) {
			if (ch == '\0') break;
			linebuf.push_back(ch);
			if (ch == '\n' || linebuf.size() >= sz - 1) {
				std::strncpy(buf, linebuf.c_str(), sz - 1);
				buf[sz - 1] = '\0';
				linebuf.clear();
				return true;
			}
		}
		return false;
	}, insert);
	reader->drop();
}
void DeckManager::LoadLFList() {
	LoadLFListSingle("specials/lflist.conf");
	LoadLFListSingle("lflist.conf");
	LFList nolimit;
	nolimit.listName = L"N/A";
	nolimit.hash = 0;
	_lfList.push_back(nolimit);
}
const wchar_t* DeckManager::GetLFListName(unsigned int lfhash) {
	auto lit = std::find_if(_lfList.begin(), _lfList.end(), [lfhash](const ygo::LFList& list) {
		return list.hash == lfhash;
	});
	if(lit != _lfList.end())
		return lit->listName.c_str();
	return dataManager.unknown_string;
}
const LFList* DeckManager::GetLFList(unsigned int lfhash) {
	auto lit = std::find_if(_lfList.begin(), _lfList.end(), [lfhash](const ygo::LFList& list) {
		return list.hash == lfhash;
	});
	if (lit != _lfList.end())
		return &(*lit);
	return nullptr;
}
static unsigned int checkAvail(unsigned int ot, unsigned int avail) {
	if(!!(ot & 0x4))
		return 0;
	if((ot & avail) == avail)
		return 0;
	if((ot & AVAIL_OCG) && (avail != AVAIL_OCG))
		return DECKERROR_OCGONLY;
	if((ot & AVAIL_TCG) && (avail != AVAIL_TCG))
		return DECKERROR_TCGONLY;
	return DECKERROR_NOTAVAIL;
}
unsigned int DeckManager::CheckDeck(const Deck& deck, unsigned int lfhash, int rule) {
	std::unordered_map<int, int> ccount;
	// rule
	if(deck.main.size() < DECK_MIN_SIZE || deck.main.size() > DECK_MAX_SIZE)
		return (DECKERROR_MAINCOUNT << 28) | (unsigned)deck.main.size();
	if(deck.extra.size() > EXTRA_MAX_SIZE)
		return (DECKERROR_EXTRACOUNT << 28) | (unsigned)deck.extra.size();
	if(deck.side.size() > SIDE_MAX_SIZE)
		return (DECKERROR_SIDECOUNT << 28) | (unsigned)deck.side.size();
	auto lflist = GetLFList(lfhash);
	if (!lflist)
		return 0;
	auto& list = lflist->content;
	const unsigned int rule_map[6] = { AVAIL_OCG, AVAIL_TCG, AVAIL_SC, AVAIL_CUSTOM, AVAIL_OCGTCG, 0 };
	unsigned int avail = 0;
	if (rule >= 0 && rule < (int)(sizeof rule_map / sizeof rule_map[0]))
		avail = rule_map[rule];
	for (auto& cit : deck.main) {
		auto gameruleDeckError = checkAvail(cit->second.ot, avail);
		if(gameruleDeckError)
			return (gameruleDeckError << 28) | cit->first;
		if (cit->second.type & (TYPES_EXTRA_DECK | TYPE_TOKEN))
			return (DECKERROR_MAINCOUNT << 28);
		int code = cit->second.alias ? cit->second.alias : cit->first;
		ccount[code]++;
		int dc = ccount[code];
		if(dc > 3)
			return (DECKERROR_CARDCOUNT << 28) | cit->first;
		auto it = list.find(code);
		if(it != list.end() && dc > it->second)
			return (DECKERROR_LFLIST << 28) | cit->first;
	}
	for (auto& cit : deck.extra) {
		auto gameruleDeckError = checkAvail(cit->second.ot, avail);
		if(gameruleDeckError)
			return (gameruleDeckError << 28) | cit->first;
		if (!(cit->second.type & TYPES_EXTRA_DECK) || cit->second.type & TYPE_TOKEN)
			return (DECKERROR_EXTRACOUNT << 28);
		int code = cit->second.alias ? cit->second.alias : cit->first;
		ccount[code]++;
		int dc = ccount[code];
		if(dc > 3)
			return (DECKERROR_CARDCOUNT << 28) | cit->first;
		auto it = list.find(code);
		if(it != list.end() && dc > it->second)
			return (DECKERROR_LFLIST << 28) | cit->first;
	}
	for (auto& cit : deck.side) {
		auto gameruleDeckError = checkAvail(cit->second.ot, avail);
		if(gameruleDeckError)
			return (gameruleDeckError << 28) | cit->first;
		if (cit->second.type & TYPE_TOKEN)
			return (DECKERROR_SIDECOUNT << 28);
		int code = cit->second.alias ? cit->second.alias : cit->first;
		ccount[code]++;
		int dc = ccount[code];
		if(dc > 3)
			return (DECKERROR_CARDCOUNT << 28) | cit->first;
		auto it = list.find(code);
		if(it != list.end() && dc > it->second)
			return (DECKERROR_LFLIST << 28) | cit->first;
	}
	return 0;
}
uint32_t DeckManager::LoadDeck(Deck& deck, uint32_t dbuf[], int mainc, int sidec, bool is_packlist) {
	deck.clear();
	uint32_t errorcode = 0;
	CardData cd;
	for(int i = 0; i < mainc; ++i) {
		auto code = dbuf[i];
		if(!dataManager.GetData(code, &cd)) {
			errorcode = code;
			continue;
		}
		if (cd.type & TYPE_TOKEN) {
			errorcode = code;
			continue;
		}
		if(is_packlist) {
			deck.main.push_back(dataManager.GetCodePointer(code));
			continue;
		}
		if (cd.type & TYPES_EXTRA_DECK) {
			if (deck.extra.size() < EXTRA_MAX_SIZE)
				deck.extra.push_back(dataManager.GetCodePointer(code));
		}
		else {
			if (deck.main.size() < DECK_MAX_SIZE)
				deck.main.push_back(dataManager.GetCodePointer(code));
		}
	}
	for(int i = 0; i < sidec; ++i) {
		auto code = dbuf[mainc + i];
		if(!dataManager.GetData(code, &cd)) {
			errorcode = code;
			continue;
		}
		if (cd.type & TYPE_TOKEN) {
			errorcode = code;
			continue;
		}
		if(deck.side.size() < SIDE_MAX_SIZE)
			deck.side.push_back(dataManager.GetCodePointer(code));
	}
	return errorcode;
}
uint32_t DeckManager::LoadDeckFromStream(Deck& deck, std::istringstream& deckStream, bool is_packlist) {
	int ct = 0;
	int mainc = 0, sidec = 0;
	uint32_t cardlist[PACK_MAX_SIZE]{};
	bool is_side = false;
	std::string linebuf;
	while (std::getline(deckStream, linebuf, '\n') && ct < PACK_MAX_SIZE) {
		if (linebuf[0] == '!') {
			is_side = true;
			continue;
		}
		if (linebuf[0] < '0' || linebuf[0] > '9')
			continue;
		auto code = std::strtoul(linebuf.c_str(), nullptr, 10);
		if (code >= UINT32_MAX)
			continue;
		cardlist[ct++] = code;
		if (is_side)
			++sidec;
		else
			++mainc;
	}
	return LoadDeck(deck, cardlist, mainc, sidec, is_packlist);
}
bool DeckManager::LoadSide(Deck& deck, uint32_t dbuf[], int mainc, int sidec) {
	std::unordered_map<uint32_t, int> pcount;
	std::unordered_map<uint32_t, int> ncount;
	for(size_t i = 0; i < deck.main.size(); ++i)
		pcount[deck.main[i]->first]++;
	for(size_t i = 0; i < deck.extra.size(); ++i)
		pcount[deck.extra[i]->first]++;
	for(size_t i = 0; i < deck.side.size(); ++i)
		pcount[deck.side[i]->first]++;
	Deck ndeck;
	LoadDeck(ndeck, dbuf, mainc, sidec);
#ifndef YGOPRO_NO_SIDE_CHECK
	if(ndeck.main.size() != deck.main.size() || ndeck.extra.size() != deck.extra.size() || ndeck.side.size() != deck.side.size())
		return false;
#endif
	for(size_t i = 0; i < ndeck.main.size(); ++i)
		ncount[ndeck.main[i]->first]++;
	for(size_t i = 0; i < ndeck.extra.size(); ++i)
		ncount[ndeck.extra[i]->first]++;
	for(size_t i = 0; i < ndeck.side.size(); ++i)
		ncount[ndeck.side[i]->first]++;
#ifndef YGOPRO_NO_SIDE_CHECK
	for (auto& cdit : ncount)
		if (cdit.second != pcount[cdit.first])
			return false;
#endif
	deck = ndeck;
	return true;
}
void DeckManager::GetCategoryPath(wchar_t* ret, int index, const wchar_t* text) {
	wchar_t catepath[256];
	switch(index) {
	case 0:
		myswprintf(catepath, L"./pack");
		break;
	case 1:
		BufferIO::CopyWideString(mainGame->gameConf.bot_deck_path, catepath);
		break;
	case -1:
	case 2:
	case 3:
		myswprintf(catepath, L"./deck");
		break;
	default:
		myswprintf(catepath, L"./deck/%ls", text);
	}
	BufferIO::CopyWStr(catepath, ret, 256);
}
void DeckManager::GetDeckFile(wchar_t* ret, int category_index, const wchar_t* category_name, const wchar_t* deckname) {
	wchar_t filepath[256];
	wchar_t catepath[256];
	if(deckname != nullptr) {
		GetCategoryPath(catepath, category_index, category_name);
		myswprintf(filepath, L"%ls/%ls.ydk", catepath, deckname);
		BufferIO::CopyWStr(filepath, ret, 256);
	}
	else {
		BufferIO::CopyWStr(L"", ret, 256);
	}
}
FILE* DeckManager::OpenDeckFile(const wchar_t* file, const char* mode) {
	FILE* fp = mywfopen(file, mode);
	return fp;
}
irr::io::IReadFile* DeckManager::OpenDeckReader(const wchar_t* file) {
#ifdef _WIN32
	auto reader = DataManager::FileSystem->createAndOpenFile(file);
#else
	char file2[256];
	BufferIO::EncodeUTF8(file, file2);
	auto reader = DataManager::FileSystem->createAndOpenFile(file2);
#endif
	return reader;
}
bool DeckManager::LoadCurrentDeck(std::istringstream& deckStream, bool is_packlist) {
	LoadDeckFromStream(current_deck, deckStream, is_packlist);
	return true;  // the above LoadDeck has return value but we ignore it here for now
}
bool DeckManager::LoadCurrentDeck(const wchar_t* file, bool is_packlist) {
	current_deck.clear();
	auto reader = OpenDeckReader(file);
	if(!reader) {
		wchar_t localfile[256];
		myswprintf(localfile, L"./deck/%ls.ydk", file);
		reader = OpenDeckReader(localfile);
	}
	if(!reader && !mywcsncasecmp(file, L"./pack", 6)) {
		wchar_t zipfile[256];
		myswprintf(zipfile, L"%ls", file + 2);
		reader = OpenDeckReader(zipfile);
	}
	if(!reader)
		return false;
	std::memset(deckBuffer, 0, sizeof deckBuffer);
	int size = reader->read(deckBuffer, sizeof deckBuffer);
	reader->drop();
	if (size >= (int)sizeof deckBuffer) {
		return false;
	}
	std::istringstream deckStream(deckBuffer);
	LoadDeckFromStream(current_deck, deckStream, is_packlist);
	return true;  // the above function has return value but we ignore it here for now
}
bool DeckManager::LoadCurrentDeck(int category_index, const wchar_t* category_name, const wchar_t* deckname) {
	wchar_t filepath[256];
	GetDeckFile(filepath, category_index, category_name, deckname);
	bool is_packlist = (category_index == 0);
	bool res = LoadCurrentDeck(filepath, is_packlist);
	if (res && mainGame->is_building)
		mainGame->deckBuilder.RefreshPackListScroll();
	return res;
}
void DeckManager::SaveDeck(const Deck& deck, std::stringstream& deckStream) {
	deckStream << "#created by ..." << std::endl;
	deckStream << "#main" << std::endl;
	for(size_t i = 0; i < deck.main.size(); ++i)
		deckStream << deck.main[i]->first << std::endl;
	deckStream << "#extra" << std::endl;
	for(size_t i = 0; i < deck.extra.size(); ++i)
		deckStream << deck.extra[i]->first << std::endl;
	deckStream << "!side" << std::endl;
	for(size_t i = 0; i < deck.side.size(); ++i)
		deckStream << deck.side[i]->first << std::endl;
}
bool DeckManager::SaveDeck(const Deck& deck, const wchar_t* file) {
	if(!FileSystem::IsDirExists(L"./deck") && !FileSystem::MakeDir(L"./deck"))
		return false;
	FILE* fp = OpenDeckFile(file, "w");
	if(!fp)
		return false;
	std::stringstream deckStream;
	SaveDeck(deck, deckStream);
	std::fwrite(deckStream.str().c_str(), 1, deckStream.str().length(), fp);
	std::fclose(fp);
	return true;
}
bool DeckManager::DeleteDeck(const wchar_t* file) {
	return FileSystem::RemoveFile(file);
}
int DeckManager::TypeCount(std::vector<code_pointer> list, unsigned int ctype) {
	int res = 0;
	for(size_t i = 0; i < list.size(); ++i) {
		code_pointer cur = list[i];
		if(cur->second.type & ctype)
			res++;
	}
	return res;
}
bool DeckManager::LoadDeckFromCode(Deck& deck, const unsigned char *code, int len) {
	unsigned char data[1024], *pdeck = data, *data_ = data;
	int decoded_len = Base64::DecodedLength(code, len);
	if(decoded_len > 1024 || decoded_len < 8 || !Base64::Decode(code, len, data_, decoded_len))
		return false;
	int mainc = BufferIO::ReadInt32(pdeck);
	int sidec = BufferIO::ReadInt32(pdeck);
	int errorcode = LoadDeck(deck, (uint32_t*)pdeck, mainc, sidec);
	return (errorcode == 0);
}
int DeckManager::SaveDeckToCode(Deck& deck, unsigned char* code) {
	unsigned char deckbuf[1024], *pdeck = deckbuf;
	BufferIO::WriteInt32(pdeck, deck.main.size() + deck.extra.size());
	BufferIO::WriteInt32(pdeck, deck.side.size());
	for(size_t i = 0; i < deck.main.size(); ++i)
		BufferIO::WriteInt32(pdeck, deck.main[i]->first);
	for(size_t i = 0; i < deck.extra.size(); ++i)
		BufferIO::WriteInt32(pdeck, deck.extra[i]->first);
	for(size_t i = 0; i < deck.side.size(); ++i)
		BufferIO::WriteInt32(pdeck, deck.side[i]->first);
	int len = pdeck - deckbuf;
	int encoded_len = Base64::EncodedLength(len);
	Base64::Encode(deckbuf, len, code, encoded_len+1);
	return encoded_len;
}
bool DeckManager::CreateCategory(const wchar_t* name) {
	if(!FileSystem::IsDirExists(L"./deck") && !FileSystem::MakeDir(L"./deck"))
		return false;
	if(name[0] == 0)
		return false;
	wchar_t localname[256];
	myswprintf(localname, L"./deck/%ls", name);
	return FileSystem::MakeDir(localname);
}
bool DeckManager::RenameCategory(const wchar_t* oldname, const wchar_t* newname) {
	if(!FileSystem::IsDirExists(L"./deck") && !FileSystem::MakeDir(L"./deck"))
		return false;
	if(newname[0] == 0)
		return false;
	wchar_t oldlocalname[256];
	wchar_t newlocalname[256];
	myswprintf(oldlocalname, L"./deck/%ls", oldname);
	myswprintf(newlocalname, L"./deck/%ls", newname);
	return FileSystem::Rename(oldlocalname, newlocalname);
}
bool DeckManager::DeleteCategory(const wchar_t* name) {
	wchar_t localname[256];
	myswprintf(localname, L"./deck/%ls", name);
	if(!FileSystem::IsDirExists(localname))
		return false;
	return FileSystem::DeleteDir(localname);
}
bool DeckManager::SaveDeckArray(const DeckArray& deck, const wchar_t* name) {
	if (!FileSystem::IsDirExists(L"./deck") && !FileSystem::MakeDir(L"./deck"))
		return false;
	FILE* fp = OpenDeckFile(name, "w");
	if (!fp)
		return false;
	std::fprintf(fp, "#created by ...\n#main\n");
	for (const auto& code : deck.main)
		std::fprintf(fp, "%u\n", code);
	std::fprintf(fp, "#extra\n");
	for (const auto& code : deck.extra)
		std::fprintf(fp, "%u\n", code);
	std::fprintf(fp, "!side\n");
	for (const auto& code : deck.side)
		std::fprintf(fp, "%u\n", code);
	std::fclose(fp);
	return true;
}
}
