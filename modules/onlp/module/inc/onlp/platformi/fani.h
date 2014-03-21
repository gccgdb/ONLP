/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 * Fan Platform Implementation.
 *
 ***********************************************************/
#ifndef __ONLP_FANI_H__
#define __ONLP_FANI_H__

#include <onlp/fan.h>

/**
 * @brief Initialize the fan platform subsystem.
 */
int onlp_fani_init(void);

/**
 * @brief Get the information structure for the given fan OID.
 * @param id The fan OID
 * @param rv [out] Receives the fan information.
 */
int onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* rv);

/**
 * @brief Set the fan speed in RPM.
 * @param id The fan OID
 * @param rpm The new RPM
 * @note This is only relevant if the RPM capability is set.
 */
int onlp_fani_rpm_set(onlp_oid_t id, int rpm);


/**
 * @brief Set the fan speed in percentage.
 * @param id The fan OID.
 * @param p The new fan speed percentage.
 * @note This is only relevant if the PERCENTAGE capability is set.
 */
int onlp_fani_percentage_set(int fid, int p);

/**
 * @brief Set the fan mode.
 * @param id The fan OID.
 * @param mode The new fan mode.
 */
int onlp_fani_mode_set(int fid, onlp_fan_mode_t mode);


/**
 * @brief Set the fan direction (if supported).
 * @param id The fan OID
 * @param dir The direction.
 */
int onlp_fan_dir_set(int fid, onlp_fan_dir_t dir);

/**
 * @brief Generic fan ioctl
 * @param id The fan OID
 * @param vargs The variable argument list for the ioctl call.
 * @param Optional
 */
int onlp_fani_ioctl(onlp_oid_t fid, va_list vargs);

#endif /* __ONLP_FANI_H__ */

