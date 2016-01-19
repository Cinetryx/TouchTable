#include "Configurator.h"
#include "IReceiver.h"
#include <sstream>
#include <memory>
#include "json\json.h"


Configurator::Configurator(TouchTracking& tracking, EventSerializer& serializer)
	: tracking(tracking), serializer(serializer)
{
}


Configurator::~Configurator()
{
}

void Configurator::receive(const std::string& data, ISender& reply)
{
	std::unique_ptr<Json::CharReader> reader(Json::CharReaderBuilder::CharReaderBuilder().newCharReader());
	Json::Value root;
	std::string err;
	if (!reader->parse(data.c_str(), data.c_str() + data.length(), &root, &err))
	{
		return replyFailed(0, reply);
	}

	int messageId = - 1;

	auto members = root.getMemberNames();
	
	bool getConfig = false;

	for (auto& member : members)
	{
		if (member == "message_id")
		{
			messageId = root[member].asInt();
			continue;
		}
		else if (member == "get_configuration")
		{
			getConfig = true;
			continue;
		}
		auto& item = root[member];
		
		update(member, item);
	}
	if (getConfig)
	{
		replyConfig(messageId, reply);
	}
	else
	{
		replyOk(messageId, reply);
	}
}

void Configurator::update(const std::string& name, const Json::Value& value)
{
	if (name == "remove_background" && value.asBool())
	{
		tracking.removeBackground();
	}
	else if (name == "data_format")
	{
		auto mode = value.asString();
		if (mode == "json")
		{
			serializer.mode(EventSerializer::Mode::Json);
		}
		else if (mode == "binary")
		{
			serializer.mode(EventSerializer::Mode::Binary);
		}
	}
	else if (name == "horizontal_flipped")
	{
		tracking.flipHorizontal(value.asBool());
	}
	else if (name == "vertical_flipped")
	{
		tracking.flipVertical(value.asBool());
	}

}

void Configurator::replyOk(int messageId, ISender& reply)
{
	std::stringstream s;
	s << "{\r\n\t\"message_id\": " << messageId << ",\r\n";
	s << "\t\"status\": \"ok\"\r\n";
	s << "}\r\n";
	reply.send(s.str());
}

void Configurator::replyFailed(int messageId, ISender& reply)
{
	std::stringstream s;
	s << "{\r\n\t\"message_id\": " << messageId << ",\r\n";
	s << "\t\"status\": \"failed\"\r\n";
	s << "}\r\n";
	reply.send(s.str());
}

void Configurator::replyConfig(int messageId, ISender& reply)
{
	std::stringstream s;
	s << "{\r\n\t\"message_id\": " << messageId << ",\r\n";
	s << "\t\"status\": \"ok\",\r\n";
	s << "\t\"horizontal_flipped\": " << (tracking.horizontalFlipped() ? "true" : "false") << ",\r\n";
	s << "\t\"vertical_flipped\": " << (tracking.verticalFlipped() ? "true" : "false") << ",\r\n";
	s << "\t\"data_format\": " << ((serializer.mode() == EventSerializer::Mode::Binary) ? "binary" : "json") << "\r\n";
	s << "}\r\n";
	reply.send(s.str());
}
