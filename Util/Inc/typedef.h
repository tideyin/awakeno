#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

/* we just reserve the OK/General_Err here */
typedef enum {
    /**
     * @name General
     * @brief Basic status returns.
     *        Reserved value[0 to -99]
     * @{
     */

    /** Operation completed successfully */
    STATUS_OK = 0,
    /** Unspecified run-time error */
    STATUS_GENERAL_ERROR = -1,
    STATUS_NOT_SUPPORTED = -6,

    /* @} */
} status_t;

#endif /* _TYPEDEF_H_ */
