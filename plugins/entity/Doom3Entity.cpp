#include "Doom3Entity.h"

#include "iradiant.h"
#include "ieclass.h"
#include "debugging/debugging.h"
#include "string/predicate.h"
#include <functional>

namespace entity {

Doom3Entity::Doom3Entity(const IEntityClassPtr& eclass) :
	_eclass(eclass),
	_undo(_keyValues, std::bind(&Doom3Entity::importState, this, std::placeholders::_1), "EntityKeyValues"),
	_instanced(false),
	_observerMutex(false),
	_isContainer(!eclass->isFixedSize())
{}

Doom3Entity::Doom3Entity(const Doom3Entity& other) :
	Entity(other),
	_eclass(other.getEntityClass()),
	_undo(_keyValues, std::bind(&Doom3Entity::importState, this, std::placeholders::_1), "EntityKeyValues"),
	_instanced(false),
	_observerMutex(false),
	_isContainer(other._isContainer)
{
	for (KeyValues::const_iterator i = other._keyValues.begin();
		 i != other._keyValues.end();
		 ++i)
	{
		insert(i->first, i->second->get());
	}
}

bool Doom3Entity::isModel() const
{
	std::string name = getKeyValue("name");
	std::string model = getKeyValue("model");
	std::string classname = getKeyValue("classname");

	return (classname == "func_static" && !name.empty() && name != model);
}

bool Doom3Entity::isOfType(const std::string& className)
{
	return _eclass->isOfType(className);
}

void Doom3Entity::importState(const KeyValues& keyValues)
{
	//rMessage() << "Importing state into entity " << getKeyValue("name") << ", got " << keyValues.size() << " key values\n";

	// Remove the entity key values, one by one
	while (_keyValues.size() > 0)
	{
		erase(_keyValues.begin());
	}

	/* greebo: This code somehow doesn't delete all the keys (only every second one)
	for(KeyValues::iterator i = _keyValues.begin(); i != _keyValues.end();) {
		erase(i++);
	}*/

	for (KeyValues::const_iterator i = keyValues.begin(); i != keyValues.end(); ++i)
	{
		insert(i->first, i->second);
	}
}

void Doom3Entity::attachObserver(Observer* observer)
{
	ASSERT_MESSAGE(!_observerMutex, "observer cannot be attached during iteration");

	// Add the observer to the internal list
	_observers.insert(observer);

	// Now notify the observer about all the existing keys
	for(KeyValues::const_iterator i = _keyValues.begin(); i != _keyValues.end(); ++i)
    {
		observer->onKeyInsert(i->first, *i->second);
	}
}

void Doom3Entity::detachObserver(Observer* observer)
{
	ASSERT_MESSAGE(!_observerMutex, "observer cannot be detached during iteration");

	// Remove the observer from the list, if it can be found
	Observers::iterator found = _observers.find(observer);

	if (found == _observers.end())
	{
		// greebo: Observer was not found, no need to call onKeyErase()
		return;
	}

	// Remove the observer
	_observers.erase(found);

	// Call onKeyErase() for every spawnarg, so that the observer gets cleanly shut down
	for(KeyValues::const_iterator i = _keyValues.begin(); i != _keyValues.end(); ++i)
    {
		observer->onKeyErase(i->first, *i->second);
	}
}

void Doom3Entity::connectUndoSystem(IMapFileChangeTracker& changeTracker)
{
	_instanced = true;

	for (const auto& keyValue : _keyValues)
	{
		//rMessage() << "Connecting keyValue: " << keyValue.first << " to Undo System\n";
		keyValue.second->connectUndoSystem(changeTracker);
	}

    _undo.connectUndoSystem(changeTracker);
}

void Doom3Entity::disconnectUndoSystem(IMapFileChangeTracker& changeTracker)
{
	_undo.disconnectUndoSystem(changeTracker);

	for (const auto& keyValue : _keyValues)
	{
		//rMessage() << "Disconnecting keyValue: " << keyValue.first << " from Undo System\n";
		keyValue.second->disconnectUndoSystem(changeTracker);
	}

	_instanced = false;
}

/** Return the EntityClass associated with this entity.
 */
IEntityClassPtr Doom3Entity::getEntityClass() const
{
	return _eclass;
}

void Doom3Entity::forEachKeyValue(const KeyValueVisitFunctor& func) const
{
    for (const KeyValuePair& pair : _keyValues)
	{
		func(pair.first, pair.second->get());
	}
}

void Doom3Entity::forEachEntityKeyValue(const EntityKeyValueVisitFunctor& func)
{
    for (const KeyValuePair& pair : _keyValues)
    {
        func(pair.first, *pair.second);
    }
}

/** Set a keyvalue on the entity.
 */
void Doom3Entity::setKeyValue(const std::string& key, const std::string& value)
{
	if (value.empty())
	{
		// Empty value means: delete the key
		erase(key);
	}
	else
	{
		// Non-empty value, "insert" it (will overwrite existing keys - no duplicates)
		insert(key, value);
	}
}

/** Retrieve a keyvalue from the entity.
 */
std::string Doom3Entity::getKeyValue(const std::string& key) const
{
	// Lookup the key in the map
	KeyValues::const_iterator i = find(key);

	// If key is found, return it, otherwise lookup the default value on
	// the entity class
	if (i != _keyValues.end())
	{
		return i->second->get();
	}
	else
	{
		return _eclass->getAttribute(key).getValue();
	}
}

bool Doom3Entity::isInherited(const std::string& key) const
{
	// Check if we have the key in the local keyvalue map
	bool definedLocally = (find(key) != _keyValues.end());

	// The value is inherited, if it doesn't exist locally and the inherited one is not empty
	return (!definedLocally && !_eclass->getAttribute(key).getValue().empty());
}

Entity::KeyValuePairs Doom3Entity::getKeyValuePairs(const std::string& prefix) const
{
	KeyValuePairs list;

	for (KeyValues::const_iterator i = _keyValues.begin(); i != _keyValues.end(); ++i)
	{
		// If the prefix matches, add to list
		if (string::istarts_with(i->first, prefix))
		{
			list.push_back(
				std::pair<std::string, std::string>(i->first, i->second->get())
			);
		}
	}

	return list;
}

EntityKeyValuePtr Doom3Entity::getEntityKeyValue(const std::string& key)
{
	KeyValues::const_iterator found = find(key);

	return (found != _keyValues.end()) ? found->second : EntityKeyValuePtr();
}

bool Doom3Entity::isWorldspawn() const
{
	return getKeyValue("classname") == "worldspawn";
}

bool Doom3Entity::isContainer() const
{
	return _isContainer;
}

void Doom3Entity::setIsContainer(bool isContainer)
{
	_isContainer = isContainer;
}

void Doom3Entity::notifyInsert(const std::string& key, KeyValue& value)
{
	// Block the addition/removal of new Observers during this process
	_observerMutex = true;

	// Notify all the Observers about the new keyvalue
	for (Observers::iterator i = _observers.begin(); i != _observers.end(); ++i)
	{
		(*i)->onKeyInsert(key, value);
	}

	_observerMutex = false;
}

void Doom3Entity::notifyChange(const std::string& k, const std::string& v)
{
    _observerMutex = true;

    for (Observers::iterator i = _observers.begin();
         i != _observers.end();
         ++i)
    {
		(*i)->onKeyChange(k, v);
	}

    _observerMutex = false;
}

void Doom3Entity::notifyErase(const std::string& key, KeyValue& value)
{
	// Block the addition/removal of new Observers during this process
	_observerMutex = true;

	for(Observers::iterator i = _observers.begin(); i != _observers.end(); ++i)
	{
		(*i)->onKeyErase(key, value);
	}

	_observerMutex = false;
}

void Doom3Entity::insert(const std::string& key, const KeyValuePtr& keyValue)
{
	// Insert the new key at the end of the list
	KeyValues::iterator i = _keyValues.insert(
		_keyValues.end(),
		KeyValuePair(key, keyValue)
	);

	// Dereference the iterator to get a KeyValue& reference and notify the observers
	notifyInsert(key, *i->second);

	if (_instanced)
	{
		//rMessage() << "Connecting new key value to undo system " << key << "\n";
		i->second->connectUndoSystem(_undo.getUndoChangeTracker());
	}
}

void Doom3Entity::insert(const std::string& key, const std::string& value)
{
	// Try to lookup the key in the map
	KeyValues::iterator i = find(key);

	if (i != _keyValues.end())
    {
		//rMessage() << "Storing value over existing key " << key << ": " << value << "\n";

		// Key has been found
		i->second->assign(value);

        // Notify observers of key change, using the found key as argument
		// as the case of the incoming "key" might be different
        notifyChange(i->first, value);
	}
	else
	{
		//rMessage() << "Setting value of new key " << key << ": " << value << "\n";

		// No key with that name found, create a new one
		_undo.save();

		// Allocate a new KeyValue object and insert it into the map
		insert(
			key,
			KeyValuePtr(new KeyValue(value, _eclass->getAttribute(key).getValue()))
		);
	}
}

void Doom3Entity::erase(const KeyValues::iterator& i)
{
	if (_instanced)
	{
		//rMessage() << "Disconnecting key value from undo system " << i->first << "\n";

		i->second->disconnectUndoSystem(_undo.getUndoChangeTracker());
	}

	// Retrieve the key and value from the vector before deletion
	std::string key(i->first);
	KeyValuePtr value(i->second);

	// Actually delete the object from the list
	_keyValues.erase(i);

	// Notify about the deletion
	notifyErase(key, *value);

	// Scope ends here, the KeyValue object will be deleted automatically
	// as the std::shared_ptr useCount will reach zero.
}

void Doom3Entity::erase(const std::string& key)
{
	// Try to lookup the key
	KeyValues::iterator i = find(key);

	if (i != _keyValues.end())
	{
		_undo.save();
		erase(i);
	}
}

Doom3Entity::KeyValues::const_iterator Doom3Entity::find(const std::string& key) const
{
	for (KeyValues::const_iterator i = _keyValues.begin();
		 i != _keyValues.end();
		 ++i)
	{
		if (string::iequals(i->first, key))
		{
			return i;
		}
	}

	// Not found
	return _keyValues.end();
}

Doom3Entity::KeyValues::iterator Doom3Entity::find(const std::string& key)
{
	for (KeyValues::iterator i = _keyValues.begin();
		 i != _keyValues.end();
		 ++i)
	{
		if (string::iequals(i->first, key))
		{
			return i;
		}
	}

	// Not found
	return _keyValues.end();
}

} // namespace entity
