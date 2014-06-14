/*
 * @file fsfloateraddtocontactset.cpp
 * @brief Add an avatar to a contact set
 *
 * (C) 2013 Cinder Roxley @ Second Life <cinder.roxley@phoenixviewer.com>
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "llviewerprecompiledheaders.h"
#include "fsfloateraddtocontactset.h"

#include "fsfloatercontacts.h"
#include "fspanelcontactsets.h"
#include "llnotificationsutil.h"
#include "llslurl.h"
#include <boost/foreach.hpp>

FSFloaterAddToContactSet::FSFloaterAddToContactSet(const LLSD& target)
:	LLFloater(target)
,	mContactSetsCombo(NULL)
{
	if (target.isArray())
	{
		mHasMultipleAgents = true;
		for (S32 i = 0; i < target.size(); ++i)
		{
			mAgentIDs.push_back(target[i].asUUID());
		}
	}
	else
	{
		mHasMultipleAgents = false;
		mAgentID = target.asUUID();
	}

	mContactSetChangedConnection = LGGContactSets::getInstance()->setContactSetChangeCallback(boost::bind(&FSFloaterAddToContactSet::updateSets, this, _1));
}

BOOL FSFloaterAddToContactSet::postBuild()
{
	if (mHasMultipleAgents)
	{
		LLStringUtil::format_map_t args;
		args["COUNT"] = llformat("%d", mAgentIDs.size());
		childSetValue("textfield", LLSD( getString("text_add_multiple", args) ) );
	}
	else
	{
		LLStringUtil::format_map_t args;
		args["NAME"] = LLSLURL("agent", mAgentID, "inspect").getSLURLString();
		childSetValue("textfield", LLSD( getString("text_add_single", args)) );
	}
	
	mContactSetsCombo = getChild<LLComboBox>("contact_sets");
	populateContactSets();
	
	childSetAction("add_btn",	boost::bind(&FSFloaterAddToContactSet::onClickAdd, this));
	childSetAction("cancel_btn", boost::bind(&FSFloaterAddToContactSet::onClickCancel, this));
	childSetAction("add_set_btn", boost::bind(&FSFloaterAddToContactSet::onClickAddSet, this));
	
	return TRUE;
}

void FSFloaterAddToContactSet::onClickAdd()
{
	const std::string set = mContactSetsCombo->getSimple();

	if (!mHasMultipleAgents)
	{
		mAgentIDs.push_back(mAgentID);
	}

	for (uuid_vec_t::iterator it = mAgentIDs.begin(); it != mAgentIDs.end(); ++it)
	{
		if (!LLAvatarTracker::instance().isBuddy(*it))
		{
			LGGContactSets::getInstance()->addNonFriendToList(*it);
		}
		LGGContactSets::getInstance()->addFriendToSet(*it, set);
	}

	if (mHasMultipleAgents)
	{
		LLSD args;
		args["COUNT"] = llformat("%d", mAgentIDs.size());
		args["SET"] = set;
		LLNotificationsUtil::add("AddToContactSetMultipleSuccess", args);
	}
	else
	{
		LLSD args;
		args["NAME"] = LLSLURL("agent", mAgentID, "inspect").getSLURLString();
		args["SET"] = set;
		LLNotificationsUtil::add("AddToContactSetSingleSuccess", args);
	}
	closeFloater();
}

void FSFloaterAddToContactSet::onClickCancel()
{
	closeFloater();
}

void FSFloaterAddToContactSet::onClickAddSet()
{
	LLNotificationsUtil::add("AddNewContactSet", LLSD(), LLSD(), &LGGContactSets::handleAddContactSetCallback);
}

void FSFloaterAddToContactSet::updateSets(LGGContactSets::EContactSetUpdate type)
{
	if (type)
		populateContactSets();
}

void FSFloaterAddToContactSet::populateContactSets()
{
	if (!mContactSetsCombo) return;
	
	mContactSetsCombo->clearRows();
	std::vector<std::string> contact_sets = LGGContactSets::getInstance()->getAllContactSets();
	if (contact_sets.empty())
	{
		mContactSetsCombo->add(getString("no_sets"), LLSD("No Set"));
	}
	else
	{
		BOOST_FOREACH(const std::string& set_name, contact_sets)
		{
			mContactSetsCombo->add(set_name);
		}
	}
	getChild<LLButton>("add_btn")->setEnabled(!contact_sets.empty());
}
