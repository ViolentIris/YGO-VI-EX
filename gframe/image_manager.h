#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include "config.h"
#include "data_manager.h"
#include <unordered_map>

namespace ygo {

class ImageManager {
public:
	std::vector<std::wstring> ImageList[12];
	bool Initial();
	//random image
	irr::video::ITexture* GetRandomImage(int image_type);
	irr::video::ITexture* GetHeadImage();
	irr::video::ITexture* GetCoverImage();
	irr::video::ITexture* GetBgImage();
	void RefreshRandomImageList();
	void RefreshImageDir(std::wstring path, int image_type);
	void SetDevice(irr::IrrlichtDevice* dev);
	void ClearTexture();
	void RemoveTexture(int code);
	irr::video::ITexture* GetTextureFromFile(char* file, s32 width, s32 height);
	irr::video::ITexture* GetTextureExpansions(char* file, s32 width, s32 height);
	irr::video::ITexture* GetTextureExpansionsDirectry(const char* path, char* file, s32 width, s32 height);
	irr::video::ITexture* GetTexture(int code, bool fit = false);
	irr::video::ITexture* GetTextureThumb(int code);
	irr::video::ITexture* GetTextureField(int code);

	std::unordered_map<int, irr::video::ITexture*> tMap[3];
	std::unordered_map<int, irr::video::ITexture*> tThumb;
	std::unordered_map<int, irr::video::ITexture*> tFields;
	irr::IrrlichtDevice* device;
	irr::video::IVideoDriver* driver;
	irr::video::ITexture* tCover[8];
	irr::video::ITexture* tAvatar[2];
	irr::video::ITexture* tUnknown;
	irr::video::ITexture* tUnknownFit;
	irr::video::ITexture* tUnknownThumb;
	irr::video::ITexture* tDV;
	irr::video::ITexture* tAct;
	irr::video::ITexture* tAttack;
	irr::video::ITexture* tTotalAtk;
	irr::video::ITexture* tNegated;
	irr::video::ITexture* tChain;
	irr::video::ITexture* tNumber;
	irr::video::ITexture* tLPFrame;
	irr::video::ITexture* tLPFrame2;
	irr::video::ITexture* tLPBar;
	irr::video::ITexture* tLPBar2;
	irr::video::ITexture* tMask;
	irr::video::ITexture* tEquip;
	irr::video::ITexture* tTarget;
	irr::video::ITexture* tChainTarget;
	irr::video::ITexture* tLim;
	irr::video::ITexture* tOT;
	irr::video::ITexture* tHand[3];
	irr::video::ITexture* tBackGround;
	irr::video::ITexture* tBackGround_menu;
	irr::video::ITexture* tBackGround_deck;
	irr::video::ITexture* tField[2];
	irr::video::ITexture* tFieldTransparent[2];
	irr::video::ITexture* tRScale[21];
	irr::video::ITexture* tLScale[21];
	irr::video::ITexture* tCardType[7];
	irr::video::ITexture* tClock;
	/*irr::video::ITexture* tLPBarFrame;
	irr::video::ITexture* tDoll_start[2];
	irr::video::ITexture* tDoll_3000M[2];
	irr::video::ITexture* tDoll_3000L[2];
	irr::video::ITexture* tDoll_LPL[2];
	irr::video::ITexture* tDoll_LPA[2];
	irr::video::ITexture* tDoll_Win[2];
	irr::video::ITexture* tDoll_Lose[2];
	irr::video::ITexture* tDoll_Draw[2];*/
	std::list<std::string> support_types;
};

extern ImageManager imageManager;

}

#endif // IMAGEMANAGER_H
