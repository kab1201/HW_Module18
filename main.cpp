#include "Chat.h"

int main()
{
	Chat chat{};

	do
	{
		chat.chatMenu();

		while (chat.getPtrCurrentUser())
		{
			// Depending on the user's choice, the User menu or the Settings menu is displayed.
			if (!chat.getIsToolsMenu())
				chat.userMenu();
			else
				chat.toolsMenu();
		}

	} while (chat.getIsWork());

	return 0;
}