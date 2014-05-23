/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * 
 *           Copyright 2014 Big Switch Networks, Inc.          
 * 
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * 
 *        http://www.eclipse.org/legal/epl-v10.html
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 * 
 * </bsn.cl>
 ************************************************************
 *
 * Power Supply Management.
 *
 ***********************************************************/

#include <onlp/onlp.h>
#include <onlp/oids.h>
#include <onlp/psu.h>
#include <onlp/platformi/psui.h>
#include "onlp_int.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

#define VALIDATENR(_id)                         \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return;                             \
        }                                       \
    } while(0)


static int
onlp_psu_present__(onlp_oid_t id, onlp_psu_info_t* info)
{
    int rv;
    VALIDATE(id);

    /* Info retrieval required. */
    rv = onlp_psui_info_get(id, info);
    if(rv < 0) {
        return rv;
    }
    /* The psu must be present. */
    if((info->status & 0x1) == 0) {
        return ONLP_STATUS_E_MISSING;
    }
    return ONLP_STATUS_OK;
}
#define ONLP_PSU_PRESENT_OR_RETURN(_id, _info)          \
    do {                                                \
        int _rv = onlp_psu_present__(_id, _info);       \
        if(_rv < 0) {                                   \
            return _rv;                                 \
        }                                               \
    } while(0)


int
onlp_psu_init(void)
{
    return onlp_psui_init();
}

int
onlp_psu_info_get(onlp_oid_t id,  onlp_psu_info_t* info)
{
    VALIDATE(id);
    return onlp_psui_info_get(id, info);
}

int
onlp_psu_ioctl(onlp_oid_t id, ...)
{
    int rv;
    onlp_psu_info_t info;
    va_list vargs;

    ONLP_PSU_PRESENT_OR_RETURN(id, &info);
    va_start(vargs, id);
    rv = onlp_psui_ioctl(id, vargs);
    va_end(vargs);
    return rv;
}



/************************************************************
 *
 * Debug and Show Functions
 *
 ***********************************************************/

void
onlp_psu_dump(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_psu_info_t info;

    VALIDATENR(id);
    onlp_oid_dump_iof_init_default(&iof, pvs);

    iof_push(&iof, "psu @ %d", ONLP_OID_ID_GET(id));
    rv = onlp_psu_info_get(id, &info);
    if(rv < 0) {
        onlp_oid_info_get_error(&iof, rv);
    }
    else {
        iof_iprintf(&iof, "Description: %s", info.hdr.description);
        if(info.status & 1) {
            /* Present */
            iof_iprintf(&iof, "Model:  %s", info.model[0] ? info.model : "Unknown.");
            iof_iprintf(&iof, "Status: %{onlp_psu_status_flags}", info.status);
            iof_iprintf(&iof, "Caps:   %{onlp_psu_caps_flags}", info.caps);
            iof_iprintf(&iof, "Vin:    %f", info.vin);
            iof_iprintf(&iof, "Vout:   %f", info.vout);
            iof_iprintf(&iof, "Iin:    %f", info.iin);
            iof_iprintf(&iof, "Iout:   %f", info.iout);
            iof_iprintf(&iof, "Pin:    %f", info.pin);
            iof_iprintf(&iof, "Pout:   %f", info.pout);
            if(flags & ONLP_OID_DUMP_F_RECURSE) {
                onlp_oid_table_dump(info.hdr.coids, &iof.inherit, flags);
            }
        }
        else {
            iof_iprintf(&iof, "Not present.");
        }
    }
    iof_pop(&iof);
}

void
onlp_psu_show(onlp_oid_t id, aim_pvs_t* pvs, uint32_t flags)
{
    int rv;
    iof_t iof;
    onlp_psu_info_t pi;

    onlp_oid_show_iof_init_default(&iof, pvs);
    rv = onlp_psu_info_get(id, &pi);

    iof_push(&iof, "PSU %d", ONLP_OID_ID_GET(id));
    if(rv < 0) {
        onlp_oid_info_get_error(&iof, rv);
    }
    else {
        onlp_oid_show_description(&iof, &pi.hdr);
        if(pi.status & 0x1) {
            /* Present */
            iof_iprintf(&iof, "State: Present");
            if(pi.status & ONLP_PSU_STATUS_UNPLUGGED) {
                iof_iprintf(&iof, "Status: Unplugged");
            }
            else if(pi.status & ONLP_PSU_STATUS_FAILED) {
                iof_iprintf(&iof, "Status: Unplugged or Failed.");
            }
            else {
                iof_iprintf(&iof, "Status: Running.");
                iof_iprintf(&iof, "Model: %s", pi.model[0] ? pi.model : "Unknown");
                if(pi.caps & ONLP_PSU_CAPS_AC) {
                    iof_iprintf(&iof, "Type: AC");
                }
                else if(pi.caps & ONLP_PSU_CAPS_DC12) {
                    iof_iprintf(&iof, "Type: DC 12V");
                }
                else if(pi.caps & ONLP_PSU_CAPS_DC48) {
                    iof_iprintf(&iof, "Type: DC 48V");
                }
                else {
                    iof_iprintf(&iof, "Type: Unknown.");
                }
                if(pi.caps & ONLP_PSU_CAPS_VIN) {
                    iof_iprintf(&iof, "Vin: %.1f",
                                onlp_float_normal(pi.vin));
                }
                if(pi.caps & ONLP_PSU_CAPS_VOUT) {
                    iof_iprintf(&iof, "Vout: %.1f", onlp_float_normal(pi.vout));
                }
                if(pi.caps & ONLP_PSU_CAPS_IIN) {
                    iof_iprintf(&iof, "Iin: %.1f", onlp_float_normal(pi.iin));
                }
                if(pi.caps & ONLP_PSU_CAPS_IOUT) {
                    iof_iprintf(&iof, "Iout: %.1f", onlp_float_normal(pi.iout));
                }
                if(pi.caps & ONLP_PSU_CAPS_PIN) {
                    iof_iprintf(&iof, "Pin: %.1f", onlp_float_normal(pi.pin));
                }
                if(pi.caps & ONLP_PSU_CAPS_POUT) {
                    iof_iprintf(&iof, "Pout: %.1f", onlp_float_normal(pi.pout));
                }

                if(flags & ONLP_OID_SHOW_F_RECURSE) {
                    /*
                     * Display sub oids.
                     *
                     * The standard version only includes
                     * Fans and Thermal Sensors.
                     */
                    onlp_oid_t* oidp;
                    ONLP_OID_TABLE_ITER_TYPE(pi.hdr.coids, oidp, FAN) {
                        onlp_oid_show(*oidp, &iof.inherit, flags);
                    }
                    ONLP_OID_TABLE_ITER_TYPE(pi.hdr.coids, oidp, THERMAL) {
                        onlp_oid_show(*oidp, &iof.inherit, flags);
                    }

                    if(flags & ONLP_OID_SHOW_F_EXTENDED) {
                        /* Include all other types as well. */
                        ONLP_OID_TABLE_ITER_TYPE(pi.hdr.coids, oidp, LED) {
                            onlp_oid_show(*oidp, &iof.inherit, flags);
                        }
                    }
                }
            }
        }
        else {
            /* Not present */
            onlp_oid_show_state_missing(&iof);
        }
    }
    iof_pop(&iof);
}
