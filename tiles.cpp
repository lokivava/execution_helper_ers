//---------------------------------------------------------------------------

#pragma hdrstop

#include "tiles.h"
#include <windows.h>
#include <type_traits>
#include <memory>
#include <utility>

using namespace std;

//---------------------------------------------------------------------------
#pragma package(smart_init)

class ProcessContext : public Context
{
	SECURITY_ATTRIBUTES saAttr = {0};
	STARTUPINFO SI = {0};
	PROCESS_INFORMATION PI = {0};
	HANDLE readHandle = nullptr;   // stdout
	HANDLE writeHandle = nullptr; // stdin
	public:
	virtual ~ProcessContext()
	{
		::CloseHandle(PI.hThread );
		PI.hThread = nullptr;
		::CloseHandle(PI.hProcess );
        PI.hProcess = nullptr;
		if(readHandle != nullptr)
			::CloseHandle(readHandle);
		readHandle = nullptr;
		if(writeHandle != nullptr)
			::CloseHandle(writeHandle);
		writeHandle = nullptr;
	}
	virtual void runCommand(String str) override
	{
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		if(!::CreatePipe(&readHandle, &writeHandle, &saAttr, 0))
		{
			readHandle = writeHandle = nullptr;
		}
		// Ensure the read handle to the pipe for STDOUT is not inherited.
		else if(!SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0))
		{
			if(readHandle != nullptr)
				::CloseHandle(readHandle);
			if(writeHandle != nullptr)
				::CloseHandle(writeHandle);
			readHandle = writeHandle = nullptr;
		}

		SI.cb = sizeof(STARTUPINFO);

		SI.hStdError = writeHandle;
		SI.hStdOutput = writeHandle;
		SI.dwFlags |= STARTF_USESTDHANDLES;

		::CreateProcess(0, str.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW | CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &SI, &PI);
	}
	virtual void pipeToStdout() override
	{
		if(readHandle == nullptr)
			return;
        const auto BUFSIZE = 2048;
		DWORD dwRead, dwWritten;
		char chBuf[BUFSIZE];
		bool bSuccess = false;
		HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		do
		{
			if(writeHandle != nullptr && isEnded())
			{
				::CloseHandle(writeHandle);
                writeHandle = nullptr;
            }
			bSuccess = ReadFile( readHandle, chBuf, BUFSIZE, &dwRead, NULL);
			if(!bSuccess || dwRead == 0)
			{
				if(::GetLastError() == ERROR_BROKEN_PIPE)
				{
					::CloseHandle(readHandle);
					readHandle = nullptr;
					std::cout << std::endl << " --- Process is closed --- " << std::endl;
				}
				break;
			}

			bSuccess = WriteFile(hParentStdOut, chBuf, dwRead, &dwWritten, NULL);
		} while(!bSuccess);
	}
	virtual bool isEnded() const override
	{
		DWORD exit_code;
		::GetExitCodeProcess(PI.hProcess, &exit_code);
		return exit_code != STILL_ACTIVE;
	}
	virtual Optional<String> waitTillEnd() override
	{
		::WaitForSingleObject(PI.hProcess, INFINITE );
		return getLastError();
	}
	virtual Optional<String> getLastError() override
	{
		DWORD exit_code;
		if (FALSE == GetExitCodeProcess(PI.hProcess, &exit_code))
			return Optional(String(GetLastError()));
		return Optional<String>{NullOption{}};
	}
};

//---------------------------------------------------------------------------

String Tile::serialize(void) const
{
    return "";
}
void Tile::deserialize(const String str)
{

}
Context * Tile::run(String path)
{
	auto ctx = new ProcessContext;
	ctx->runCommand(command + " " + path);
	return ctx;
}
