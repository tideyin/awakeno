// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"
#include "nvm.h"
#include "rlk_cmd.h"
#include "rlk_gpio.h"





extern uint8_t hw_ver ;



sys_parameter_t sys_para = {0};
uint8_t valid_vflg[VFLG_LENGTH] = {VALID_FLAG_L, VALID_FLAG_H};
uint8_t inval_vflg[VFLG_LENGTH] = {INVAL_FLAG_L, INVAL_FLAG_H};



void nvm_init(void) 
{
       uint8_t buf[64] = {0};
	   uint8_t flg_data[NVM_VFLG_LENGTH]= {0x77 , 0x88} ;
	   uint8_t usr_data[NVM_VFLG_LENGTH]= {0x77 , 0x88} ;
	   uint8_t default_dsn[DSN_LENGTH + 1] = "AWKN-0-0000-0000";
	   uint8_t init_nvm_ver[NVM_VER_LENGTH+1] = {NVM_VER,0} ;
	   uint8_t default_owner[OWNER_LENGTH + 1] = "USER";
	   uint8_t default_login[LOGIN_LENGTH + 1] = "user@";	   
	   uint8_t default_locat[LOCAT_LENGTH + 1] = "Beijing";
	   uint8_t default_prduct[LOCAT_LENGTH + 1] = "PRD-AWKN";
	   uint8_t read_data[30];
	   uint8_t default_hw_ver=0x0 ;


	      i2c_eeprom_init();
			  eeprom_buffer_read_wrapper(flg_data, NVM_VFLG_OFFSET, NVM_VFLG_LENGTH);
			  if (flg_data[0] != VALID_FLAG_L || flg_data[1] != VALID_FLAG_H) {
				  printf_("\r\nNVM Data is Empty ! Init... ") ; 	  
				  memcpy(sys_para.nvm_vflg, valid_vflg, NVM_VFLG_LENGTH);
				  eeprom_buffer_write_wrapper(sys_para.nvm_vflg, NVM_VFLG_OFFSET, NVM_VFLG_LENGTH); 	  
				  memcpy(sys_para.nvm_ver, init_nvm_ver, NVM_VER_LENGTH);
				  eeprom_buffer_write_wrapper(init_nvm_ver, NVM_VER_OFFSET, NVM_VER_LENGTH);
			  }
			  else
			  {
				  eeprom_buffer_read_wrapper(init_nvm_ver, NVM_VER_OFFSET, NVM_VER_LENGTH);
				  printf_("\r\nNVM Version:0x%x", init_nvm_ver[0]);
			  }
	   
	   
	   // Read DSN from EEPROM	  
			  eeprom_buffer_read_wrapper(flg_data, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);
			  uint8_t dsn_data[DSN_LENGTH + 1];
			  uint8_t hwver_data[HWVER_LENGTH];
			  if (flg_data[0] != VALID_FLAG_L || flg_data[1] != VALID_FLAG_H) {
				  printf_("\r\nDSN: %s", default_dsn);
				  printf_("\r\nHW_VER: %d", default_hw_ver);
				  hw_ver = default_hw_ver ;
				  
			  } else {
				  eeprom_buffer_read_wrapper(dsn_data, DSN_OFFSET, DSN_LENGTH);
				  dsn_data[DSN_LENGTH] = '\0';			  
				  printf_("\r\nDSN:%s", dsn_data);
	   
				  eeprom_buffer_read_wrapper(hwver_data, HWVER_OFFSET, HWVER_LENGTH);
				  if(hwver_data[0]>2)
				  {
					 hw_ver = default_hw_ver ;
				  }
				  else
				  {
					hw_ver = hwver_data[0] ;
				  } 	   
				  printf_("\r\nHW_VER: %d", hw_ver);		  
			  }
	   
	   // Read User Info from EEPROM
			  eeprom_buffer_read_wrapper(usr_data, USR_VFLG_OFFSET, USR_VFLG_LENGTH);
			  uint8_t user_data[USER_LENGTH + 1];
			  if (usr_data[0] != VALID_FLAG_L || usr_data[1] != VALID_FLAG_H) {
				  printf_("\r\nOWNER dft: %s", default_owner);
				  printf_("\r\nLOGIN dft: %s", default_login);
				printf_("\r\nLOCAT dft: %s", default_locat);
			  } else {
				  eeprom_buffer_read_wrapper(user_data, OWNER_OFFSET, OWNER_LENGTH);
				  user_data[DSN_LENGTH] = '\0'; 	  
				  printf_("\r\nOWNER nvm:%s", user_data);
				  eeprom_buffer_read_wrapper(user_data, LOGIN_OFFSET, LOGIN_LENGTH);
				  user_data[LOGIN_LENGTH] = '\0';	  
				  printf_("\r\nLOGIN nvm:%s", user_data);		  
				  eeprom_buffer_read_wrapper(user_data, LOCAT_OFFSET, LOCAT_LENGTH);
				  user_data[LOCAT_LENGTH] = '\0';	  
				  printf_("\r\nLOCAT nvm:%s", user_data);		  
			  }
	   
	  


}	   


void nvm_hw_init(void) 
{


}



void sys_para_load(void)
{
	return ;
  //  eeprom_buffer_read_wrapper((uint8_t*)&sys_para, 0, sizeof(sys_para));
}


