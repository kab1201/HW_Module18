#pragma once

#include <exception>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

typedef unsigned int uint;

class UserException : public exception
{
private:
	string error_;

public:
	explicit UserException(const string& error);

	#if defined(_WIN32)
		const char* what() const override;
	#else
		const char* what() const noexcept override;
	#endif
};

class User
{
private:
	string login_;
	uint hashPassword_;
	string salt_;
	string name_;

public:
	User(const string& login, const uint& hashPassword, const string& salt);
	User(const string& login, const uint& hashPassword, const string& salt, const string& name);

	~User() = default;

	const string& getLogin() const;
	const string& getSalt() const;

	const uint& getHashPassword() const;
	void setHashPassword(const uint& newHashPassword);

	const string& getName() const;
	void setName(const string& newName);

	friend ostream& operator << (ostream& out_s, const User& user);
};





