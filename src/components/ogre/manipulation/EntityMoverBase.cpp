//
// C++ Class: EntityMoverBase
//
// Description:
//
//
// Author: Erik Hjortsberg <erik.hjortsberg@gmail.com>, (C) 2009
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

#include "EntityMoverBase.h"

#include "../EmberEntity.h"
#include "../MathConverter.h"

#include "SnapToMovement.h"

#include <Eris/Entity.h>

namespace EmberOgre
{
namespace Manipulation
{
EntityMoverBase::EntityMoverBase(Eris::Entity& entity, Ogre::SceneNode* node) :
	mEntity(entity), mNode(node), mSnapping(new Manipulation::SnapToMovement(entity, *node, 2.0f, true))
{

}

EntityMoverBase::~EntityMoverBase()
{
}

const WFMath::Quaternion& EntityMoverBase::getOrientation() const
{
	mOrientation = Ogre2Atlas(mNode->_getDerivedOrientation());
	return mOrientation;
}

const WFMath::Point<3>& EntityMoverBase::getPosition() const
{
	mPosition = Ogre2Atlas(mNode->_getDerivedPosition());
	return mPosition;
}

void EntityMoverBase::setPosition(const WFMath::Point<3>& position)
{
	WFMath::Point<3> finalPosition(position);
	if (position.isValid())
	{
		WFMath::Vector<3> adjustment;
		EmberEntity* entity(0);
		if (mSnapping.get() && mSnapping->testSnapTo(position, getOrientation(), adjustment, entity))
		{
			finalPosition = finalPosition.shift(adjustment);
		}

		///We need to offset into local space.
		Ogre::Vector3 posOffset = Ogre::Vector3::ZERO;
		if (mNode->getParent())
		{
			posOffset = mNode->getParent()->_getDerivedPosition();
		}
		mNode->setPosition(Atlas2Ogre(finalPosition) - posOffset);
		newEntityPosition(mNode->getPosition());
	}
}
void EntityMoverBase::move(const WFMath::Vector<3>& directionVector)
{
	if (directionVector.isValid())
	{
		mNode->translate(Atlas2Ogre(directionVector));
		newEntityPosition(mNode->getPosition());
	}
}
void EntityMoverBase::setRotation(int axis, WFMath::CoordType angle)
{
	///not implemented yet
}

void EntityMoverBase::yaw(WFMath::CoordType angle)
{
	mNode->yaw(Ogre::Degree(angle));
}

void EntityMoverBase::setOrientation(const WFMath::Quaternion& rotation)
{
	if (rotation.isValid())
	{
		///We need to offset into local space.
		Ogre::Quaternion rotOffset = Ogre::Quaternion::IDENTITY;
		if (mNode->getParent())
		{
			rotOffset = mNode->getParent()->_getDerivedOrientation();
		}
		mNode->setOrientation(Atlas2Ogre(rotation) - rotOffset);
	}
}

void EntityMoverBase::newEntityPosition(const Ogre::Vector3& position)
{
}

}
}