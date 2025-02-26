#include "AmbientColor.hpp"


void AmbientColor::onChange(CCObject* sender) {
	setIconColor(getScreenColor());
}

ccColor3B AmbientColor::getRenderColor(CCSprite* bgSprite, Settings::ColorPicker picker) {
	auto renderTexture = CCRenderTexture::create(1, 1);

	renderTexture->begin(); // Rendering block

	if (Settings::getPicker() == Settings::ColorPicker::BG && bgSprite)
		bgSprite->visit();
	else
		m_layer->visit();

	renderTexture->end(); // Rendering block

	auto img = renderTexture->newCCImage();
	auto data = img->getData();
	ccColor3B color = ccColor3B{data[0], data[1], data[2]};
	
	delete img;
	renderTexture->removeMeAndCleanup();

	return color;
}

CCSprite* AmbientColor::getPickSprite() {
	auto parent = m_layer->getChildByIDRecursive("main-node");
	CCSprite* bgSprite = nullptr;

	if (parent->getChildByID("background"))
		bgSprite = static_cast<CCSprite *>(parent->getChildByID("background"));
	else {
		log::debug("No background found");
		bgSprite = parent->getChildByType<CCSprite>(0);
	}
	return bgSprite;
}

ccColor3B AmbientColor::getScreenColor() {
	m_pickPos = CCPoint(getRenderXPos(), getRenderYPos());

	auto size = CCDirector::sharedDirector()->getWinSize();

	auto oldPos = m_layer->getPosition();

	// For getting the exact position on the screen, we will move the layer itself, because of the RenderTexture capabilities
	// For m_layer (which is the layer), we need negative numbers to move the layer off screen and get the exact position
	m_layer->setPositionX(-size.width * m_pickPos.x);
	m_layer->setPositionY(-size.height * m_pickPos.y);

	if (Mod::get()->getSettingValue<bool>("draw-pos")) {
		if (auto spr = m_layer->m_objectLayer->getChildByIDRecursive("test"_spr))
			spr->removeMeAndCleanup();

		auto circleSpr = CCSprite::create("circle.png");
		circleSpr->setPosition({static_cast<float>(size.width * m_pickPos.x), static_cast<float>(size.height * m_pickPos.y)});
		circleSpr->setID("test"_spr);

		m_layer->addChild(circleSpr);
	}


	auto pickSprite = getPickSprite();
	ccColor3B color = getRenderColor(pickSprite, Settings::getPicker());
	
	if (Mod::get()->getSettingValue<bool>("debug-color"))
		log::info("RGB: {} {} {}", color.r, color.g, color.b);

	if (color == ccColor3B{0, 0, 0} && m_changeMethodWhenBlack) {
		getRenderColor(pickSprite, Settings::ColorPicker::SCREEN);
	}
	m_layer->setPosition(oldPos);

	return color;
}

void AmbientColor::setIconColor(ccColor3B color) {
	switch (Settings::getPlayerPreference()) {
		case Settings::BOTH:
			m_layer->m_player1->setColor(color);
			m_layer->m_player1->setSecondColor(color);
			break;
		case Settings::MAIN:
			m_layer->m_player1->setColor(color);
			break;
		case Settings::SECONDARY:
			m_layer->m_player1->setSecondColor(color);
			break;
	}

	switch (Settings::getPlayerPreference(true)) {
		case Settings::BOTH:
			m_layer->m_player2->setColor(color);
			m_layer->m_player2->setSecondColor(color);
			break;
		case Settings::MAIN:
			m_layer->m_player2->setColor(color);
			break;
		case Settings::SECONDARY:
			m_layer->m_player2->setSecondColor(color);
			break;
	}

	switch (Settings::getExtra()) {
		case Settings::GLOW:
			m_layer->m_player1->m_glowColor = color;
			m_layer->m_player1->updateGlowColor();

			m_layer->m_player2->m_glowColor = color;
			m_layer->m_player2->updateGlowColor();
			break;
		case Settings::WAVE_TRAIL:
			if (m_layer->m_player1->m_isDart)
				m_layer->m_player1->m_waveTrail->setColor(color);
			if (m_layer->m_player2->m_isDart)
				m_layer->m_player2->m_waveTrail->setColor(color);
			break;
	}
}