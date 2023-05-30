#include "Message.h"

MessageException::MessageException(const string& error) : error_(error) {};

#if defined(_WIN32)
	const char* MessageException::what() const
	{
		return error_.c_str();
	}
#else
	const char* MessageException::what() const noexcept
	{
		return error_.c_str();
	}
#endif

Message::Message(const string& from, const string& to, const string& text) : from_(from), to_(to), text_(text) {}

const string& Message::getFrom() const { return from_; }
const string& Message::getTo() const { return to_; }

ostream& operator<<(ostream& out_s, const Message& message)
{
	size_t ln = message.from_.length() + message.to_.length() + 15;
	out_s << "From: " << message.from_ << " To: " << message.to_ << endl << message.text_ << endl << string(ln, '-');

	return out_s;
}
