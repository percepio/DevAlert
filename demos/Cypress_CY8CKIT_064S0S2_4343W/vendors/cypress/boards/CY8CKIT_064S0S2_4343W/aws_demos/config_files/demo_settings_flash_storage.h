/*
 * demo_settings_flash_storage.h
 *
 *  Created on: 3 okt. 2019
 *      Author: johan
 */

#if !defined(DEMO_SETTINGS_FLASH_STORAGE_H)
#define DEMO_SETTINGS_FLASH_STORAGE_H
//#ifndef _DEMO_SETTINGS_FLASH_STORAGE_H_
//#define _DEMO_SETTINGS_FLASH_STORAGE_H_

#define VALID_ENTRY_VALUE 0xF0F1F2F3

typedef struct{
	uint32_t validEntry;
	const char mqtt_broker_endpoint[64];
	const char device_name[64];
	const char wifi_ssid[64];
	const char wifi_password[64];
	int wifi_security;
} ClientCredentials_t;

extern ClientCredentials_t flashClientCredentials;

extern uint8_t* tmpCertificate;
extern uint8_t* tmpPrivateKey;

#endif /* APPLICATION_CODE_COMMON_DEMOS_INCLUDE_DEMO_SETTINGS_FLASH_STORAGE_H_ */
