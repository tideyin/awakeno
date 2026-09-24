// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef __NVM_H
#define __NVM_H


#define VALID_FLAG_L 0xA5
#define VALID_FLAG_H 0x5A
#define INVAL_FLAG_L 0xF0
#define INVAL_FLAG_H 0x0F

#define VFLG_LENGTH 0x2  //valid flag length
#define USER_LENGTH 0x10


/************************************************
*   NVM Layout        V01                       *
*************************************************/
#define NVM_VER_OFFSET 0
#define NVM_VER_LENGTH 1

#define NVM_VFLG_OFFSET NVM_VER_OFFSET+NVM_VER_LENGTH
#define NVM_VFLG_LENGTH VFLG_LENGTH


#define DSN_OFFSET NVM_VFLG_OFFSET+NVM_VFLG_LENGTH
#define DSN_LENGTH 16

#define DSN_VFLG_OFFSET DSN_OFFSET+DSN_LENGTH
#define DSN_VFLG_LENGTH VFLG_LENGTH

#define OWNER_OFFSET DSN_VFLG_OFFSET+DSN_VFLG_LENGTH
#define OWNER_LENGTH USER_LENGTH

#define LOGIN_OFFSET OWNER_OFFSET+OWNER_LENGTH
#define LOGIN_LENGTH USER_LENGTH

#define LOCAT_OFFSET LOGIN_OFFSET+LOGIN_LENGTH
#define LOCAT_LENGTH USER_LENGTH

#define USR_VFLG_OFFSET LOCAT_OFFSET+LOCAT_LENGTH  //For owner/login/location
#define USR_VFLG_LENGTH VFLG_LENGTH


/* Protected by PRD_VFLG */
#define PRODUCT_OFFSET    USR_VFLG_OFFSET+USR_VFLG_LENGTH
#define PRODUCT_LENGTH    12

#define PRD_VFLG_OFFSET    PRODUCT_OFFSET+PRODUCT_LENGTH
#define PRD_VFLG_LENGTH    VFLG_LENGTH


/* Protected by PRD_VFLG end */

// Protected by DSN_VFLG
#define HWVER_OFFSET PRD_VFLG_OFFSET+PRD_VFLG_LENGTH 
#define HWVER_LENGTH 1







#define NVM_VER 0x1


typedef struct {
    uint8_t nvm_vflg[VFLG_LENGTH];
    uint8_t nvm_ver[VFLG_LENGTH];
    uint8_t dsn_vflg[VFLG_LENGTH];	
    uint8_t dsn[DSN_LENGTH];
  	uint8_t prd_vflg[PRD_VFLG_LENGTH];
    uint8_t product[PRODUCT_LENGTH];
    uint8_t usr_vflg[VFLG_LENGTH];
    uint8_t owner[OWNER_LENGTH];
    uint8_t login[LOGIN_LENGTH];
    uint8_t locat[LOCAT_LENGTH];
    uint8_t hwver[HWVER_LENGTH];
}sys_parameter_t;

void nvm_init(void) ;
void nvm_hw_init(void) ;
void sys_para_load(void) ;



#endif


