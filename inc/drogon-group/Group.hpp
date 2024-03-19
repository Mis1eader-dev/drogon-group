#pragma once

#include "drogon-user/Room.hpp"
#include "drogon-user/User.hpp"
#include "drogon/HttpRequest.h"
#include "drogon/WebSocketConnection.h"
#include <json/value.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Group
{
private:
	using GroupPtr = std::shared_ptr<Group>;

	const std::string id_;

	UserPtr add(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn) = delete;
	UserPtr remove(const drogon::WebSocketConnectionPtr& conn) = delete;

public:
	static void configure();

	/// Must not be used by the user
	Group(std::string_view id, const UserPtr& user);

	void add(const UserPtr& user);
	UserPtr get(std::string_view id, bool extendLifespan = false) const;
	inline UserPtr get(const drogon::HttpRequestPtr& req, bool extendLifespan = false) const
	{
		return std::move(get(drogon::user::getId(req), extendLifespan));
	}
	inline UserPtr get(const drogon::WebSocketConnectionPtr& conn) const
	{
		return User::get(conn);
	}
	void remove(const UserPtr& user);

	static GroupPtr createGroup(std::string_view id, const UserPtr& user);
	static void createGroupBatch(const std::vector<std::string_view>& ids, const UserPtr& user);
	static GroupPtr getGroup(std::string_view id);

	std::string_view id() const;

	inline void notify(const drogon::WebSocketConnectionPtr& conn, const char* msg,
		uint64_t len, const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		conn->send(msg, len, type);
	}
	inline void notify(const drogon::WebSocketConnectionPtr& conn, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		conn->send(msg, type);
	}
	void notify(const drogon::WebSocketConnectionPtr& conn, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text); // TODO: Once drogon has JSON ws, inline this func
	inline void notify(const drogon::WebSocketConnectionPtr& conn, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notify(conn, *(Json::Value*)json, type);
	}

	inline void notify(const UserPtr& user, Room& room, const char* msg,
		uint64_t len, const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		room.notify(user, msg, len, type);
	}
	inline void notify(const UserPtr& user, Room& room, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notify(user, room, msg.data(), msg.size(), type);
	}
	void notify(const UserPtr& user, Room& room, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text);
	inline void notify(const UserPtr& user, Room& room, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notify(user, room, *(Json::Value*)json, type);
	}

	/// Alias for notify, with user and room parameters swapped
	inline void notify(Room& room, const UserPtr& user, const char* msg,
		uint64_t len, const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notify(user, room, msg, len, type);
	}
	/// Alias for notify, with user and room parameters swapped
	inline void notify(Room& room, const UserPtr& user, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notify(user, room, msg, type);
	}
	/// Alias for notify, with user and room parameters swapped
	inline void notify(Room& room, const UserPtr& user, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notify(user, room, json, type);
	}
	/// Alias for notify, with user and room parameters swapped
	inline void notify(Room& room, const UserPtr& user, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notify(user, room, json, type);
	}



	void notifyAll(Room& room, const char* msg, uint64_t len,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text);
	inline void notifyAll(Room& room, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAll(room, msg.data(), msg.size(), type);
	}
	void notifyAll(Room& room, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text);
	inline void notifyAll(Room& room, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAll(room, *(Json::Value*)json, type);
	}



	void notifyAllExcept(const UserPtr& user, Room& room, const char* msg,
		uint64_t len, const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text);
	inline void notifyAllExcept(const UserPtr& user, Room& room, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(user, room, msg.data(), msg.size(), type);
	}
	void notifyAllExcept(const UserPtr& user, Room& room, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text);
	inline void notifyAllExcept(const UserPtr& user, Room& room, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(user, room, *(Json::Value*)json, type);
	}

	/// Alias for notifyAllExcept, with user and room parameters swapped
	inline void notifyAllExcept(Room& room, const UserPtr& user, const char* msg,
		uint64_t len, const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(user, room, msg, len, type);
	}
	/// Alias for notifyAllExcept, with user and room parameters swapped
	inline void notifyAllExcept(Room& room, const UserPtr& user, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(user, room, msg, type);
	}
	/// Alias for notifyAllExcept, with user and room parameters swapped
	inline void notifyAllExcept(Room& room, const UserPtr& user, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(user, room, json, type);
	}
	/// Alias for notifyAllExcept, with user and room parameters swapped
	inline void notifyAllExcept(Room& room, const UserPtr& user, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(user, room, json, type);
	}

	void notifyAllExcept(const drogon::WebSocketConnectionPtr& conn, Room& room, const char* msg,
		uint64_t len, const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text);
	inline void notifyAllExcept(const drogon::WebSocketConnectionPtr& conn, Room& room, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(conn, room, msg.data(), msg.size(), type);
	}
	void notifyAllExcept(const drogon::WebSocketConnectionPtr& conn, Room& room, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text);
	inline void notifyAllExcept(const drogon::WebSocketConnectionPtr& conn, Room& room, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(conn, room, *(Json::Value*)json, type);
	}

	/// Alias for notifyAllExcept, with conn and room parameters swapped
	inline void notifyAllExcept(Room& room, const drogon::WebSocketConnectionPtr& conn, const char* msg,
		uint64_t len, const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(conn, room, msg, len, type);
	}
	/// Alias for notifyAllExcept, with conn and room parameters swapped
	inline void notifyAllExcept(Room& room, const drogon::WebSocketConnectionPtr& conn, std::string_view msg,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(conn, room, msg, type);
	}
	/// Alias for notifyAllExcept, with conn and room parameters swapped
	inline void notifyAllExcept(Room& room, const drogon::WebSocketConnectionPtr& conn, Json::Value& json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(conn, room, json, type);
	}
	/// Alias for notifyAllExcept, with conn and room parameters swapped
	inline void notifyAllExcept(Room& room, const drogon::WebSocketConnectionPtr& conn, const Json::Value* json,
		const drogon::WebSocketMessageType type = drogon::WebSocketMessageType::Text)
	{
		notifyAllExcept(conn, room, json, type);
	}

protected:
	std::unordered_map<std::string_view, UserPtr> users_;
	mutable std::shared_mutex mutex_;
};

using GroupPtr = std::shared_ptr<Group>;
