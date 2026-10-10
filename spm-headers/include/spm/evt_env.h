#pragma once

#include <common.h>
#include <evt_cmd.h>
#include <wii/gx.h>

CPP_WRAPPER(spm::evt_env)

USING(wii::gx::GXTexObj)

EVT_DECLARE_USER_FUNC(evt_env_blur_on, 2)
EVT_DECLARE_USER_FUNC(evt_env_static_blur_on, 0)
EVT_DECLARE_USER_FUNC(evt_env_warp_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_env_set_warp_alpha, 1)
EVT_DECLARE_USER_FUNC(evt_env_screen_warp_on, 0)
void evtEnvRefractDisp(GXTexObj * texObj);
EVT_DECLARE_USER_FUNC(evt_env_refract_on, 1)

CPP_WRAPPER_END()
