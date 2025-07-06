#ifndef DECKMANAGER_H
#define DECKMANAGER_H

#include "config.h"
#include "client_card.h"
#include <unordered_map>
#include <vector>
#include <sstream>

namespace ygo {
	constexpr int DECK_MAX_SIZE = 60;
	constexpr int DECK_MIN_SIZE = 40;
	constexpr int EXTRA_MAX_SIZE = 15;
	constexpr int SIDE_MAX_SIZE = 15;
	constexpr int PACK_MAX_SIZE = 1000;

struct LFList {
	unsigned int hash{};
	std::wstring listName;
	std::unordered_map<int, int> content;
};
struct Deck {
	std::vector<code_pointer> main;
	std::vector<code_pointer> extra;
	std::vector<code_pointer> side;
	Deck() {}
	Deck(const Deck& ndeck) {
		main = ndeck.main;
		extra = ndeck.extra;
		side = ndeck.side;
	}
	void clear() {
		main.clear();
		extra.clear();
		side.clear();
	}
};

struct DeckArray {
	std::vector<uint32_t> main;
	std::vector<uint32_t> extra;
	std::vector<uint32_t> side;
};

class DeckManager {
public:
	Deck current_deck;
	std::vector<LFList> _lfList;

	static char deckBuffer[0x10000];

	void LoadLFListSingle(const char* path);
	void LoadLFList();
	const wchar_t* GetLFListName(int lfhash);
	const std::unordered_map<int, int>* GetLFListContent(int lfhash);
	int CheckDeck(Deck& deck, int lfhash, int rule);
	uint32_t LoadDeck(Deck& deck, uint32_t dbuf[], int mainc, int sidec, bool is_packlist = false);
	bool LoadSide(Deck& deck, uint32_t dbuf[], int mainc, int sidec);
	static FILE* OpenDeckFile(const wchar_t* file, const char* mode);
	bool LoadDeck(const wchar_t* file);
	bool SaveDeck(Deck& deck, const wchar_t* name);
	bool DeleteDeck(Deck& deck, const wchar_t* name);
	static bool RenameDeck(const wchar_t* oldname, const wchar_t* newname);
	wchar_t DeckFormatBuffer[128];
	const wchar_t* GetMainFormatString();
	const wchar_t* GetSideFormatString();
	const wchar_t* GetExtraFormatString();
	int GetTypeCount(std::vector<code_pointer> list, unsigned int ctype);
	static bool SaveDeckArray(const DeckArray& deck, const wchar_t* name);
};

extern DeckManager deckManager;

}

#endif //DECKMANAGER_H
