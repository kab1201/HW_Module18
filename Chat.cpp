#include "Chat.h"

#if defined(_WIN32)
// Colors for the console				analogs for Linux
const size_t colorWhite = 7;		// \033[37m ... \033[0m
const size_t colorLightBlue = 11;	// \033[96m ... \033[0m
const size_t colorLightGreen = 10;	// \033[92m ... \033[0m
const size_t colorYellow = 14;		// \033[93m ... \033[0m
const size_t colorDarkBlue = 19;	// \033[94m ... \033[0m

// Declare and initialize variables to change the color of text in the console
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
size_t concolColor = colorWhite;

//Log-file's names
string nameUserLogFile = "UserLog_Win.txt";
string nameMessageLogFile = "MessageLog_Win.txt";
#else
//Log-file's names
string nameUserLogFile = "UserLog_Linux";
string nameMessageLogFile = "MessageLog_Linux";
#endif

// Reserve place for users in unordered_map
const size_t countChatUsers = 200;
// Reserve place for masseges in vector
const size_t countMessages = 1000;

Chat::Chat()
{
	isWork_ = true;
	users_.reserve(countChatUsers);
	messages_.reserve(countMessages);

	try {
		readUserLogFile();
		readMessegeLogFile();
	}
	catch (out_of_range& e)
	{
		isWork_ = false;
		cerr << e.what();
	}
	catch (UserException& e)
	{
		isWork_ = false;
		ptr_currentUser_ = nullptr;
		cerr << e.what();
	}
	catch (MessageException& e)
	{
		isWork_ = false;
		cerr << e.what();
	}
}

Chat::~Chat()
{
	writeUserLogFile();
}

bool Chat::getIsWork() const { return isWork_; }
bool Chat::getIsToolsMenu() const { return isToolsMenu_; }

shared_ptr<User> Chat::getPtrCurrentUser() const { return ptr_currentUser_; }
void Chat::setPtrCurrentUser(shared_ptr<User> ptr_currentUser) { ptr_currentUser_ = ptr_currentUser; }

shared_ptr<User> Chat::getPtrUserByLogin(const string& login) const
{
	auto result = users_.find(login);
	if (result != end(users_)) {
		return result->second;
	}
	return nullptr;
}

shared_ptr<User> Chat::getPtrUserByName(const string& name) const
{
	for (const auto& u : users_)
	{
		if (u.second->getName() == name)
			return u.second;
	}
	return nullptr;
}

bool Chat::isAlphaNumeric(const string &str)
{
	auto it = find_if_not(str.begin(), str.end(), ::isalnum);
	return it == str.end();
}

void Chat::checkNewLogin(const string& login)
{
	if (!isAlphaNumeric(login))
		throw UserException("The login must contain letters and numbers only!\n");

	auto ptrUser = getPtrUserByLogin(login);
	if (ptrUser)
		throw UserException("This login already exists!\n");

	if (login == "all")
		throw UserException("This login is reserved!\n");
}

void Chat::checkNewPassword(const string& psw)
{
	if (psw.length() < minPswLen)
		throw UserException("\nPassword must be equal to or more than 5 characters long!\n");

	if (psw.length() > maxPswLen)
		throw UserException("\nPassword must not exceed 20 characters long!\n");

	const char* pattern = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"`~!@#$%^&*()_-+=|/[]{}:;',.<>\?\" "; 

	if (psw.find_first_not_of(pattern) != string::npos)
		throw UserException("\nPassword contains invalid characters!\n");
}

void Chat::checkLogin(const string& login, const string& psw)
{

	auto ptrUser = getPtrUserByLogin(login);
	if (!ptrUser)
		throw UserException("Invalid login! Try again, please.\n");

	if (ptrUser->getHashPassword() != createHashPassword(psw + ptrUser->getSalt()))
		throw UserException("Invalid password! Try again, please.\n");

	setPtrCurrentUser(ptrUser);
}

#if defined(_WIN32)
string Chat::passwordField()
{
	const size_t size_password = 25;
	char password[size_password] = { 0 };
	char ch;
	size_t k = 0;
	while (k < size_password - 1)
	{
		ch = _getch();
		password[k] = ch;
		if (password[k] == 13) {
			password[k] = '\0';
			break;
		}
		else if (password[k] == 8) {
			cout << "\b \b";
			--k;
		}
		else {
			++k;
			cout << '*';
		}
	}

	return string(password);
}
#endif

uint Chat::createHashPassword(const string& psw)
{
	// Convert string to char*[]
	size_t lenPsw = psw.length();
	char* userPsw = new char[lenPsw];
	memcpy(userPsw, psw.c_str(), lenPsw);

	// Hash password (including salt)
	uint* userHashPsw = sha1(userPsw, lenPsw);

	// Clear
	delete[] userPsw;

	return *userHashPsw;
}

string Chat::createSalt()
{
	// Using the computer’s internal clock to generate the seed.
	const auto p = std::chrono::system_clock::now() - std::chrono::hours(24);
	auto seed = p.time_since_epoch().count();
	// Quickly generating pseudo random integers between 1 and 2147483646
	default_random_engine randomEngine(seed);
	// Converting those random integers into the range [33, 126] such that they’re uniformly distributed.
	uniform_int_distribution<int> distribution(0x21, 0x7E);
	auto randomReadableCharacter = [&]() { return distribution(randomEngine); };

	size_t size = 10;
	string salt;
	generate_n(back_inserter(salt), size, randomReadableCharacter);

	return salt;
}

void Chat::signUp()
{
	string userLogin;
	string userPassword;
	string userName;

	do
	{
		try
		{
			// User sign in attempt 
			cout << "Login: ";
			cin >> userLogin;
			checkNewLogin(userLogin);

			#if defined(_WIN32)
				cout << endl << "Password: ";
				userPassword = passwordField();
			#else
				userPassword = getpass("Password: ");
			#endif
			checkNewPassword(userPassword);

			cout << endl << "Name (optional): ";
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			getline(cin, userName);
			cout << endl;

			if (userName.length() == 0)
				userName = userLogin;

			string salt = createSalt();
			auto ptrNewUser = make_shared<User>(User{ userLogin, createHashPassword(userPassword + salt), salt, userName });
			users_.insert(make_pair( userLogin, ptrNewUser ));
			setPtrCurrentUser(ptrNewUser);
		}
		catch (UserException& e)
		{
			ptr_currentUser_ = nullptr;
			cerr << e.what();
		}
	} while (ptr_currentUser_ == nullptr);

	cout << *ptr_currentUser_ << ", you have successfully registered!\n" << endl;
}

void Chat::signIn()
{
	string userLogin;
	string userPassword;
	size_t cntAttemptsLogin = 0;

	do
	{
		cout << "Login: ";
		cin >> userLogin;
		#if defined(_WIN32)
			cout << endl << "Password: ";
			userPassword = passwordField();
		#else
			userPassword = getpass("Password: ");
		#endif
		cout << endl;

		try
		{
			// User sign in attempt 
			++cntAttemptsLogin;
			checkLogin(userLogin, userPassword);
		}
		catch (UserException& e)
		{
			cerr << e.what();
		}
	} while (ptr_currentUser_ == nullptr && (cntAttemptsLogin < cntAttempts));

	if (ptr_currentUser_ == nullptr)
		cout << "You have made " << cntAttempts << " attempts! Sign Up, please.\n";
}

void Chat::Exit()
{
	cout << "Goodby, " << *ptr_currentUser_ << "!" << endl;
	if (ptr_currentUser_)
		ptr_currentUser_ = nullptr;
}

void Chat::showAllUsers()
{
	// change the color of the text
	#if defined(_WIN32)
		concolColor = colorDarkBlue;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
		cout << "*****   USERS   *****" << endl;
		// reset the color of the text
		concolColor = colorWhite;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
	#else
		cout << "\033[94m*****   USERS   *****\033[0m" << endl;
	#endif 
	for (auto& u : users_)
	{
		cout << u.second->getName() << ((u.second == ptr_currentUser_) ? "\t(*)" : "") << endl;
	}
}

void Chat::showAllMessages()
{
	// change the color of the text
	#if defined(_WIN32)
		concolColor = colorDarkBlue;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
		cout << "*****   MESSAGES   *****" << endl;
		// change the color of the text
		concolColor = colorWhite;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
	#else
		cout << "\033[94m*****   MESSAGES   *****\033[0m" << endl;
	#endif 
	string userLogin;
	for (auto& msg : messages_)
	{
		userLogin = ptr_currentUser_->getLogin();
		if (msg->getTo() == userLogin || (msg->getTo() == static_cast<string>("all") && msg->getFrom() != userLogin))
			cout << "->  " << *msg << endl;
		if (msg->getFrom() == userLogin)
			cout << "<-  " << *msg << endl;
	}
}

void Chat::sendMessage()
{
	string to{};
	string text{};

	cout << "To (name or all): ";
	cin >> to;
	cout << endl << "Text: ";
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	getline(cin, text);
	cout << endl;

	try
	{
		auto ptrUser = getPtrUserByName(to);
		if (!ptrUser && to != "all")
			throw MessageException("Invalid user name (to)! Choose another user, please.\n");

		messages_.push_back(make_shared<Message>(Message{ ptr_currentUser_->getLogin(), ptrUser->getLogin(), text }));
		writeMessageLogFile(ptr_currentUser_->getLogin(), ptrUser->getLogin(), text);
	}
	catch (MessageException& e)
	{
		cerr << e.what();
	}
}

void Chat::changePassword()
{
	string newPassword;

	#if defined(_WIN32)
		cout << endl << "Enter new password: ";
		newPassword = passwordField();
	#else
		newPassword = getpass("Password: ");
	#endif
	cout << endl;

	try
	{
		checkNewPassword(newPassword);
	}
	catch (UserException& e)
	{
		cerr << e.what();
	}

	ptr_currentUser_->setHashPassword(createHashPassword(newPassword + ptr_currentUser_->getSalt()));
}

void Chat::changeName()
{
	string newName;

	cout << "Enter new nick name: ";
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	getline(cin, newName);
	cout << endl;

	ptr_currentUser_->setName(newName);
}

void Chat::chatMenu()
{
	int choiceSign = 1;

	// change the color of the text
	#if defined(_WIN32)
		concolColor = colorYellow;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
		cout << string(47, '-') << endl;
		cout << "| Sign In (1) || Sign Up (2) || Stop chat (0) |" << endl;
		cout << string(47, '-') << endl;
		// reset the color of the text
		concolColor = colorWhite;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
	#else
		cout << "\033[93m" << string(47, '-') << endl;
		cout << "| Sign In (1) || Sign Up (2) || Stop chat (0) |" << endl;
		cout << string(47, '-') << "\033[0m" << endl;
	#endif
	cin >> choiceSign;

	switch (choiceSign)
	{
	case 1:
		signIn();
		break;
	case 2:
		signUp();
		break;
	case 0:
		isWork_ = false;
		break;
	default:
		cout << "Wrong choice!" << endl << endl;
		break;
	}
}

void Chat::userMenu()
{
	int choice = 1;

	#if defined(_WIN32)
		// change the color of the text
		concolColor = colorLightBlue;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
		cout << string(74, '-') << endl;
		cout << "| Send Message (1) || Messages (2) || Users (3) || Tools (4) || Exit (0) |" << endl;
		cout << string(74, '-') << endl;
		// reset the color of the text
		concolColor = colorWhite;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
	#else
		cout << "\033[96m" << string(74, '-') << endl;
		cout << "| Send Message (1) || Messages (2) || Users (3) || Tools (4) || Exit (0) |" << endl;
		cout << string(74, '-') << "\033[0m" << endl;
	#endif
	cin >> choice;

	switch (choice)
	{
	case 1:
		sendMessage();
		break;
	case 2:
		showAllMessages();
		break;
	case 3:
		showAllUsers();
		break;
	case 4:
		isToolsMenu_ = true;
		break;
	case 0:
		Exit();
		break;
	default:
		cout << "Wrong choice!" << endl << endl;
		break;
	}
}

void Chat::toolsMenu()
{
	int choice = 1;

	#if defined(_WIN32)
		// change the color of the text
		concolColor = colorLightGreen;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
		cout << string(56, '-') << endl;
		cout << "| Change Name (1) || Change password (2) || Return (0) |" << endl;
		cout << string(56, '-') << endl;
		// change the color of the text
		concolColor = colorWhite;
		SetConsoleTextAttribute(hConsole, concolColor);
		// ----------
	#else
		cout << "\033[92m" << string(56, '-') << endl;
		cout << "| Change Name (1) || Change password (2) || Return (0) |" << endl;
		cout << string(56, '-') << "\033[0m" << endl;
	#endif
	cin >> choice;

	switch (choice)
	{
	case 1:
		changeName();
		break;
	case 2:
		changePassword();
		break;
	case 0:
		isToolsMenu_ = false;
		break;
	default:
		cout << "Wrong choice!" << endl << endl;
		break;
	}
}

void Chat::readUserLogFile()
{
	/*
	Default user:
	login: all, password: allPswd
	Users for testing:
	login: test1, password: t1Pswd
	Login: test2, password: t2Pswd
	*/

	// Object to read from file
	fstream fin(nameUserLogFile, ios::in | ios::out);

	// Check that the file is open
	if (!fin.is_open())
		throw UserException("\nFailed to open log file to get user credentials!\n");
	else {
		string line;

		size_t posLogin{};
		size_t posHashPsw{};
		size_t posSalt{};
		size_t posName{};
		size_t pos{};

		string login;
		uint hashPsw{};
		string salt;
		string name;

		fin.seekg(0, ios_base::beg);

		while (!fin.eof()) {
			line = "";
			getline(fin, line);
			posLogin = line.find("login: ", 0);
			if (posLogin == string::npos) continue;

			posHashPsw = line.find("hashPsw: ", posLogin);
			posSalt = line.find("salt: ", posHashPsw);
			posName = line.find("name: ", posSalt);

			login = line.substr(7, posHashPsw - 8);
			// Convert string to uint
			hashPsw = (uint)stoll(line.substr(posHashPsw + 9, posSalt - posHashPsw - 10), &pos);
			salt = line.substr(posSalt + 6, posName - posSalt - 7);
			name = line.substr(posName + 6, line.length() - posName - 6);

			auto ptrNewUser = make_shared<User>(User{ login, hashPsw, salt, name });
			users_.insert(make_pair(ptrNewUser->getLogin(), ptrNewUser));
		}
	}

	fin.close();
}

void Chat::writeUserLogFile()
{
	ofstream fout(nameUserLogFile, ios::out | ios::trunc);

	if (!fout.is_open())
		throw UserException("\nFailed to open log-file to save user credentials!\n");
	else {
		for (auto& u : users_)
		{
			fout << "login: " << u.first << " "
				<< "hashPsw: " << u.second->getHashPassword() << " "
				<< "salt: " << u.second->getSalt() << " "
				<< "name: " << u.second->getName() << endl;
		}
	}

	fout.close();
}

void Chat::readMessegeLogFile()
{
	// Object to read from file
	fstream fin(nameMessageLogFile, ios::in | ios::out);

	// Check that the file is open
	if (!fin.is_open())
		throw MessageException("\nFailed to open log file to get user messages!\n");
	else {
		string line;

		size_t posFrom{};
		size_t posTo{};
		size_t posText{};
		
		string loginFrom;
		string loginTo;
		string text;

		fin.seekg(0, ios_base::beg);

		while (!fin.eof()) {
			line = "";
			getline(fin, line);
			posFrom = line.find("from: ", 0);
			if (posFrom == string::npos) continue;

			posTo = line.find("to: ", posFrom);
			posText = line.find("text: ", posTo);

			loginFrom = line.substr(6, posTo - 7);
			loginTo = line.substr(posTo + 4, posText - posTo - 5);
			text = line.substr(posText + 6, line.length() - posText - 6);

			messages_.push_back(make_shared<Message>(Message{ loginFrom, loginTo, text }));
		}
	}

	fin.close();
}

void Chat::writeMessageLogFile(const string& from, const string& to, string& text)
{
	ofstream fout(nameMessageLogFile, ios::in | ios::out | ios::app);

	if (!fout.is_open())
		throw UserException("\nFailed to open log-file to save message!\n");
	else {
		fout << "from: " << from << " "
			<< "to: " << to << " "
			<< "text: " << text << endl;
	}

	fout.close();
}