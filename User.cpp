#include "User.h"

UserException::UserException(const string& error) : error_(error) {};

#if defined(_WIN32)
	const char* UserException::what() const
	{
		return error_.c_str();
	}
#else
	const char* UserException::what() const noexcept
	{
		return error_.c_str();
	}
#endif

User::User(const string& login, const uint& hashPassword, const string& salt) : 
	login_(login), 
	hashPassword_(hashPassword), 
	salt_(salt), name_(login) {}
User::User(const string& login, const uint& hashPassword, const string& salt, const string& nickName) : 
	login_(login), 
	hashPassword_(hashPassword), 
	salt_(salt), 
	name_(nickName) {}

const string& User::getLogin() const { return login_; }
const string& User::getSalt() const { return salt_; }

const uint& User::getHashPassword() const { return hashPassword_; }
void User::setHashPassword(const uint& newHashPassword) { hashPassword_ = newHashPassword; }

const string& User::getName() const { return name_; }
void User::setName(const string& newName) { name_ = newName; }

ostream& operator << (ostream& out_s, const User& user)
{
	out_s << user.login_;

	return out_s;
}

