#include <windows.h>

class emulator
{
public:
	emulator() = default;
	~emulator() = default;

	bool start();

private:

	bool setup_pipe();
	bool m_enabled;
	HANDLE m_pipe;
};