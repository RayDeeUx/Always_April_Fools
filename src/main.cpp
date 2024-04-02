
#include <Geode/Geode.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/InfoLayer.hpp>

#include <random>
#include <algorithm>
#include <ctime>

#include "name_variations.hpp"

using namespace geode::prelude;

bool sameColor = false;
ccColor3B sharedColor = ccc3(255, 255, 255);

bool randomColors = false;
bool randomNames = false;

std::string randomName(int accountID, std::string username) {
	std::random_device rd;
	std::mt19937 rng(randomNames ? rd() : accountID);

	std::string shuffledArr[sizeof(names) / sizeof(names[0])];
	std::copy(std::begin(names), std::end(names), shuffledArr);

	std::shuffle(std::begin(shuffledArr), std::end(shuffledArr), rng);

	std::string str = shuffledArr[0];

	size_t pos = str.find("{n}");

	if (pos != std::string::npos)
		str.replace(pos, 3, username);

	return str;
}

ccColor3B randomPastelColor(int accountID) {
	std::random_device rd;
	std::mt19937 rng(randomColors ? rd() : accountID);

	std::uniform_int_distribution<int> dis(100, 220);

	int r = dis(rng);
	int g = dis(rng);
	int b = dis(rng);

	int highest = r > g ? r : g;
	highest = highest > b ? highest : b;

	return ccc3(r == highest ? 255 : r, g == highest ? 255 : g, b == highest ? 255 : b);
}

class $modify(CommentCell) {
	void loadFromComment(GJComment * comment) {
		CommentCell::loadFromComment(comment);
		if (comment->m_isSpam || !Mod::get()->getSettingValue<bool>("enabled")) return;
		
		CCLabelBMFont* usernameLabel = nullptr;
		CCArray* commentLabels = nullptr;

		CCLayer* mainLayer = getChildOfType<CCLayer>(this, 1);
		CCMenu* mainMenu = dynamic_cast<CCMenu*>(mainLayer->getChildByID("main-menu"));
		if (!mainMenu) return;
		CCMenuItemSpriteExtra* menuItem = nullptr;

		bool isProfileComment = this->m_accountComment;
		bool usernameClickable = mainLayer->getChildByID("username-label") == nullptr;

		TextArea* textArea = dynamic_cast<TextArea*>(mainLayer->getChildByID("comment-text-area"));
		bool isTextArea = textArea != nullptr;

		CCSprite* modBadge = nullptr;
		CCObject* obj;
		bool skip = false;
		CCARRAY_FOREACH(mainLayer->getChildren(), obj) {
			CCSprite* spr = dynamic_cast<CCSprite*>(obj);
			if (spr && !dynamic_cast<SimplePlayer*>(obj)) {
				if (!skip && isTextArea && comment->m_modBadge < 1) {
					skip = true;
					continue;
				} else {
					modBadge = spr;
					break;
				}
			}
		}

		CCLabelBMFont* percentageLabel = dynamic_cast<CCLabelBMFont*>(mainLayer->getChildByID("percentage-label"));

		if (isTextArea)
			commentLabels = getChildOfType<CCNode>(textArea, 0)->getChildren();
		else {
			commentLabels = CCArray::create();
			commentLabels->addObject(mainLayer->getChildByID("comment-text-label"));
		}

		if (isProfileComment)
			usernameLabel = static_cast<CCLabelBMFont*>(mainLayer->getChildByID("username-label"));
		else {
			menuItem = usernameClickable ? dynamic_cast<CCMenuItemSpriteExtra*>(mainMenu->getChildByID("username-button"))
				: CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("modBadge_01_001.png"), mainLayer, nullptr);
			if (!menuItem) return;
			usernameLabel = usernameClickable ? static_cast<CCLabelBMFont*>(menuItem->getChildren()->objectAtIndex(0)) : static_cast<CCLabelBMFont*>(mainLayer->getChildByID("username-label"));

			if (usernameClickable) {
				auto likeButton = static_cast<CCMenuItemSpriteExtra*>(mainMenu->getChildByID("like-button"));
				likeButton->setPosition(likeButton->getPosition() + mainMenu->getPosition());

				auto deleteButton = dynamic_cast<CCMenuItemSpriteExtra*>(mainMenu->getChildByID("delete-button"));
				if (deleteButton) deleteButton->setPosition(deleteButton->getPosition() + mainMenu->getPosition());

				menuItem->setPosition(menuItem->getPosition() + mainMenu->getPosition() - CCPoint(menuItem->getContentSize().width / 2.f + 4, 0));
				menuItem->setAnchorPoint({ 0, 0.5f });
				mainMenu->setPosition({ 0, 0 });
			} else {
				menuItem->setPosition(usernameLabel->getPosition());
				menuItem->setContentSize(usernameLabel->getContentSize());
				menuItem->setVisible(false);
			}

		}
		if (!usernameLabel) return;

		std::string username = this->m_comment->m_userName;
		int accountID = this->m_comment->m_accountID;

		if (!usernameLabel) return;
		usernameLabel->setString(randomName(accountID, username).c_str());
		usernameLabel->setAnchorPoint({ 0.f, 0.5f });
		usernameLabel->setPositionX(isProfileComment ? 11.f : (usernameClickable ? 0 : usernameLabel->getPositionX()));

		bool badgeFound = true;
		if (!isProfileComment) {
			menuItem->setContentWidth((usernameLabel->getContentSize().width / 2) / (isTextArea ? usernameLabel->getScale() : 1.f));
			if (!modBadge) {
				badgeFound = false;
				std::random_device rd;
				std::mt19937 gen(rd());
				std::bernoulli_distribution d(0.9);

				modBadge = CCSprite::createWithSpriteFrameName(d(gen) ? "modBadge_01_001.png" : "modBadge_02_001.png");
				modBadge->setScale(isTextArea ? 0.75f : 0.55f);
				modBadge->setPosition(menuItem->getPosition() + CCPoint(menuItem->getContentSize().width + 6, 0));
	
				mainLayer->addChild(modBadge);
			}
			else
				modBadge->setPositionX(menuItem->getPositionX() + menuItem->getContentSize().width + 6);

			if (percentageLabel)
				percentageLabel->setPositionX(modBadge->getPositionX() + 11);
		} else
			badgeFound = false;

		obj = nullptr;
		CCARRAY_FOREACH(commentLabels, obj) {
			CCLabelBMFont* lbl = dynamic_cast<CCLabelBMFont*>(obj);
			if (!lbl) continue;
			lbl->setColor(sameColor ? sharedColor : randomPastelColor(accountID));
			if (badgeFound && isTextArea && comment->m_modBadge < 1) lbl->setPositionX(-(menuItem->getContentSize().width / (isTextArea ? textArea->getScale() : 1.f)) - 25.f);
		}
	}

};

class $modify(InfoLayer) {
	void loadPage(int p0, bool p1) {
		if (!Mod::get()->getSettingValue<bool>("enabled")) {
			InfoLayer::loadPage(p0, p1);
			return;
		}

		randomColors = Mod::get()->getSettingValue<bool>("random_colors");
		randomNames = Mod::get()->getSettingValue<bool>("random_names");

		std::random_device rd;
		std::mt19937 gen(rd());
		std::bernoulli_distribution d(0.5);

		sameColor = Mod::get()->getSettingValue<bool>("always_repeat") ? true
			: Mod::get()->getSettingValue<bool>("never_repeat") ? false : d(gen);

		if (sameColor) sharedColor = randomPastelColor(rd());

		InfoLayer::loadPage(p0, p1);
	}
};

class $modify(ProfilePage) {
	bool init(int accountID, bool ownProfile) {
		if (!Mod::get()->getSettingValue<bool>("enabled")) return ProfilePage::init(accountID, ownProfile);

		randomColors = Mod::get()->getSettingValue<bool>("random_colors");
		randomNames = Mod::get()->getSettingValue<bool>("random_names");

		sameColor = false;
		return ProfilePage::init(accountID, ownProfile);
	}
};