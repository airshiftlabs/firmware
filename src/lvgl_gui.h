/*
 *  lvgl_gui.h
 *
 *  Created on: May 5, 2023
 */

#ifndef __LVGL_GUI__H_
#define __LVGL_GUI__H_

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/



/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/



/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/



/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/

/**
 * @brief  Print sensor datas
 * @param  CO2, temperature and PM2.5 values
 * @retval None
 */
void lvgl_gui_print(int co2, float temp, int pm2);

/**
 * @brief  Display image screen
 * @param  None
 * @retval None
 */
void lvgl_gui_sensor_start(void);

/**
 * @brief  Initialize LVGL for gui
 * @param  None
 * @retval None
 */
void lvgl_gui_init(void);

/******************************************************************************/

#endif /* __LVGL_GUI__H_ */
