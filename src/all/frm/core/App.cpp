#include "App.h"

#include <frm/core/Input.h>
#include <frm/core/Profiler.h>

#if FRM_MODULE_AUDIO
	#include <frm/audio/Audio.h>
#endif

#include <apt/platform.h>
#ifdef APT_PLATFORM_WIN
	#include <apt/win.h> // SetCurrentDirectory
	#include <apt/log.h>
#endif
#include <apt/ArgList.h>

#include <cstring>

using namespace frm;
using namespace apt;


// PUBLIC

bool App::init(const apt::ArgList& _args)
{
	#if FRM_MODULE_AUDIO
		Audio::Init();
	#endif

	return true;
}

void App::shutdown()
{
	#if FRM_MODULE_AUDIO
		Audio::Shutdown();
	#endif
}

bool App::update()
{
	Profiler::NextFrame();

	PROFILER_MARKER_CPU("#App::update");

	Input::PollAllDevices();
	Timestamp thisUpdate = Time::GetTimestamp();
	m_deltaTime = (thisUpdate - m_prevUpdate).asSeconds() * m_timeScale;
	m_prevUpdate = thisUpdate;

	
	#if FRM_MODULE_AUDIO
		Audio::Update();
	#endif
	
	return true;
}

// PROTECTED

App::App()
	: m_timeScale(1.0)
	, m_deltaTime(0.0)
{
	m_prevUpdate = Time::GetTimestamp();

#ifdef APT_PLATFORM_WIN
 // force the current working directoy to the exe location
	TCHAR buf[MAX_PATH] = {};
	DWORD buflen;
	APT_PLATFORM_VERIFY(buflen = GetModuleFileName(0, buf, MAX_PATH));
	char* pathend = strrchr(buf, (int)'\\');
	*(++pathend) = '\0';
	APT_PLATFORM_VERIFY(SetCurrentDirectory(buf));
	APT_LOG("Set current directory: '%s'", buf);
#endif
}

App::~App()
{
}
