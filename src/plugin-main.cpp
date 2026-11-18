#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QMainWindow>

#include "guard/replay_buffer_guard.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-replay-buffer-confirm", "en-US")

static ReplayBufferGuard *g_guard = nullptr;

static void frontendEvent(enum obs_frontend_event event, void *data)
{
	auto *guard = static_cast<ReplayBufferGuard *>(data);
	if (!guard)
		return;

	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		// 系统托盘与部分控件在插件加载之后才建好，加载完成时再挂一次。
		guard->attachLate();
	} else if (event == OBS_FRONTEND_EVENT_EXIT) {
		guard->prepareShutdown();
	}
}

bool obs_module_load(void)
{
	auto *mainWindow = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	if (!mainWindow) {
		blog(LOG_ERROR, "[replay-buffer-confirm] OBS main window unavailable");
		return false;
	}

	g_guard = new ReplayBufferGuard(mainWindow);
	obs_frontend_add_event_callback(frontendEvent, g_guard);
	blog(LOG_INFO, "[replay-buffer-confirm] loaded");
	return true;
}

void obs_module_unload(void)
{
	if (g_guard) {
		obs_frontend_remove_event_callback(frontendEvent, g_guard);
		g_guard->prepareShutdown();
		delete g_guard;
		g_guard = nullptr;
	}
	blog(LOG_INFO, "[replay-buffer-confirm] unloaded");
}