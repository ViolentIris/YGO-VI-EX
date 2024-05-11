#include "deck_manager.h"
#include "data_manager.h"
#include "network.h"
#include "game.h"

namespace ygo {

char DeckManager::deckBuffer[0x10000]{};
DeckManager deckManager;

void DeckManager::LoadLFListSingle(const char* path) {
	LFList* cur = nullptr;
	FILE* fp = fopen(path, "r");
	char linebuf[256]{};
	wchar_t strBuffer[256]{};
	if(fp) {
		while(fgets(linebuf, 256, fp)) {
			if(linebuf[0] == '#')
				continue;
			if(linebuf[0] == '!') {
				int sa = BufferIO::DecodeUTF8(&linebuf[1], strBuffer);
				while(strBuffer[sa - 1] == L'\r' || strBuffer[sa - 1] == L'\n' )
					sa--;
				strBuffer[sa] = 0;
				LFList newlist;
				_lfList.push_back(newlist);
				cur = &_lfList[_lfList.size() - 1];
				cur->listName = strBuffer;
				cur->hash = 0x7dfcee6a;
				continue;
			}
			if(linebuf[0] == 0)
				continue;
			int code = 0;
			int count = -1;
			if (sscanf(linebuf, "%d %d", &code, &count) != 2)
				continue;
			if (code <= 0 || code > 99999999)
				continue;
			if (count < 0 || count > 2)
				continue;
			if (!cur)
				continue;
			cur->content[code] = count;
			cur->hash = cur->hash ^ ((code << 18) | (code >> 14)) ^ ((code << (27 + count)) | (code >> (5 - count)));
		}
		fclose(fp);
	}
}
void DeckManager::LoadLFList() {
	LoadLFListSingle("expansions/lflist.conf");
	LoadLFListSingle("lflist.conf");
	LFList nolimit;
	nolimit.listName = L"N/A";
	nolimit.hash = 0;
	_lfList.push_back(nolimit);
}
bool DeckManager::RenameDeck(const wchar_t* oldname, const wchar_t* newname) {
	wchar_t oldfname[256];
	wchar_t newfname[256];
	myswprintf(oldfname, L"./deck/%ls.ydk", oldname);
	myswprintf(newfname, L"./deck/%ls.ydk", newname);
#ifdef WIN32
	BOOL result = MoveFileW(oldfname, newfname);
	return !!result;
#else
	char oldfilefn[256];
	char newfilefn[256];
	BufferIO::EncodeUTF8(oldfname, oldfilefn);
	BufferIO::EncodeUTF8(newfname, newfilefn);
	int result = rename(oldfilefn, newfilefn);
	return result == 0;
#endif
}
const wchar_t* DeckManager::GetLFListName(int lfhash) {
	auto lit = std::find_if(_lfList.begin(), _lfList.end(), [lfhash](const ygo::LFList& list) {
		return list.hash == lfhash;
	});
	if(lit != _lfList.end())
		return lit->listName.c_str();
	return dataManager.unknown_string;
}
const std::unordered_map<int, int>* DeckManager::GetLFListContent(int lfhash) {
	auto lit = std::find_if(_lfList.begin(), _lfList.end(), [lfhash](const ygo::LFList& list) {
		return list.hash == lfhash;
	});
	if(lit != _lfList.end())
		return &lit->content;
	return nullptr;
}
static int checkAvail(int ot, int avail) {
	if((ot & avail) == avail)
		return 0;
	if((ot & AVAIL_OCG) && !(avail == AVAIL_OCG))
		return DECKERROR_OCGONLY;
	if((ot & AVAIL_TCG) && !(avail == AVAIL_TCG))
		return DECKERROR_TCGONLY;
	return DECKERROR_NOTAVAIL;
}
int DeckManager::CheckDeck(Deck& deck, int lfhash, int rule) {
	std::unordered_map<int, int> ccount;
	std::unordered_map<int, int>* list = 0;
	for(size_t i = 0; i < _lfList.size(); ++i) {
		if(_lfList[i].hash == (unsigned int)lfhash) {
			list = &_lfList[i].content;
			break;
		}
	}
	if(!list)
		return 0;
	int dc = 0;
	if(deck.main.size() < 40 || deck.main.size() > 60)
		return (DECKERROR_MAINCOUNT << 28) + deck.main.size();
	if(deck.extra.size() > 15)
		return (DECKERROR_EXTRACOUNT << 28) + deck.extra.size();
	if(deck.side.size() > 15)
		return (DECKERROR_SIDECOUNT << 28) + deck.side.size();
	const int rule_map[6] = { AVAIL_OCG, AVAIL_TCG, AVAIL_SC, AVAIL_CUSTOM, AVAIL_OCGTCG, 0 };
	int avail = rule_map[rule];
	for(size_t i = 0; i < deck.main.size(); ++i) {
		code_pointer cit = deck.main[i];
		int gameruleDeckError = checkAvail(cit->second.ot, avail);
		if(gameruleDeckError)
			return (gameruleDeckError << 28) + cit->first;
		if(cit->second.type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_XYZ | TYPE_TOKEN | TYPE_LINK))
			return (DECKERROR_EXTRACOUNT << 28);
		int code = cit->second.alias ? cit->second.alias : cit->first;
		ccount[code]++;
		dc = ccount[code];
		if(dc > 3)
			return (DECKERROR_CARDCOUNT << 28) + cit->first;
		auto it = list->find(code);
		if(it != list->end() && dc > it->second)
			return (DECKERROR_LFLIST << 28) + cit->first;
	}
	for(size_t i = 0; i < deck.extra.size(); ++i) {
		code_pointer cit = deck.extra[i];
		int gameruleDeckError = checkAvail(cit->second.ot, avail);
		if(gameruleDeckError)
			return (gameruleDeckError << 28) + cit->first;
		int code = cit->second.alias ? cit->second.alias : cit->first;
		ccount[code]++;
		dc = ccount[code];
		if(dc > 3)
			return (DECKERROR_CARDCOUNT << 28) + cit->first;
		auto it = list->find(code);
		if(it != list->end() && dc > it->second)
			return (DECKERROR_LFLIST << 28) + cit->first;
	}
	for(size_t i = 0; i < deck.side.size(); ++i) {
		code_pointer cit = deck.side[i];
		int gameruleDeckError = checkAvail(cit->second.ot, avail);
		if(gameruleDeckError)
			return (gameruleDeckError << 28) + cit->first;
		int code = cit->second.alias ? cit->second.alias : cit->first;
		ccount[code]++;
		dc = ccount[code];
		if(dc > 3)
			return (DECKERROR_CARDCOUNT << 28) + cit->first;
		auto it = list->find(code);
		if(it != list->end() && dc > it->second)
			return (DECKERROR_LFLIST << 28) + cit->first;
	}
	return 0;
}
int DeckManager::LoadDeck(Deck& deck, int* dbuf, int mainc, int sidec) {
	deck.clear();
	int code;
	int errorcode = 0;
	CardData cd;
	for(int i = 0; i < mainc; ++i) {
		code = dbuf[i];
		if(!dataManager.GetData(code, &cd)) {
			errorcode = code;
			continue;
		}
		if(cd.type & TYPE_TOKEN)
			continue;
		else if(cd.type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_XYZ | TYPE_LINK) && deck.extra.size() < 15) {
			if(deck.extra.size() >= 15)
				continue;
			deck.extra.push_back(dataManager.GetCodePointer(code));	//verified by GetData()
		} else if(deck.main.size() < 60) {
			deck.main.push_back(dataManager.GetCodePointer(code));
		}
	}
	for(int i = 0; i < sidec; ++i) {
		code = dbuf[mainc + i];
		if(!dataManager.GetData(code, &cd)) {
			errorcode = code;
			continue;
		}
		if(cd.type & TYPE_TOKEN)
			continue;
		if(deck.side.size() < 15)
			deck.side.push_back(dataManager.GetCodePointer(code));	//verified by GetData()
	}
	return errorcode;
}
bool DeckManager::LoadSide(Deck& deck, int* dbuf, int mainc, int sidec) {
	std::unordered_map<int, int> pcount;
	std::unordered_map<int, int> ncount;
	for(size_t i = 0; i < deck.main.size(); ++i)
		pcount[deck.main[i]->first]++;
	for(size_t i = 0; i < deck.extra.size(); ++i)
		pcount[deck.extra[i]->first]++;
	for(size_t i = 0; i < deck.side.size(); ++i)
		pcount[deck.side[i]->first]++;
	Deck ndeck;
	LoadDeck(ndeck, dbuf, mainc, sidec);
	if(ndeck.main.size() != deck.main.size() || ndeck.extra.size() != deck.extra.size())
		return false;
	for(size_t i = 0; i < ndeck.main.size(); ++i)
		ncount[ndeck.main[i]->first]++;
	for(size_t i = 0; i < ndeck.extra.size(); ++i)
		ncount[ndeck.extra[i]->first]++;
	for(size_t i = 0; i < ndeck.side.size(); ++i)
		ncount[ndeck.side[i]->first]++;
	for(auto cdit = ncount.begin(); cdit != ncount.end(); ++cdit)
		if(cdit->second != pcount[cdit->first])
			return false;
	deck = ndeck;
	return true;
}
FILE* DeckManager::OpenDeckFile(const wchar_t* file, const char* mode) {
#ifdef WIN32
	FILE* fp = _wfopen(file, (wchar_t*)mode);
#else
	char file2[256];
	BufferIO::EncodeUTF8(file, file2);
	FILE* fp = fopen(file2, mode);
#endif
	return fp;
}
bool DeckManager::LoadDeck(const wchar_t* file) {
	int sp = 0, ct = 0, mainc = 0, sidec = 0, code;
	wchar_t localfile[64];
	myswprintf(localfile, L"./deck/%ls.ydk", file);
	FILE* fp = OpenDeckFile(localfile, "r");
	if(!fp) {
		fp = OpenDeckFile(file, "r");
	}
	if(!fp)
		return false;
	int cardlist[128];
	bool is_side = false;
	char linebuf[256];
	while(fgets(linebuf, 256, fp) && ct < 128) {
		if(linebuf[0] == '!') {
			is_side = true;
			continue;
		}
		if(linebuf[0] < '0' || linebuf[0] > '9')
			continue;
		sp = 0;
		while(linebuf[sp] >= '0' && linebuf[sp] <= '9') sp++;
		linebuf[sp] = 0;
		code = atoi(linebuf);
		cardlist[ct++] = code;
		if(is_side) sidec++;
		else mainc++;
	}
	fclose(fp);
	LoadDeck(current_deck, cardlist, mainc, sidec);
	return true;
}
bool DeckManager::SaveDeck(Deck& deck, const wchar_t* name) {
	if(!FileSystem::IsDirExists(L"./deck") && !FileSystem::MakeDir(L"./deck"))
		return false;
	wchar_t file[64];
	myswprintf(file, L"./deck/%ls.ydk", name);
	FILE* fp = OpenDeckFile(file, "w");
	if(!fp)
		return false;
	fprintf(fp, "#created by ...\n#main\n");
	for(size_t i = 0; i < deck.main.size(); ++i)
		fprintf(fp, "%d\n", deck.main[i]->first);
	fprintf(fp, "#extra\n");
	for(size_t i = 0; i < deck.extra.size(); ++i)
		fprintf(fp, "%d\n", deck.extra[i]->first);
	fprintf(fp, "!side\n");
	for(size_t i = 0; i < deck.side.size(); ++i)
		fprintf(fp, "%d\n", deck.side[i]->first);
	fclose(fp);
	return true;
}
bool DeckManager::DeleteDeck(Deck& deck, const wchar_t* name) {
	wchar_t file[64];
	myswprintf(file, L"./deck/%ls.ydk", name);
#ifdef WIN32
	BOOL result = DeleteFileW(file);
	return !!result;
#else
	char filefn[256];
	BufferIO::EncodeUTF8(file, filefn);
	int result = unlink(filefn);
	return result == 0;
#endif
}
const wchar_t* DeckManager::GetMainFormatString() {
	myswprintf(DeckFormatBuffer, L"%d    ( %d / %d / %d )", deckManager.current_deck.main.size(), GetTypeCount(deckManager.current_deck.main, TYPE_MONSTER), GetTypeCount(deckManager.current_deck.main, TYPE_SPELL), GetTypeCount(deckManager.current_deck.main, TYPE_TRAP));
	wchar_t* result = DeckFormatBuffer;
	return result;
}
const wchar_t* DeckManager::GetExtraFormatString() {
	myswprintf(DeckFormatBuffer, L"%d    ( %d / %d / %d / %d )", deckManager.current_deck.extra.size(), GetTypeCount(deckManager.current_deck.extra, TYPE_FUSION), GetTypeCount(deckManager.current_deck.extra, TYPE_SYNCHRO), GetTypeCount(deckManager.current_deck.extra, TYPE_XYZ), GetTypeCount(deckManager.current_deck.extra, TYPE_LINK));
	wchar_t* result = DeckFormatBuffer;
	return result;
}
const wchar_t* DeckManager::GetSideFormatString() {
	myswprintf(DeckFormatBuffer, L"%d    ( %d / %d / %d / %d )", deckManager.current_deck.side.size(), GetTypeCount(deckManager.current_deck.side, TYPE_MONSTER), GetTypeCount(deckManager.current_deck.side, TYPE_SPELL), GetTypeCount(deckManager.current_deck.side, TYPE_TRAP), GetTypeCount(deckManager.current_deck.side, TYPE_FUSION | TYPE_SYNCHRO | TYPE_XYZ | TYPE_LINK));
	wchar_t* result = DeckFormatBuffer;
	return result;
}
int DeckManager::GetTypeCount(std::vector<code_pointer> list, unsigned int ctype) {
	int res = 0;
	for(size_t i = 0; i < list.size(); ++i) {
		code_pointer cur = list[i];
		if(cur->second.type & ctype)
			res++;
	}
	return res;
}
}
