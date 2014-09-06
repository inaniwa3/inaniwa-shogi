#pragma once

unsigned long timeGetTime();

unsigned long GetPrivateProfileString
(
	const char* lpAppName,
	const char* lpKeyName,
	const char* lpDefault,
	char* lpReturnedString,
	unsigned long nSize,
	const char* lpFileName
);

bool WritePrivateProfileString
(
	const char* lpAppName,
	const char* lpKeyName,
	const char* lpString,
	const char* lpFileName
);
