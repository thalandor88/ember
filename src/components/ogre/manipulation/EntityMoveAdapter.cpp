//
// C++ Implementation: EntityMoveAdapter
//
// Description: 
//
//
// Author: Erik Hjortsberg <erik@katastrof.nu>, (C) 2006
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.//
//
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "EntityMoveAdapter.h"
#include "../EmberOgre.h"
#include "IEntityMoveBridge.h"
#include "EntityMoveManager.h"
#include "../AvatarCamera.h"
#include "../AvatarTerrainCursor.h"
#include "../GUIManager.h"
#include "../MathConverter.h"
//#include "../input/Input.h"

using namespace WFMath;
using namespace Ember;

namespace EmberOgre {


EntityMoveAdapterWorkerBase::EntityMoveAdapterWorkerBase(EntityMoveAdapter& adapter)
: mAdapter(adapter)
{
}

EntityMoveAdapterWorkerBase::~EntityMoveAdapterWorkerBase()
{
}

IEntityMoveBridge* EntityMoveAdapterWorkerBase::getBridge()
{
	return mAdapter.mBridge;
}


EntityMoveAdapterWorkerDiscrete::EntityMoveAdapterWorkerDiscrete(EntityMoveAdapter& adapter)
: EntityMoveAdapterWorkerBase(adapter), mMovementSpeed(10)
{
}

bool EntityMoveAdapterWorkerDiscrete::injectMouseMove(const Ember::MouseMotion& motion, bool& freezeMouse)
{
	///this will move the entity instead of the mouse
	
	Vector<3> direction;
	direction.zero();
	direction.x() = -motion.xRelativeMovement;
	direction.y() = motion.yRelativeMovement;
	direction = direction * mMovementSpeed;
	///hard coded to allow the shift button to increase the speed
// 	if (Input::getSingleton().isKeyDown(SDLK_RSHIFT) || Input::getSingleton().isKeyDown(SDLK_LSHIFT)) {
// 		direction = direction * 5;
// 	}
	
	///move it relative to the camera
	direction = direction.rotate(Ogre2Atlas(EmberOgre::getSingleton().getMainCamera()->getOrientation()));
	
	getBridge()->move( direction);//move the entity a fixed distance for each mouse movement.
	
	///we don't want to move the cursor
	freezeMouse = true;
	
	return false;

}

EntityMoveAdapterWorkerTerrainCursor::EntityMoveAdapterWorkerTerrainCursor(EntityMoveAdapter& adapter)
: EntityMoveAdapterWorkerBase(adapter)
{
	/// Register this as a frame listener
	Ogre::Root::getSingleton().addFrameListener(this);
}

EntityMoveAdapterWorkerTerrainCursor::~EntityMoveAdapterWorkerTerrainCursor()
{
	Ogre::Root::getSingleton().removeFrameListener(this);
}

bool EntityMoveAdapterWorkerTerrainCursor::frameStarted(const Ogre::FrameEvent& event)
{
	const Ogre::Vector3* position(0);
	if (EmberOgre::getSingleton().getMainCamera()->getTerrainCursor().getTerrainCursorPosition(&position)) {
		getBridge()->setPosition(Ogre2Atlas(*position));
	}
	return true;
}


EntityMoveAdapter::EntityMoveAdapter(EntityMoveManager* manager)
: mBridge(0), mManager(manager), mWorker(0)
{}

EntityMoveAdapter::~EntityMoveAdapter()
{}

void EntityMoveAdapter::finalizeMovement()
{
	mBridge->finalizeMovement();
	removeAdapter();
	mManager->EventFinishedMoving.emit();
}

void EntityMoveAdapter::cancelMovement()
{
	mBridge->cancelMovement();
	removeAdapter();
	mManager->EventCancelledMoving.emit();
}

bool EntityMoveAdapter::injectMouseMove(const Ember::MouseMotion& motion, bool& freezeMouse)
{
	if (mWorker) {
		return mWorker->injectMouseMove(motion, freezeMouse);
	}
	return true;
}

bool EntityMoveAdapter::injectMouseButtonUp(const Ember::Input::MouseButton& button)
{
	if (button == Input::MouseButtonLeft)
	{
		finalizeMovement();
	}
	else if(button == Input::MouseButtonRight)
	{
	}
	else
	{
		return false;
	}
	
	return false;
}

bool EntityMoveAdapter::injectMouseButtonDown(const Ember::Input::MouseButton& button)
{
	if (button == Input::MouseButtonLeft)
	{
	}
	else if(button == Input::MouseButtonRight)
	{

	}
	else if(button == Input::MouseButtonMiddle)
	{

	}
	else if(button == Input::MouseWheelUp)
	{
		int movementDegrees = 10;
		if (Input::getSingleton().isKeyDown(SDLK_LSHIFT) ||Input::getSingleton().isKeyDown(SDLK_RSHIFT)) {
			movementDegrees = 1;
		}
		mBridge->yaw(movementDegrees);
	}
	else if(button == Input::MouseWheelDown)
	{
		int movementDegrees = 10;
		if (Input::getSingleton().isKeyDown(SDLK_LSHIFT) ||Input::getSingleton().isKeyDown(SDLK_RSHIFT)) {
			movementDegrees = 1;
		}
		mBridge->yaw(-movementDegrees);
	}

	return false;
}

bool EntityMoveAdapter::injectChar(char character)
{
	return true;
}

bool EntityMoveAdapter::injectKeyDown(const SDLKey& key)
{
	if (mWorker) {
		///by pressing and holding shift we'll allow the user to position it with more precision. We do this by switching the worker instances.
		if (key == SDLK_LSHIFT || key == SDLK_RSHIFT) {
			delete mWorker;
			mWorker = new EntityMoveAdapterWorkerDiscrete(*this);
		}
	}
	return true;
}

bool EntityMoveAdapter::injectKeyUp(const SDLKey& key)
{
	if (key == SDLK_ESCAPE) {
		cancelMovement();
		return false;
	} else if (key == SDLK_LSHIFT || key == SDLK_RSHIFT) {
		if (mWorker) {
			delete mWorker;
			mWorker = new EntityMoveAdapterWorkerTerrainCursor(*this);
		}
	}

	return true;
}

void EntityMoveAdapter::attachToBridge(IEntityMoveBridge* bridge)
{
	mBridge = bridge;
	addAdapter();
}

void EntityMoveAdapter::detach()
{
	delete mBridge;
	mBridge = 0;
	removeAdapter();
}

void EntityMoveAdapter::removeAdapter()
{
	Input::getSingleton().removeAdapter(this);
	delete mWorker;
	mWorker = 0;
}

void EntityMoveAdapter::addAdapter()
{
	Input::getSingleton().addAdapter(this);
	///default to the terrain cursor positioning mode
	mWorker = new EntityMoveAdapterWorkerTerrainCursor(*this);
}

}