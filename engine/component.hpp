#pragma once

#include <memory>

// base class for all entity manager component 
class Component
{
	// child components will override this to perform non-slicing copy
	virtual std::unique_ptr<Component>	CloneImp() const = 0;
public:
	// 2 step cloning for copying the dervicd component thru base interface
	std::unique_ptr<Component>			Clone() const;

	virtual								~Component();
};
