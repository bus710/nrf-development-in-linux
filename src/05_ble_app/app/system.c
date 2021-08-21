
#include "system.h"
#include "bluetooth.h"

extern bool m_rr_interval_enabled;           /**< Flag for enabling and disabling the registration of new RR interval measurements (the purpose of disabling this is just to test sending HRM without RR interval data. */
extern TimerHandle_t m_battery_timer;        /**< Definition of battery timer. */
extern TimerHandle_t m_heart_rate_timer;     /**< Definition of heart rate timer. */
extern TimerHandle_t m_rr_interval_timer;    /**< Definition of RR interval timer. */
extern TimerHandle_t m_sensor_contact_timer; /**< Definition of sensor contact detected timer. */

static sensorsim_cfg_t m_battery_sim_cfg;         /**< Battery Level sensor simulator configuration. */
static sensorsim_state_t m_battery_sim_state;     /**< Battery Level sensor simulator state. */
static sensorsim_cfg_t m_heart_rate_sim_cfg;      /**< Heart Rate sensor simulator configuration. */
static sensorsim_state_t m_heart_rate_sim_state;  /**< Heart Rate sensor simulator state. */
static sensorsim_cfg_t m_rr_interval_sim_cfg;     /**< RR Interval sensor simulator configuration. */
static sensorsim_state_t m_rr_interval_sim_state; /**< RR Interval sensor simulator state. */

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
void battery_level_update(void)
{
    ret_code_t err_code;
    uint8_t battery_level;

    battery_level = (uint8_t)sensorsim_measure(&m_battery_sim_state, &m_battery_sim_cfg);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
    {
        APP_ERROR_HANDLER(err_code);
    }
}

/**@brief Function for handling the Battery measurement timer time-out.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] xTimer Handler to the timer that called this function.
 *                   You may get identifier given to the function xTimerCreate using pvTimerGetTimerID.
 */
void battery_level_meas_timeout_handler(TimerHandle_t xTimer)
{
    UNUSED_PARAMETER(xTimer);
    battery_level_update();
}

/**@brief Function for handling the Heart rate measurement timer time-out.
 *
 * @details This function will be called each time the heart rate measurement timer expires.
 *          It will exclude RR Interval data from every third measurement.
 *
 * @param[in] xTimer Handler to the timer that called this function.
 *                   You may get identifier given to the function xTimerCreate using pvTimerGetTimerID.
 */
void heart_rate_meas_timeout_handler(TimerHandle_t xTimer)
{
    // static uint32_t cnt = 0;
    // ret_code_t err_code;
    // uint16_t heart_rate;

    // UNUSED_PARAMETER(xTimer);

    // heart_rate = (uint16_t)sensorsim_measure(&m_heart_rate_sim_state, &m_heart_rate_sim_cfg);

    // cnt++;
    // err_code = ble_hrs_heart_rate_measurement_send(&m_hrs, heart_rate);
    // if ((err_code != NRF_SUCCESS) &&
    //     (err_code != NRF_ERROR_INVALID_STATE) &&
    //     (err_code != NRF_ERROR_RESOURCES) &&
    //     (err_code != NRF_ERROR_BUSY) &&
    //     (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
    // {
    //     APP_ERROR_HANDLER(err_code);
    // }

    // Disable RR Interval recording every third heart rate measurement.
    // NOTE: An application will normally not do this. It is done here just for testing generation
    // of messages without RR Interval measurements.
    // m_rr_interval_enabled = ((cnt % 3) != 0);
}

/**@brief Function for handling the RR interval timer time-out.
 *
 * @details This function will be called each time the RR interval timer expires.
 *
 * @param[in] xTimer Handler to the timer that called this function.
 *                   You may get identifier given to the function xTimerCreate using pvTimerGetTimerID.
 */
void rr_interval_timeout_handler(TimerHandle_t xTimer)
{
    UNUSED_PARAMETER(xTimer);

    if (m_rr_interval_enabled)
    {
        uint16_t rr_interval;

        rr_interval = (uint16_t)sensorsim_measure(&m_rr_interval_sim_state,
                                                  &m_rr_interval_sim_cfg);
        ble_hrs_rr_interval_add(&m_hrs, rr_interval);
    }
}

/**@brief Function for handling the Sensor Contact Detected timer time-out.
 *
 * @details This function will be called each time the Sensor Contact Detected timer expires.
 *
 * @param[in] xTimer Handler to the timer that called this function.
 *                   You may get identifier given to the function xTimerCreate using pvTimerGetTimerID.
 */
void sensor_contact_detected_timeout_handler(TimerHandle_t xTimer)
{
    static bool sensor_contact_detected = false;

    UNUSED_PARAMETER(xTimer);

    sensor_contact_detected = !sensor_contact_detected;
    ble_hrs_sensor_contact_detected_update(&m_hrs, sensor_contact_detected);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.
    m_battery_timer = xTimerCreate("BATT",
                                   BATTERY_LEVEL_MEAS_INTERVAL,
                                   pdTRUE,
                                   NULL,
                                   battery_level_meas_timeout_handler);
    m_heart_rate_timer = xTimerCreate("HRT",
                                      HEART_RATE_MEAS_INTERVAL,
                                      pdTRUE,
                                      NULL,
                                      heart_rate_meas_timeout_handler);
    m_rr_interval_timer = xTimerCreate("RRT",
                                       RR_INTERVAL_INTERVAL,
                                       pdTRUE,
                                       NULL,
                                       rr_interval_timeout_handler);
    m_sensor_contact_timer = xTimerCreate("SCT",
                                          SENSOR_CONTACT_DETECTED_INTERVAL,
                                          pdTRUE,
                                          NULL,
                                          sensor_contact_detected_timeout_handler);

    /* Error checking */
    if ((NULL == m_battery_timer) || (NULL == m_heart_rate_timer) || (NULL == m_rr_interval_timer) || (NULL == m_sensor_contact_timer))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
}

/**@brief Function for initializing the sensor simulators. */
void sensor_simulator_init(void)
{
    m_battery_sim_cfg.min = MIN_BATTERY_LEVEL;
    m_battery_sim_cfg.max = MAX_BATTERY_LEVEL;
    m_battery_sim_cfg.incr = BATTERY_LEVEL_INCREMENT;
    m_battery_sim_cfg.start_at_max = true;

    sensorsim_init(&m_battery_sim_state, &m_battery_sim_cfg);

    m_heart_rate_sim_cfg.min = MIN_HEART_RATE;
    m_heart_rate_sim_cfg.max = MAX_HEART_RATE;
    m_heart_rate_sim_cfg.incr = HEART_RATE_INCREMENT;
    m_heart_rate_sim_cfg.start_at_max = false;

    sensorsim_init(&m_heart_rate_sim_state, &m_heart_rate_sim_cfg);

    m_rr_interval_sim_cfg.min = MIN_RR_INTERVAL;
    m_rr_interval_sim_cfg.max = MAX_RR_INTERVAL;
    m_rr_interval_sim_cfg.incr = RR_INTERVAL_INCREMENT;
    m_rr_interval_sim_cfg.start_at_max = false;

    sensorsim_init(&m_rr_interval_sim_state, &m_rr_interval_sim_cfg);
}

/**@brief   Function for starting application timers.
 * @details Timers are run after the scheduler has started.
 */
void application_timers_start(void)
{
    // Start application timers.
    if (pdPASS != xTimerStart(m_battery_timer, OSTIMER_WAIT_FOR_QUEUE))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
    if (pdPASS != xTimerStart(m_heart_rate_timer, OSTIMER_WAIT_FOR_QUEUE))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
    if (pdPASS != xTimerStart(m_rr_interval_timer, OSTIMER_WAIT_FOR_QUEUE))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
    if (pdPASS != xTimerStart(m_sensor_contact_timer, OSTIMER_WAIT_FOR_QUEUE))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
}

/**@brief Function for initializing the nrf log module.
 */
void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
void buttons_leds_init(bool *p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

/**@brief Function for initializing the clock.
 */
void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}
