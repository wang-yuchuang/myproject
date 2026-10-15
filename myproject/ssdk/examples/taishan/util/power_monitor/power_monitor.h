/*
 * power_monitor.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level power monitor example.
 *
 * Revision History:
 * -----------------
 */

/**
 * @brief voltage channel.
 */
struct voltage_channel {
    const char  *name;

    /* ADC controller */
    unsigned int adc;

    /* ADC channel selection. */
    unsigned int adc_chnl;

    /* Multiple voltage inputs may share one ADC channel, and they should
     * be selected by analog mux.
     */
    unsigned int mux_chnl;

    /* This function is called to prepare the chanel, before ADC sampling. */
    void (*prepare)(struct voltage_channel *voltage);

    /*
     *  When measuring Voltage, ratio_mul and ratio_div correspond to
     *  Board level voltage division resistors.
     *      Target voltage = adc_sample * ratio_mul / ratio_div;
     *  When measuring Current, ratio_mul and ratio_div correspond to
     *  1 and the serial resistor.
     *      Target current = adc_sample * ratio_mul / ratio_div
     *                     = adc_sample / serial_resistance
     *                     = adc_sample * 1000 / (serial_resistance * 1000);
     */
    unsigned int ratio_mul;
    unsigned int ratio_div;

    /* True when this voltage channel is the "negative" voltage channel for
     * current measurement.
     */
    bool diff_n;

    /* ADC input pin number. This number is used to configure digital/analog
     * selection SCR, as well as pinmux.
     *
     * TODO - pad configuration will be added to pinmux driver later.
     */
    unsigned int pin;
    unsigned int pin_neg;
};

/**
 * @brief Current measurement channel.
 *
 * A current measurement channel is composed of positive and negative
 * voltage channels, and high resolution resistor.
 */
struct current_channel {
    /* name used to find positive and negative voltage channels. */
    const char  *name;

    /* resistor in micro ohm. */
    unsigned int m_ohm;
};

