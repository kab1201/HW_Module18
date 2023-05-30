#pragma once

#include <exception>
#include <string>
#include <iostream>
using namespace std;

class MessageException : public exception
{
private:
	string error_;

public:
	explicit MessageException(const string& error);

	#if defined(_WIN32)
		const char* what() const override;
	#else
		const char* what() const noexcept override;
	#endif
};

class Message
{
private:
	string from_;
	string to_;
	string text_;

public:
	Message(const string& from, const string& to, const string& text);
	~Message() = default;

	const string& getFrom() const;
	const string& getTo() const;

	friend ostream& operator<<(ostream& out, const Message& message);
};
