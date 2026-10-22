#pragma once

#include <common.h>
#include <evt_cmd.h>

CPP_WRAPPER(spm::evt_nand)

EVT_DECLARE_USER_FUNC(evt_nand_write_save, 2)
EVT_DECLARE_USER_FUNC(evt_nand_update_save, 0)

EVT_DECLARE(evt_memcard_save)
EVT_DECLARE(savefile_update_evt)
EVT_DECLARE(delete_save_confirm_evt)
EVT_DECLARE(copy_save_confirm_evt)

CPP_WRAPPER_END()
