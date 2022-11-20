#include "image_manager.h"
#include "game.h"
#ifndef _WIN32
#include <dirent.h>
#endif

namespace ygo {

ImageManager imageManager;

bool ImageManager::Initial()  {
	RefreshRandomImageList();
	tCover[0] = NULL;
	tCover[1] = NULL;
	tCover[2] = NULL;
	tCover[3] = NULL;
	if(mainGame->gameConf.random)
		tCover[0] = GetRandomImage(TEXTURE_COVER_S);
	if(!tCover[0])
		tCover[0] = driver->getTexture("textures/cover.jpg");
	if(mainGame->gameConf.random)
		tCover[1] = GetRandomImage(TEXTURE_COVER_O);
	if(!tCover[1])
		tCover[1] = driver->getTexture("textures/cover2.jpg");
	if(!tCover[1])
		tCover[1] = GetRandomImage(TEXTURE_COVER_S);
	if(!tCover[1])
		tCover[1] = tCover[0];
	if(mainGame->gameConf.random)
		tCover[2] = GetRandomImage(TEXTURE_COVER_S2);
	if(!tCover[2])
		tCover[2] = driver->getTexture("textures/cover3.jpg");
	if(!tCover[2])
		tCover[2] = GetRandomImage(TEXTURE_COVER_S);
	if(!tCover[2])
		tCover[2] = tCover[1];
	if(mainGame->gameConf.random)
		tCover[3] = GetRandomImage(TEXTURE_COVER_O2);
	if(!tCover[3])
		tCover[3] = driver->getTexture("textures/cover4.jpg");
	if(!tCover[3])
		tCover[3] = GetRandomImage(TEXTURE_COVER_O);
	if(!tCover[3])
		tCover[3] = tCover[2];
	tCover[4] = tCover[0];
	tCover[5] = tCover[1];
	tCover[6] = tCover[2];
	tCover[7] = tCover[3];
	tUnknown = driver->getTexture("textures/unknown.jpg");
	tUnknownFit = NULL;
	tUnknownThumb = NULL;
	tAvatar[0] = driver->getTexture("textures/head.jpg");
	tAvatar[1] = GetRandomImage(TEXTURE_AVATAR_S);
	tAct = NULL;
	tAttack = NULL;
	if(mainGame->gameConf.random)
		tAct = GetRandomImage(TEXTURE_ACTIVATE);
	if(mainGame->gameConf.random)
		tAttack = GetRandomImage(TEXTURE_ATTACK);
	if(!tAct)
		tAct = driver->getTexture("textures/act.png");
	if(!tAttack)
		tAttack = driver->getTexture("textures/attack.png");
	tTotalAtk = driver->getTexture("textures/totalAtk.png");
	tChain = driver->getTexture("textures/chain.png");
	tNegated = driver->getTexture("textures/negated.png");
	tNumber = driver->getTexture("textures/number.png");
	tLPBar = driver->getTexture("textures/lpb.png");
	tLPBar2 = driver->getTexture("textures/lp.png");
	tLPFrame = driver->getTexture("textures/lpfb.png");
	tLPFrame2 = driver->getTexture("textures/lpf.png");
	//tLPBarFrame = driver->getTexture("textures/lpbarf.png");
	tMask = driver->getTexture("textures/mask.png");
	tEquip = driver->getTexture("textures/equip.png");
	tTarget = driver->getTexture("textures/target.png");
	tChainTarget = driver->getTexture("textures/chaintarget.png");
	tLim = driver->getTexture("textures/lim.png");
	tOT = driver->getTexture("textures/ot.png");
	tHand[0] = driver->getTexture("textures/f1.jpg");
	tHand[1] = driver->getTexture("textures/f2.jpg");
	tHand[2] = driver->getTexture("textures/f3.jpg");
	tBackGround = NULL;
	tBackGround_menu = NULL;
	tBackGround_deck = NULL;
	if(mainGame->gameConf.random)
		tBackGround = GetRandomImage(TEXTURE_DUEL);
	if(!tBackGround)
		tBackGround = driver->getTexture("textures/bg.jpg");
	if(!tBackGround)
		tBackGround = driver->getTexture("textures/bg_duel.jpg");
	if(mainGame->gameConf.random)
		tBackGround_menu = GetRandomImage(TEXTURE_MENU);
	if(!tBackGround_menu)
		tBackGround_menu = driver->getTexture("textures/bg_menu.jpg");
	if(!tBackGround_menu)
		tBackGround_menu = GetRandomImage(TEXTURE_DUEL);
	tCardType[0] = driver->getTexture("textures/type/monster.jpg");
	tCardType[1] = driver->getTexture("textures/type/spell.jpg");
	tCardType[2] = driver->getTexture("textures/type/trap.jpg");
	tCardType[3] = driver->getTexture("textures/type/fusion.jpg");
	tCardType[4] = driver->getTexture("textures/type/synchro.jpg");
	tCardType[5] = driver->getTexture("textures/type/xyz.jpg");
	tCardType[6] = driver->getTexture("textures/type/link.jpg");
	if(!tBackGround_menu)
		tBackGround_menu = tBackGround;
	if(mainGame->gameConf.random)
		tBackGround_deck = GetRandomImage(TEXTURE_DECK);
	if(!tBackGround_deck)
		tBackGround_deck = driver->getTexture("textures/bg_deck.jpg");
	if(!tBackGround_deck)
		tBackGround_deck = GetRandomImage(TEXTURE_DUEL);
	if(!tBackGround_deck)
		tBackGround_deck = tBackGround;
	tField[0] = driver->getTexture("textures/field2.png");
	tFieldTransparent[0] = driver->getTexture("textures/field-transparent2.png");
	tField[1] = driver->getTexture("textures/field3.png");
	tFieldTransparent[1] = driver->getTexture("textures/field-transparent3.png");
	/*tDoll_start[0] = driver->getTexture("textures/Dolls1/Doll_Start.png");
	tDoll_start[1] = driver->getTexture("textures/Dolls2/Doll_Start.png");
	tDoll_3000M[0] = driver->getTexture("textures/Dolls1/Doll_3000M.png");
	tDoll_3000M[1] = driver->getTexture("textures/Dolls2/Doll_3000M.png");
	tDoll_3000L[0] = driver->getTexture("textures/Dolls1/Doll_3000L.png");
	tDoll_3000L[1] = driver->getTexture("textures/Dolls2/Doll_3000L.png");
	tDoll_LPL[0] = driver->getTexture("textures/Dolls1/Doll_LPL.png");
	tDoll_LPL[1] = driver->getTexture("textures/Dolls2/Doll_LPL.png");
	tDoll_LPA[0] = driver->getTexture("textures/Dolls1/Doll_LPA.png");
	tDoll_LPA[1] = driver->getTexture("textures/Dolls2/Doll_LPA.png");
	tDoll_Win[0] = driver->getTexture("textures/Dolls1/Doll_Win.png");
	tDoll_Win[1] = driver->getTexture("textures/Dolls2/Doll_Win.png");
	tDoll_Lose[0] = driver->getTexture("textures/Dolls1/Doll_Lose.png");
	tDoll_Lose[1] = driver->getTexture("textures/Dolls2/Doll_Lose.png");
	tDoll_Draw[0] = driver->getTexture("textures/Dolls1/Doll_Draw.png");
	tDoll_Draw[1] = driver->getTexture("textures/Dolls2/Doll_Draw.png");*/
	tClock = driver->getTexture("textures/clock.png");
	tDV = GetRandomImage(TEXTURE_DV);
	int i = 0;
	char buff[100];
	for (; i < 21; i++) {
		snprintf(buff, 100, "textures/pscale/rscale_%d.png", i);
		tRScale[i] = driver->getTexture(buff);
	}
	for (i = 0; i < 21; i++) {
		snprintf(buff, 100, "textures/pscale/lscale_%d.png", i);
		tLScale[i] = driver->getTexture(buff);
	}
	support_types.push_back(std::string("jpg"));
	support_types.push_back(std::string("png"));
	support_types.push_back(std::string("bpg"));
	return true;
}
irr::video::ITexture* ImageManager::GetRandomImage(int image_type) {
	int count = ImageList[image_type].size();
	if(count <= 0)
		return 0;
	char ImageName[1024];
	wchar_t fname[1024];
	int image_id = rand() % count;
	auto name = ImageList[image_type][image_id].c_str();
	myswprintf(fname, L"./textures/%ls", name);
	BufferIO::EncodeUTF8(fname, ImageName);
	return driver->getTexture(ImageName);
}
irr::video::ITexture* ImageManager::GetHeadImage() {
	char ImageNameS[1024];
	wchar_t fnameS[1024];
	myswprintf(fnameS, L"./textures/head/%ls", mainGame->cbHDS->getItem(mainGame->cbHDS->getSelected()));
	BufferIO::EncodeUTF8(fnameS, ImageNameS);
	return driver->getTexture(ImageNameS);
}
irr::video::ITexture* ImageManager::GetCoverImage() {
	char ImageNameC[1024];
	wchar_t fnameC[1024];
	myswprintf(fnameC, L"./textures/cover/%ls", mainGame->cbCRS->getItem(mainGame->cbCRS->getSelected()));
	BufferIO::EncodeUTF8(fnameC, ImageNameC);
	return driver->getTexture(ImageNameC);
}
irr::video::ITexture* ImageManager::GetBgImage() {
	char ImageNameB[1024];
	wchar_t fnameB[1024];
	myswprintf(fnameB, L"./textures/bg/%ls", mainGame->cbBGS->getItem(mainGame->cbBGS->getSelected()));
	BufferIO::EncodeUTF8(fnameB, ImageNameB);
	return driver->getTexture(ImageNameB);
}
void ImageManager::RefreshRandomImageList() {
	RefreshImageDir(L"bg", TEXTURE_DUEL);
	RefreshImageDir(L"bg_duel", TEXTURE_DUEL);
	RefreshImageDir(L"bg_deck", TEXTURE_DECK);
	RefreshImageDir(L"bg_menu", TEXTURE_MENU);
	RefreshImageDir(L"cover", TEXTURE_COVER_S);
	RefreshImageDir(L"cover2", TEXTURE_COVER_O);
	RefreshImageDir(L"cover3", TEXTURE_COVER_S2);
	RefreshImageDir(L"cover4", TEXTURE_COVER_O2);
	RefreshImageDir(L"attack", TEXTURE_ATTACK);
	RefreshImageDir(L"act", TEXTURE_ACTIVATE);
	RefreshImageDir(L"head", TEXTURE_AVATAR_S);
	RefreshImageDir(L"fortune", TEXTURE_DV);
}
void ImageManager::RefreshImageDir(std::wstring path, int image_type) {
	std::wstring search = L"./textures/" + path;
	FileSystem::TraversalDir(search.c_str(), [this, &path, image_type](const wchar_t* name, bool isdir) {
		if(!isdir && wcsrchr(name, '.') && (!mywcsncasecmp(wcsrchr(name, '.'), L".jpg", 4) || !mywcsncasecmp(wcsrchr(name, '.'), L".png", 4))) {
			std::wstring filename = path + L"/" + name;
			ImageList[image_type].push_back(filename);
		}
	});
}
void ImageManager::SetDevice(irr::IrrlichtDevice* dev) {
	device = dev;
	driver = dev->getVideoDriver();
}
void ImageManager::ClearTexture() {
	for(auto tit = tMap[0].begin(); tit != tMap[0].end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	for(auto tit = tMap[1].begin(); tit != tMap[1].end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	for(auto tit = tMap[2].begin(); tit != tMap[2].end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	for(auto tit = tThumb.begin(); tit != tThumb.end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	tMap[0].clear();
	tMap[1].clear();
	tMap[2].clear();
	tThumb.clear();
}
void ImageManager::RemoveTexture(int code) {
	auto tit = tMap[0].find(code);
	if(tit != tMap[0].end()) {
		if(tit->second)
			driver->removeTexture(tit->second);
		tMap[0].erase(tit);
	}
	tit = tMap[1].find(code);
	if(tit != tMap[1].end()) {
		if(tit->second)
			driver->removeTexture(tit->second);
		tMap[1].erase(tit);
	}
	tit = tMap[2].find(code);
	if(tit != tMap[2].end()) {
		if(tit->second)
			driver->removeTexture(tit->second);
		tMap[2].erase(tit);
	}
}
// function by Warr1024, from https://github.com/minetest/minetest/issues/2419 , modified
void imageScaleNNAA(irr::video::IImage *src, irr::video::IImage *dest) {
	double sx, sy, minsx, maxsx, minsy, maxsy, area, ra, ga, ba, aa, pw, ph, pa;
	u32 dy, dx;
	irr::video::SColor pxl;

	// Cache rectsngle boundaries.
	double sw = src->getDimension().Width * 1.0;
	double sh = src->getDimension().Height * 1.0;

	// Walk each destination image pixel.
	// Note: loop y around x for better cache locality.
	irr::core::dimension2d<u32> dim = dest->getDimension();
	for(dy = 0; dy < dim.Height; dy++)
		for(dx = 0; dx < dim.Width; dx++) {

			// Calculate floating-point source rectangle bounds.
			minsx = dx * sw / dim.Width;
			maxsx = minsx + sw / dim.Width;
			minsy = dy * sh / dim.Height;
			maxsy = minsy + sh / dim.Height;

			// Total area, and integral of r, g, b values over that area,
			// initialized to zero, to be summed up in next loops.
			area = 0;
			ra = 0;
			ga = 0;
			ba = 0;
			aa = 0;

			// Loop over the integral pixel positions described by those bounds.
			for(sy = floor(minsy); sy < maxsy; sy++)
				for(sx = floor(minsx); sx < maxsx; sx++) {

					// Calculate width, height, then area of dest pixel
					// that's covered by this source pixel.
					pw = 1;
					if(minsx > sx)
						pw += sx - minsx;
					if(maxsx < (sx + 1))
						pw += maxsx - sx - 1;
					ph = 1;
					if(minsy > sy)
						ph += sy - minsy;
					if(maxsy < (sy + 1))
						ph += maxsy - sy - 1;
					pa = pw * ph;

					// Get source pixel and add it to totals, weighted
					// by covered area and alpha.
					pxl = src->getPixel((u32)sx, (u32)sy);
					area += pa;
					ra += pa * pxl.getRed();
					ga += pa * pxl.getGreen();
					ba += pa * pxl.getBlue();
					aa += pa * pxl.getAlpha();
				}

			// Set the destination image pixel to the average color.
			if(area > 0) {
				pxl.setRed(ra / area + 0.5);
				pxl.setGreen(ga / area + 0.5);
				pxl.setBlue(ba / area + 0.5);
				pxl.setAlpha(aa / area + 0.5);
			} else {
				pxl.setRed(0);
				pxl.setGreen(0);
				pxl.setBlue(0);
				pxl.setAlpha(0);
			}
			dest->setPixel(dx, dy, pxl);
		}
}
irr::video::ITexture* ImageManager::GetTextureFromFile(char* file, s32 width, s32 height) {
#ifdef _WIN32
	wchar_t name[1024];
	BufferIO::DecodeUTF8(file, name);
#else
	char* name = file;
#endif // _WIN32

	if(mainGame->gameConf.use_image_scale) {
		irr::video::ITexture* texture;
		irr::video::IImage* srcimg = driver->createImageFromFile(name);
		if(srcimg == NULL)
			return NULL;
		if(srcimg->getDimension() == irr::core::dimension2d<u32>(width, height)) {
			texture = driver->addTexture(name, srcimg);
		} else {
			video::IImage *destimg = driver->createImage(srcimg->getColorFormat(), irr::core::dimension2d<u32>(width, height));
			imageScaleNNAA(srcimg, destimg);
			texture = driver->addTexture(name, destimg);
			destimg->drop();
		}
		srcimg->drop();
		return texture;
	} else {
		return driver->getTexture(name);
	}
}
irr::video::ITexture* ImageManager::GetTextureExpansions(char* file, s32 width, s32 height) {
	irr::video::ITexture* img = GetTextureExpansionsDirectry("./expansions", file, width, height);
	if(img != NULL)
		return img;
	bool find = false;
	FileSystem::TraversalDir("./expansions", [this, file, width, height, &img, &find](const char* name, bool isdir) {
		if(!find && isdir && strcmp(name, ".") && strcmp(name, "..")) {
			char subdir[1024];
			sprintf(subdir, "./expansions/%s", name);
			img = GetTextureExpansionsDirectry(subdir, file, width, height);
			if(img)
				find = true;
		}
	});
	if(find)
		return img;
	return img;
}
irr::video::ITexture* ImageManager::GetTextureExpansionsDirectry(const char* path, char* file, s32 width, s32 height) {
	char fpath[1000];
	sprintf(fpath, "%s/%s", path, file);
	return GetTextureFromFile(fpath, width, height);
}
irr::video::ITexture* ImageManager::GetTexture(int code, bool fit) {
	if(code == 0)
		return fit ? tUnknownFit : tUnknown;
	int width = CARD_IMG_WIDTH;
	int height = CARD_IMG_HEIGHT;
	if(fit) {
		float mul = mainGame->xScale;
		if(mainGame->xScale > mainGame->yScale)
			mul = mainGame->yScale;
		width = width * mul;
		height = height * mul;
	}
	auto tit = tMap[fit ? 1 : 0].find(code);
	if(tit == tMap[fit ? 1 : 0].end()) {
		char file[256];
		sprintf(file, "pics/%d.png", code);
		irr::video::ITexture* img = GetTextureExpansions(file, width, height);
		if(img == NULL) {
			sprintf(file, "pics/%d.jpg", code);
			img = GetTextureExpansions(file, width, height);
		}
		if(img == NULL) {
			sprintf(file, "pics/%d.png", code);
			img = GetTextureFromFile(file, width, height);
		}
		if(img == NULL) {
			sprintf(file, "pics/%d.jpg", code);
			img = GetTextureFromFile(file, width, height);
		}
		if(img == NULL && !mainGame->gameConf.use_image_scale) {
			tMap[fit ? 1 : 0][code] = NULL;
			return GetTextureThumb(code);
		}
		tMap[fit ? 1 : 0][code] = img;
		float mul = (mainGame->xScale > mainGame->yScale) ? mainGame->yScale : mainGame->xScale;
		irr::s32 imgWidthFit = CARD_IMG_WIDTH * mul;
		irr::s32 imgHeightFit = CARD_IMG_HEIGHT * mul;
		tUnknownFit = GetTextureFromFile("textures/unknown.jpg", imgWidthFit, imgHeightFit);
		return (img == NULL) ? (fit ? tUnknownFit : tUnknown) : img;
	}
	if(tit->second)
		return tit->second;
	else
		return mainGame->gameConf.use_image_scale ? (fit ? tUnknownFit : tUnknown) : GetTextureThumb(code);
}
irr::video::ITexture* ImageManager::GetTextureThumb(int code) {
	if(code == 0)
		return tUnknownThumb;
	auto tit = tThumb.find(code);
	int width = CARD_THUMB_WIDTH * mainGame->xScale;
	int height = CARD_THUMB_HEIGHT * mainGame->yScale;
	if(tit == tThumb.end()) {
		static const std::string exts[] = { ".bpg", ".jpg" };
		char file[256];
		sprintf(file, "pics/thumbnail/%d.png", code);
		irr::video::ITexture* img = GetTextureExpansions(file, width, height);
		if(img == NULL) {
			sprintf(file, "pics/thumbnail/%d.jpg", code);
			img = GetTextureExpansions(file, width, height);
		}
		if(img == NULL) {
			sprintf(file, "pics/thumbnail/%d.png", code);
			img = GetTextureFromFile(file, width, height);
		}
		if(img == NULL) {
			sprintf(file, "pics/thumbnail/%d.jpg", code);
			img = GetTextureFromFile(file, width, height);
		}
		if(img == NULL && mainGame->gameConf.use_image_scale) {
			sprintf(file, "pics/%d.png", code);
			img = GetTextureExpansions(file, width, height);
			if(img == NULL) {
				sprintf(file, "pics/%d.jpg", code);
				img = GetTextureExpansions(file, width, height);
			}
			if(img == NULL) {
				sprintf(file, "expansions/pics/%d.jpg", code);
				img = GetTextureFromFile(file, width, height);
			}
			if(img == NULL) {
				sprintf(file, "pics/%d.png", code);
				img = GetTextureFromFile(file, width, height);
			}
			if(img == NULL) {
				sprintf(file, "pics/%d.jpg", code);
				img = GetTextureFromFile(file, width, height);
			}
		}
		tThumb[code] = img;
		tUnknownThumb = GetTextureFromFile("textures/unknown.jpg", width, height);
		return (img == NULL) ? tUnknownThumb : img;
	}
	if(tit->second)
		return tit->second;
	else
		return tUnknownThumb;
}
irr::video::ITexture* ImageManager::GetTextureField(int code) {
	if(code == 0)
		return NULL;
	auto tit = tFields.find(code);
	if(tit == tFields.end()) {
		static const std::string exts[] = { ".bpg", ".png", ".jpg" };
		char file[256];
		sprintf(file, "pics/field/%d.png", code);
		irr::video::ITexture* img = GetTextureExpansions(file, 512 * mainGame->xScale, 512 * mainGame->yScale);
		if(img == NULL) {
			sprintf(file, "pics/field/%d.jpg", code);
			img = GetTextureExpansions(file, 512 * mainGame->xScale, 512 * mainGame->yScale);
		}
		if(img == NULL) {
			sprintf(file, "pics/field/%d.png", code);
			img = GetTextureFromFile(file, 512 * mainGame->xScale, 512 * mainGame->yScale);
		}
		if(img == NULL) {
			sprintf(file, "pics/field/%d.jpg", code);
			img = GetTextureFromFile(file, 512 * mainGame->xScale, 512 * mainGame->yScale);
			if(img == NULL) {
				tFields[code] = NULL;
				return NULL;
			} else {
				tFields[code] = img;
				return img;
			}
		} else {
			tFields[code] = img;
			return img;
		}
	}
	if(tit->second)
		return tit->second;
	else
		return NULL;
}
}
