#pragma once
#include "GroupObject.h"

void GroupObject::Draw(glm::mat4 &perspective, glm::mat4 &view)
{
	//tranfers the draw call
	for (auto it = vObject.begin(); it != vObject.end(); ++it)
	{
		(*it)->Draw(perspective, view);
	}
}

void GroupObject::addObject(std::shared_ptr<AbstractObject> object) 
{
	vObject.push_back(object);
}

void GroupObject::deleteObjectAt(const int index)
{
	vObject.erase(vObject.begin() + index);
}

unsigned int GroupObject::size()
{
	return vObject.size();
}

std::shared_ptr<AbstractObject> GroupObject::getObjectAt(const int index)
{
	return vObject.at(index);
}


GroupObject::~GroupObject()
{
	vObject.clear();
}
