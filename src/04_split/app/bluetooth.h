#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "fds.h"
#include "ble_conn_state.h"
#include "ble_dis.h"
#include "nrf_ble_bms.h"
#include "nrf_ble_gatt.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

void ble_stack_init(void);
void gap_params_init(void);
void gatt_init(void);
void advertising_init(void);
void services_init(void);
void conn_params_init(void);
void peer_manager_init(void);
void advertising_init(void);
void advertising_start(bool erase_bonds);
void idle_state_handle(void);