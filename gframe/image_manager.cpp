#include "image_manager.h"
#include "game.h"

namespace ygo {

ImageManager imageManager;

bool ImageManager::Initial() {
	tUnknown = driver->getTexture("textures/unknown.jpg");
	tUnknownFit = tUnknown;
	tUnknownThumb = tUnknown;
	tAct = driver->getTexture("textures/act.png");
	tAttack = driver->getTexture("textures/attack.png");
	tChain = driver->getTexture("textures/chain.png");
	tNegated = driver->getTexture("textures/negated.png");
	tNumber = driver->getTexture("textures/number.png");
	tLPBar = driver->getTexture("textures/lp.png");
	tLPFrame = driver->getTexture("textures/lpf.png");
	tMask = driver->getTexture("textures/mask.png");
	tEquip = driver->getTexture("textures/equip.png");
	tTarget = driver->getTexture("textures/target.png");
	tChainTarget = driver->getTexture("textures/chaintarget.png");
	tLim = driver->getTexture("textures/lim.png");
	tOT = driver->getTexture("textures/ot.png");
	tHand[0] = driver->getTexture("textures/f1.jpg");
	tHand[1] = driver->getTexture("textures/f2.jpg");
	tHand[2] = driver->getTexture("textures/f3.jpg");
	tBackGround = driver->getTexture("textures/bg.jpg");
	tBackGround_menu = driver->getTexture("textures/bg_menu.jpg");
	if (!tBackGround_menu)
		tBackGround_menu = tBackGround;
	tBackGround_deck = driver->getTexture("textures/bg_deck.jpg");
	if (!tBackGround_deck)
		tBackGround_deck = tBackGround;
	tField[0] = driver->getTexture("textures/field2.png");
	tFieldTransparent[0] = driver->getTexture("textures/field-transparent2.png");
	tField[1] = driver->getTexture("textures/field3.png");
	tFieldTransparent[1] = driver->getTexture("textures/field-transparent3.png");

	for (int i = 0; i < 4; ++i)
	{
		tAvatar[i] = NULL;
		tBorder[i] = NULL;
		tCover[i] = NULL;
		tPartner[i] = NULL;
		tEmblem[i] = NULL;
	}
	return true;
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
	for(auto tit = tThumb.begin(); tit != tThumb.end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	tMap[0].clear();
	tMap[1].clear();
	tThumb.clear();
	tFields.clear();
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
irr::video::ITexture* ImageManager::GetTextureFromFile(const char* file, s32 width, s32 height) {
	if(mainGame->gameConf.use_image_scale) {
		irr::video::ITexture* texture;
		irr::video::IImage* srcimg = driver->createImageFromFile(file);
		if(srcimg == NULL)
			return NULL;
		if(srcimg->getDimension() == irr::core::dimension2d<u32>(width, height)) {
			texture = driver->addTexture(file, srcimg);
		} else {
			video::IImage *destimg = driver->createImage(srcimg->getColorFormat(), irr::core::dimension2d<u32>(width, height));
			imageScaleNNAA(srcimg, destimg);
			texture = driver->addTexture(file, destimg);
			destimg->drop();
		}
		srcimg->drop();
		return texture;
	} else {
		return driver->getTexture(file);
	}
}
irr::video::ITexture* ImageManager::GetTexture(int code, bool fit) {
	if(code == 0)
		return fit ? tUnknownFit : tUnknown;
	auto tit = tMap[fit ? 1 : 0].find(code);
	int width = CARD_IMG_WIDTH;
	int height = CARD_IMG_HEIGHT;
	float xScale = (float)mainGame->window_size.Width / 1024.0;
	float yScale = (float)mainGame->window_size.Height / 640.0;
	if (fit) {
		float mul = xScale;
		if (xScale > yScale)
			mul = yScale;
		width = width * mul;
		height = height * mul;
	}
	if(tit == tMap[fit ? 1 : 0].end()) {
		char file[256];
		sprintf(file, "expansions/pics/%d.jpg", code);
		irr::video::ITexture* img = GetTextureFromFile(file, width, height);
		if(img == NULL) {
			sprintf(file, "pics/%d.jpg", code);
			img = GetTextureFromFile(file, width, height);
		}
		tMap[fit ? 1 : 0][code] = img;
		return (img == NULL) ? (fit ? tUnknownFit : tUnknown) : img;
	}
	if (tit->second)
		return tit->second;
	else
		return (fit ? tUnknownFit : tUnknown);
}
irr::video::ITexture* ImageManager::GetTextureField(int code) {
	if (code == 0)
		return NULL;
	auto tit = tFields.find(code);
	if (tit == tFields.end()) {
		char file[256];
		sprintf(file, "expansions/pics/field/%d.png", code);
		irr::video::ITexture* img = GetTextureFromFile(file, 512, 512);
		if (img == NULL) {
			sprintf(file, "expansions/pics/field/%d.jpg", code);
			img = GetTextureFromFile(file, 512, 512);
		}
		if (img == NULL) {
			sprintf(file, "pics/field/%d.png", code);
			img = GetTextureFromFile(file, 512, 512);
		}
		if (img == NULL) {
			sprintf(file, "pics/field/%d.jpg", code);
			img = GetTextureFromFile(file, 512, 512);
			if (img == NULL) {
				tFields[code] = NULL;
				return NULL;
			}
			else {
				tFields[code] = img;
				return img;
			}
		}
		else {
			tFields[code] = img;
			return img;
		}
	}
	if (tit->second)
		return tit->second;
	else
		return NULL;
}
void ImageManager::LoadBCACustomTextures(bool force)
{
	float xScale = (float)mainGame->window_size.Width / 1024.0;
	float yScale = (float)mainGame->window_size.Height / 640.0;

	irr::s32 imgWidth = CARD_IMG_WIDTH * min(xScale, yScale);
	irr::s32 imgHeight = CARD_IMG_HEIGHT * min(xScale, yScale);
	irr::s32 avatarWidth = AVATAR_SIZE * xScale;
	irr::s32 avatarHeight = AVATAR_SIZE * yScale;


	for (int i = 0; i < 4; ++i)
	{
		if (!tAvatar[i] || force) {
			std::string avatarpath = "textures/avatars/a_" + std::to_string(i) + ".png";
			tAvatar[i] = GetTextureFromFile(avatarpath.c_str(), avatarWidth, avatarHeight);
		}

		if (!tBorder[i] || force) {
			std::string borderpath = "textures/borders/b_" + std::to_string(i) + ".png";
			tBorder[i] = GetTextureFromFile(borderpath.c_str(), 306 * xScale, 136 * yScale);
		}

		if (!tCover[i] || force) {
			std::string coverpath = "textures/sleeves/s_" + std::to_string(i) + ".png";
			tCover[i] = GetTextureFromFile(coverpath.c_str(), imgWidth, imgHeight);
		}

		if (!tPartner[i] || force)
		{
			std::string partnerpath = "textures/partners/p_" + std::to_string(i) + ".png";
			tPartner[i] = GetTextureFromFile(partnerpath.c_str(), 256, 256);
		}
		if (!tEmblem[i] || force)
		{
			std::string emblempath = "textures/emblems/e_" + std::to_string(i) + ".png";
			tEmblem[i] = GetTextureFromFile(emblempath.c_str(), 256, 256);
		}
	}
}
void ImageManager::DeleteBCACustomTextures()
{
	for (int i = 0; i < 4; ++i)
	{
		std::string avatarpath = "textures/avatars/a_" + std::to_string(i) + ".png";
		std::string borderpath = "textures/borders/b_" + std::to_string(i) + ".png";
		std::string coverpath = "textures/sleeves/s_" + std::to_string(i) + ".png";
		std::string emblempath = "textures/emblems/e_" + std::to_string(i) + ".png";
		
		std::remove(avatarpath.c_str());
		std::remove(borderpath.c_str());
		std::remove(coverpath.c_str());
		std::remove(emblempath.c_str());
	}
}
}
