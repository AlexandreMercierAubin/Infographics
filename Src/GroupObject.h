#pragma once
#include "AbstractObject.h"
#include "Model.h"
#include <memory>

class GroupObject : public AbstractObject
{
public:
	virtual void Draw(glm::mat4 &perspective,glm::mat4 &view);
	void addObject(std::shared_ptr<AbstractObject> object);
private:
	std::vector<std::shared_ptr<AbstractObject>> vObject;
	~GroupObject();
};
