#include "system.h"
#include "bluetooth.h"

#define DEVICE_NAME "Nordic_BMS"                //!< Name of device. Will be included in the advertising data.
#define MANUFACTURER_NAME "NordicSemiconductor" //!< Manufacturer. Will be passed to Device Information Service.

#define APP_BLE_OBSERVER_PRIO 3 //!< Application's BLE observer priority. You shouldn't need to modify this value.
#define APP_BLE_CONN_CFG_TAG 1  //!< A tag identifying the SoftDevice BLE configuration.

#define SECOND_10_MS_UNITS 100                    //!< Definition of 1 second, when 1 unit is 10 ms.
#define MIN_CONN_INTERVAL 7                       //!< Minimum acceptable connection interval (0.25 seconds), Connection interval uses 1.25 ms units.
#define MAX_CONN_INTERVAL 400                     //!< Maximum acceptable connection interval (0.5 second), Connection interval uses 1.25 ms units.
#define SLAVE_LATENCY 0                           //!< Slave latency.
#define CONN_SUP_TIMEOUT (4 * SECOND_10_MS_UNITS) //!< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units.

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(15000) //!< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds).
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000)   //!< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds).
#define MAX_CONN_PARAMS_UPDATE_COUNT 3                        //!< Number of attempts before giving up the connection parameter negotiation.

#define SEC_PARAM_BOND 1                               //!< Perform bonding.
#define SEC_PARAM_MITM 0                               //!< Man In The Middle protection not required.
#define SEC_PARAM_LESC 0                               //!< LE Secure Connections not enabled.
#define SEC_PARAM_KEYPRESS 0                           //!< Keypress notifications not enabled.
#define SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE //!< No I/O capabilities.
#define SEC_PARAM_OOB 0                                //!< Out Of Band data not available.
#define SEC_PARAM_MIN_KEY_SIZE 7                       //!< Minimum encryption key size.
#define SEC_PARAM_MAX_KEY_SIZE 16                      //!< Maximum encryption key size.

#define APP_ADV_FAST_INTERVAL 0x0028 //!< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.).
#define APP_ADV_DURATION 18000       //!< The advertising duration (180 seconds) in units of 10 milliseconds.

#define MEM_BUFF_SIZE 512
#define DEAD_BEEF 0xDEADBEEF //!< Value used as error code on stack dump, can be used to identify stack location on stack unwind.

NRF_BLE_QWR_DEF(m_qwr);             //!< Context for the Queued Write module.
NRF_BLE_BMS_DEF(m_bms);             //!< Structure used to identify the Bond Management service.
NRF_BLE_GATT_DEF(m_gatt);           //!< GATT module instance.
BLE_ADVERTISING_DEF(m_advertising); //!< Advertising module instance.

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;    //!< Handle of the current connection.
static uint8_t m_qwr_mem[MEM_BUFF_SIZE];                    //!< Write buffer for the Queued Write module.
static ble_conn_state_user_flag_id_t m_bms_bonds_to_delete; //!< Flags used to identify bonds that should be deleted.

static ble_uuid_t m_adv_uuids[] = {
    {BLE_UUID_BMS_SERVICE, BLE_UUID_TYPE_BLE},
}; //!< Universally unique service identifiers.

#ifdef USE_AUTHORIZATION_CODE
static uint8_t m_auth_code[] = {'A', 'B', 'C', 'D'}; //0x41, 0x42, 0x43, 0x44
static int m_auth_code_len = sizeof(m_auth_code);
#endif

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling advertising errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void ble_advertising_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
    }
    else
    {
        uint32_t ret;

        ret = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(ret);
    }
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
void gap_params_init(void)
{
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from bond management service.
 */
void bms_evt_handler(nrf_ble_bms_t *p_ess, nrf_ble_bms_evt_t *p_evt)
{
    ret_code_t err_code;
    bool is_authorized = true;

    switch (p_evt->evt_type)
    {
    case NRF_BLE_BMS_EVT_AUTH:
        NRF_LOG_DEBUG("Authorization request.");
#if USE_AUTHORIZATION_CODE
        if ((p_evt->auth_code.len != m_auth_code_len) ||
            (memcmp(m_auth_code, p_evt->auth_code.code, m_auth_code_len) != 0))
        {
            is_authorized = false;
        }
#endif
        err_code = nrf_ble_bms_auth_response(&m_bms, is_authorized);
        APP_ERROR_CHECK(err_code);
    }
}

uint16_t qwr_evt_handler(nrf_ble_qwr_t *p_qwr, nrf_ble_qwr_evt_t *p_evt)
{
    return nrf_ble_bms_on_qwr_evt(&m_bms, p_qwr, p_evt);
}

/**@brief Function for deleting a single bond if it does not belong to a connected peer.
 *
 * This will mark the bond for deferred deletion if the peer is connected.
 */
static void bond_delete(uint16_t conn_handle, void *p_context)
{
    UNUSED_PARAMETER(p_context);
    ret_code_t err_code;
    pm_peer_id_t peer_id;

    if (ble_conn_state_status(conn_handle) == BLE_CONN_STATUS_CONNECTED)
    {
        ble_conn_state_user_flag_set(conn_handle, m_bms_bonds_to_delete, true);
    }
    else
    {
        NRF_LOG_DEBUG("Attempting to delete bond.");
        err_code = pm_peer_id_get(conn_handle, &peer_id);
        APP_ERROR_CHECK(err_code);
        if (peer_id != PM_PEER_ID_INVALID)
        {
            err_code = pm_peer_delete(peer_id);
            APP_ERROR_CHECK(err_code);
            ble_conn_state_user_flag_set(conn_handle, m_bms_bonds_to_delete, false);
        }
    }
}

/**@brief Function for performing deferred deletions.
*/
static void delete_disconnected_bonds(void)
{
    uint32_t n_calls = ble_conn_state_for_each_set_user_flag(m_bms_bonds_to_delete, bond_delete, NULL);
    UNUSED_RETURN_VALUE(n_calls);
}

/**@brief Function for marking the requester's bond for deletion.
*/
static void delete_requesting_bond(nrf_ble_bms_t const *p_bms)
{
    NRF_LOG_INFO("Client requested that bond to current device deleted");
    ble_conn_state_user_flag_set(p_bms->conn_handle, m_bms_bonds_to_delete, true);
}

/**@brief Function for deleting all bonds
*/
static void delete_all_bonds(nrf_ble_bms_t const *p_bms)
{
    ret_code_t err_code;
    uint16_t conn_handle;

    NRF_LOG_INFO("Client requested that all bonds be deleted");

    pm_peer_id_t peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
    while (peer_id != PM_PEER_ID_INVALID)
    {
        err_code = pm_conn_handle_get(peer_id, &conn_handle);
        APP_ERROR_CHECK(err_code);

        bond_delete(conn_handle, NULL);

        peer_id = pm_next_peer_id_get(peer_id);
    }
}

/**@brief Function for deleting all bet requesting device bonds
*/
static void delete_all_except_requesting_bond(nrf_ble_bms_t const *p_bms)
{
    ret_code_t err_code;
    uint16_t conn_handle;

    NRF_LOG_INFO("Client requested that all bonds except current bond be deleted");

    pm_peer_id_t peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
    while (peer_id != PM_PEER_ID_INVALID)
    {
        err_code = pm_conn_handle_get(peer_id, &conn_handle);
        APP_ERROR_CHECK(err_code);

        /* Do nothing if this is our own bond. */
        if (conn_handle != p_bms->conn_handle)
        {
            bond_delete(conn_handle, NULL);
        }

        peer_id = pm_next_peer_id_get(peer_id);
    }
}

/**@brief Function for initializing the services that will be used by the application.
 *
 * @details Initialize the Bond Management and Device Information services.
 */

void services_init(void)
{
    ret_code_t err_code;
    ble_dis_init_t dis_init;
    nrf_ble_bms_init_t bms_init;
    nrf_ble_qwr_init_t qwr_init;

    // Initialize Queued Write Module
    memset(&qwr_init, 0, sizeof(qwr_init));
    qwr_init.mem_buffer.len = MEM_BUFF_SIZE;
    qwr_init.mem_buffer.p_mem = m_qwr_mem;
    qwr_init.callback = qwr_evt_handler;
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Bond Management Service
    memset(&bms_init, 0, sizeof(bms_init));

    m_bms_bonds_to_delete = ble_conn_state_user_flag_acquire();
    bms_init.evt_handler = bms_evt_handler;
    bms_init.error_handler = service_error_handler;
#if USE_AUTHORIZATION_CODE
    bms_init.feature.delete_requesting_auth = true;
    bms_init.feature.delete_all_auth = true;
    bms_init.feature.delete_all_but_requesting_auth = true;
#else
    bms_init.feature.delete_requesting = true;
    bms_init.feature.delete_all = true;
    bms_init.feature.delete_all_but_requesting = true;
#endif
    bms_init.bms_feature_sec_req = SEC_JUST_WORKS;
    bms_init.bms_ctrlpt_sec_req = SEC_JUST_WORKS;

    bms_init.p_qwr = &m_qwr;
    bms_init.bond_callbacks.delete_requesting = delete_requesting_bond;
    bms_init.bond_callbacks.delete_all = delete_all_bonds;
    bms_init.bond_callbacks.delete_all_except_requesting = delete_all_except_requesting_bond;

    err_code = nrf_ble_bms_init(&m_bms, &bms_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Connection Parameter events.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail configuration parameter, but instead we use the
 *                event handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    ret_code_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
        NRF_LOG_INFO("Fast adverstising.");
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_ADV_EVT_IDLE:
        sleep_mode_enter();
        break;
    default:
        break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    pm_handler_secure_on_connection(p_ble_evt);

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_INFO("Connected");
        err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
        APP_ERROR_CHECK(err_code);
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code = nrf_ble_bms_set_conn_handle(&m_bms, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_INFO("Disconnected");
        delete_disconnected_bonds();
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        NRF_LOG_DEBUG("GATT Client Timeout.");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        NRF_LOG_DEBUG("GATT Server Timeout.");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const *p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
    case PM_EVT_PEERS_DELETE_SUCCEEDED:
        advertising_start(false);
        break;

    default:
        break;
    }
}

/**@brief Function for the Peer Manager initialization.
 */
void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond = SEC_PARAM_BOND;
    sec_param.mitm = SEC_PARAM_MITM;
    sec_param.lesc = SEC_PARAM_LESC;
    sec_param.keypress = SEC_PARAM_KEYPRESS;
    sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob = SEC_PARAM_OOB;
    sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc = 1;
    sec_param.kdist_own.id = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    ret_code_t err_code;
    uint8_t adv_flags;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    adv_flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = true;
    init.advdata.flags = adv_flags;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_whitelist_enabled = false;
    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_FAST_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;
    init.config.ble_adv_slow_enabled = false;

    init.evt_handler = on_adv_evt;
    init.error_handler = ble_advertising_error_handler;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}
