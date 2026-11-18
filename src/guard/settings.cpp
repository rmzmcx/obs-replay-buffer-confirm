#include "settings.hpp"

#include <obs-module.h>

namespace {
constexpr const char *kConfigFile = "settings.json";
constexpr const char *kConfirmKey = "confirm_on_stop";

const char *ConfigPath()
{
	return obs_module_config_path(kConfigFile);
}
} // namespace

bool plugin_config::LoadConfirmEnabled()
{
	char *path = const_cast<char *>(ConfigPath());
	if (!path)
		return true;

	obs_data_t *data = obs_data_create_from_json_file(path);
	bool enabled = data ? obs_data_get_bool(data, kConfirmKey) : true;
	if (data)
		obs_data_release(data);
	bfree(path);
	return enabled;
}

void plugin_config::SaveConfirmEnabled(bool enabled)
{
	char *path = const_cast<char *>(ConfigPath());
	if (!path)
		return;

	obs_data_t *data = obs_data_create_from_json_file(path);
	if (!data)
		data = obs_data_create();

	obs_data_set_bool(data, kConfirmKey, enabled);
	obs_data_save_json(data, path);

	obs_data_release(data);
	bfree(path);
}