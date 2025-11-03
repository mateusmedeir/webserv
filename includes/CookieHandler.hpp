#pragma once

# include "WebservHeader.hpp"

class CookieHandler
{
	private:

		CookieHandler();
		CookieHandler(const CookieHandler & other);
		CookieHandler & operator=(const CookieHandler & other);

		static std::string generateRandomString(size_t size);

	public:

		static void handleCookie(
			HttpResponse & response, const HttpRequest & request);

};

