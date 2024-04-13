#include "drogon-group/Group.hpp"
#include "drogon-user/User.hpp"
#include <cstddef>
#include <json/value.h>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace drogon;
using std::string;
using std::scoped_lock;
using std::shared_lock;
using std::string_view;

static std::shared_mutex mutex_;
static std::unordered_map<string_view, GroupPtr> groups_;

void Group::configure()
{
	user::registerOfflineUserCallback([](const UserPtr& user)
	{
		shared_lock slock(user->groupsMutex_);
		for(const auto& [_, group] : user->groups_)
			group->remove(user);
	});
}



/* Group Class */

Group::Group(string_view id, const UserPtr& user) :
	id_(id),
	users_({
		{
			user->id(), user
		}
	})
{}

GroupPtr Group::createGroup(string_view id, const UserPtr& user)
{
	GroupPtr group = std::make_shared<Group>(string(id), user);
	auto pair = std::make_pair(group->id(), group);
	{
		scoped_lock lock(::mutex_);
		groups_.insert(pair);
	}
	{
		scoped_lock lock(user->groupsMutex_);
		user->groups_.insert(
			std::move(pair)
		);
	}
	return std::move(group);
}
void Group::createGroupBatch(const std::vector<string_view>& ids, const UserPtr& user)
{
	scoped_lock lock(user->groupsMutex_);
	for(string_view id : ids)
	{
		GroupPtr group = getGroup(id);
		if(group)
		{
			group->add(user);

			auto pair = std::make_pair(group->id(), group);
			user->groups_.insert( // already locked groupsMutex_
				std::move(pair)
			);
			continue;
		}

		group = std::make_shared<Group>(id, user);
		auto pair = std::make_pair(group->id(), group);
		{
			scoped_lock lock2(::mutex_);
			groups_.insert(pair);
		}

		user->groups_.insert( // already locked groupsMutex_
			std::move(pair)
		);
	}
}

GroupPtr Group::getGroup(string_view id)
{
	shared_lock slock(::mutex_);
	auto find = groups_.find(id);
	return find != groups_.end() ? find->second : nullptr;
}

std::vector<GroupPtr> Group::getGroups(const std::vector<string_view>& ids)
{
	std::vector<GroupPtr> groups;
	groups.reserve(ids.size());

	shared_lock slock(::mutex_);
	auto end = groups_.end();
	for(string_view id : ids)
	{
		auto find = groups_.find(id);
		if(find == end)
			continue;

		groups.emplace_back(find->second);
	}
	return groups;
}

std::vector<GroupPtr> Group::getGroups(const std::vector<string>& ids)
{
	std::vector<GroupPtr> groups;
	groups.reserve(ids.size());

	shared_lock slock(::mutex_);
	auto end = groups_.end();
	for(string_view id : ids)
	{
		auto find = groups_.find(id);
		if(find == end)
			continue;

		groups.emplace_back(find->second);
	}
	return groups;
}

void Group::add(const UserPtr& user)
{
	scoped_lock lock(mutex_);
	users_[user->id()] = user;
}

UserPtr Group::get(std::string_view id, bool extendLifespan) const
{
	UserPtr user = nullptr;
	{
		shared_lock slock(mutex_);
		auto find = users_.find(id);
		if(find == users_.end())
			return std::move(user);

		user = find->second;
	}

	if(extendLifespan)
		User::prolongPurge(user->id_);

	return std::move(user);
}

void Group::remove(const UserPtr& user)
{
	{
		scoped_lock lock(mutex_);
		if(users_.size() != 1)
		{
			users_.erase(user->id());
			return;
		}
		users_.clear();
	}

	scoped_lock lock(::mutex_);
	groups_.erase(id_);
}

string_view Group::id() const
{
	return id_;
}



void Group::notify(const WebSocketConnectionPtr& conn, Json::Value& json,
	const WebSocketMessageType type)
{
	Json::FastWriter writer;
	writer.omitEndingLineFeed();
	auto msg = writer.write(json);
	notify(conn, msg.data(), msg.size(), type);
}



void Group::notifyAll(Room& room, const char* msg, uint64_t len,
	const WebSocketMessageType type)
{
	shared_lock slock(mutex_);
	for(const auto& [_, user] : users_)
		notify(user, room, msg, len, type);
}
void Group::notifyAll(Room& room, Json::Value& json,
	const WebSocketMessageType type)
{
	Json::FastWriter writer;
	writer.omitEndingLineFeed();
	auto msg = writer.write(json);
	notifyAll(room, msg.data(), msg.size(), type);
}



void Group::notifyAllExcept(const UserPtr& user, Room& room, const char* msg,
	uint64_t len, const WebSocketMessageType type)
{
	shared_lock slock(mutex_);
	auto it = users_.cbegin();
	const auto end = users_.cend();
	for(; it != end; ++it)
	{
		const UserPtr& cur = it->second;
		if(cur == user)
		{
			++it;
			break;
		}
		notify(cur, room, msg, len, type);
	}
	for(; it != end; ++it)
		notify(it->second, room, msg, len, type);
}
void Group::notifyAllExcept(const UserPtr& user, Room& room, Json::Value& json,
	const WebSocketMessageType type)
{
	Json::FastWriter writer;
	writer.omitEndingLineFeed();
	auto msg = writer.write(json);
	notifyAllExcept(user, room, msg.data(), msg.size(), type);
}

void Group::notifyAllExcept(const WebSocketConnectionPtr& conn, Room& room, const char* msg,
	uint64_t len, const WebSocketMessageType type)
{
	const UserPtr user = room.get(conn);
	notifyAllExcept(user, room, msg, len, type);

	const auto& connsMap = user->conns_;
	shared_lock slock(user->mutex_);
	const auto find = connsMap.find(&room);
	if(find == connsMap.end())
		return;

	const auto& connsSet = find->second;
	auto it = connsSet.cbegin();
	const auto end = connsSet.cend();
	for(; it != end; ++it)
	{
		const WebSocketConnectionPtr& cur = *it;
		if(cur == conn)
		{
			++it;
			break;
		}
		notify(cur, msg, len, type);
	}
	for(; it != end; ++it)
		notify(*it, msg, len, type);
}
void Group::notifyAllExcept(const WebSocketConnectionPtr& conn, Room& room, Json::Value& json,
	const WebSocketMessageType type)
{
	Json::FastWriter writer;
	writer.omitEndingLineFeed();
	auto msg = writer.write(json);
	notifyAllExcept(conn, room, msg.data(), msg.size(), type);
}



/* User Class */

GroupPtr User::firstGroup(size_t sizePredicate) const
{
	shared_lock slock(groupsMutex_);
	return (
		sizePredicate == 0 && !groups_.empty() ||
		sizePredicate > 0 && groups_.size() == sizePredicate
	) ? std::move(groups_.begin()->second) : nullptr;
}

std::vector<GroupPtr> User::groups() const
{
	std::vector<GroupPtr> groups;

	shared_lock slock(groupsMutex_);
	groups.reserve(groups_.size());
	for(const auto& [_, group] : groups_)
		groups.emplace_back(group);

	return groups;
}
