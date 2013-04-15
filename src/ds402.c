/* -*- mode: C; c-basic-offset: 4 -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2008-2013, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Neil T. Dantam <ntd@gatech.edu>
 * Georgia Tech Humanoid Robotics Lab
 * Under Direction of Prof. Mike Stilman <mstilman@cc.gatech.edu>
 *
 *
 * This file is provided under the following "BSD-style" License:
 *
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <assert.h>
#include "socanmatic.h"
#include "socanmatic/dict402.h"

enum canmat_402_state_val canmat_402_state( const struct canmat_402_drive *drive ) {
    uint16_t s = drive->stat_word;
    if( CANMAT_402_STATE_VAL_OFF_NRDY == (s & CANMAT_402_STATE_VAL_MASK_OFF_NRDY) ) {
        return CANMAT_402_STATE_VAL_OFF_NRDY;
    } else if  ( CANMAT_402_STATE_VAL_OFF_SW_ON_DISABLE == (s & CANMAT_402_STATE_VAL_MASK_OFF_SW_ON_DISABLE) ) {
        return CANMAT_402_STATE_VAL_OFF_SW_ON_DISABLE;
    } else if  ( CANMAT_402_STATE_VAL_OFF_RDY == (s & CANMAT_402_STATE_VAL_MASK_OFF_RDY) ) {
        return CANMAT_402_STATE_VAL_OFF_RDY;
    } else if  ( CANMAT_402_STATE_VAL_ON_OP_DIS == (s & CANMAT_402_STATE_VAL_MASK_ON_OP_DIS) ) {
        return CANMAT_402_STATE_VAL_ON_OP_DIS;
    } else if  ( CANMAT_402_STATE_VAL_ON_OP_EN == (s & CANMAT_402_STATE_VAL_MASK_ON_OP_EN) ) {
        return CANMAT_402_STATE_VAL_ON_OP_EN;
    } else if  ( CANMAT_402_STATE_VAL_ON_QUICK_STOP == (s & CANMAT_402_STATE_VAL_MASK_ON_QUICK_STOP )) {
        return CANMAT_402_STATE_VAL_ON_QUICK_STOP;
    } else if  ( CANMAT_402_STATE_VAL_FAULT_REACTION_ACTIVE ==
                 (s & CANMAT_402_STATE_VAL_MASK_FAULT_REACTION_ACTIVE) ) {
        return CANMAT_402_STATE_VAL_FAULT_REACTION_ACTIVE;
    } else if  ( CANMAT_402_STATE_VAL_FAULT == (s & CANMAT_402_STATE_VAL_MASK_FAULT) ) {
        return CANMAT_402_STATE_VAL_FAULT;
    } else {
        return CANMAT_402_STATE_VAL_UNKNOWN;
    }
}

const char *canmat_402_state_string( enum canmat_402_state_val s ) {
    switch(s) {
    case CANMAT_402_STATE_VAL_OFF_NRDY: return "Not ready to switch on";
    case CANMAT_402_STATE_VAL_OFF_SW_ON_DISABLE: return "Off, switch on disabled";
    case CANMAT_402_STATE_VAL_OFF_RDY: return "Off, ready to switch on";
    case CANMAT_402_STATE_VAL_ON_OP_DIS: return "On, operation disabled";
    case CANMAT_402_STATE_VAL_ON_OP_EN: return "On, operation enabled";
    case CANMAT_402_STATE_VAL_ON_QUICK_STOP: return "Quick stop";
    case CANMAT_402_STATE_VAL_FAULT_REACTION_ACTIVE: return "Fault reaction active";
    case CANMAT_402_STATE_VAL_FAULT: return "Fault";
    case CANMAT_402_STATE_VAL_UNKNOWN: return "?";
    }
    return "?";
}

static enum canmat_status dl_control( struct canmat_iface *cif, struct canmat_402_drive *drive, uint16_t ctrl ) {
    return canmat_402_dl_controlword( cif, drive->node_id, ctrl, &drive->abort_code );
}

static enum canmat_status dl_control_mask( struct canmat_iface *cif, struct canmat_402_drive *drive,
                                           uint16_t mask_and, uint16_t mask_or ) {
    return dl_control( cif, drive, (drive->ctrl_word & mask_and) | mask_or );
}

enum canmat_status canmat_402_init( struct canmat_iface *cif, uint8_t id, struct canmat_402_drive *drive ) {

    canmat_status_t r;

    drive->node_id = id;

    // control word
    r = canmat_402_ul_controlword( cif, drive->node_id, &drive->ctrl_word, &drive->abort_code );
    if( CANMAT_OK != r ) return r;

    // status word
    r = canmat_402_ul_statusword( cif, drive->node_id, &drive->stat_word, &drive->abort_code );
    if( CANMAT_OK != r ) return r;

    // position
    r = canmat_402_ul_position_actual_value( cif, drive->node_id, &drive->actual_pos_raw, &drive->abort_code );
    if( CANMAT_OK != r ) return r;
    // velocity
    r = canmat_402_ul_velocity_actual_value( cif, drive->node_id, &drive->actual_vel_raw, &drive->abort_code );
    if( CANMAT_OK != r ) return r;

    // current
    /* { */
    /*     canmat_obj_t *obj = canmat_dict_search_name( &canmat_dict402, "current actual value" ); */
    /*     assert( obj && obj->data_type == CANMAT_DATA_TYPE_INTEGER32 ); */
    /*     r = canmat_obj_ul( cif, drive->node_id, obj, &val ); */
    /*     if( CANMAT_OK != r ) return r; */
    /*     drive->actual_cur_raw = val.i32; */
    /* } */
    return CANMAT_OK;
}

enum canmat_status canmat_402_start( struct canmat_iface *cif, struct canmat_402_drive *drive ) {
    // TODO: handle quick-stopped drives
    // TODO: check status word after each dl_control to make sure we really transitioned

    canmat_status_t r;

    // shutdown
    printf("shutdown\n");
    r = dl_control_mask( cif, drive,
                         CANMAT_402_CTRLCMD_MASK_AND_SHUTDOWN, CANMAT_402_CTRLCMD_MASK_OR_SHUTDOWN );
    if( CANMAT_OK != r ) return r;

    // reset error
    printf("reset err\n");
    r = dl_control_mask( cif, drive,
                         CANMAT_402_CTRLCMD_MASK_AND_RESET_FAULT, CANMAT_402_CTRLCMD_MASK_OR_RESET_FAULT );
    if( CANMAT_OK != r ) return r;

    // NMT start
    printf("nmt start\n");
    r = canmat_send_nmt( cif, drive->node_id, CANMAT_NMT_START_REMOTE );
    if( CANMAT_OK != r ) return r;

    sleep(1);
    // check status
    printf("check status\n");
    r = canmat_402_ul_statusword( cif, drive->node_id, &drive->stat_word, &drive->abort_code );
    if( CANMAT_OK != r ) return r;


    /* // shutdown */
    /* r = dl_control_mask( cif, drive, */
    /*                      CANMAT_402_CTRLCMD_MASK_AND_SHUTDOWN, CANMAT_402_CTRLCMD_MASK_OR_SHUTDOWN ); */
    /* if( CANMAT_OK != r ) return r; */

    /* // switch-on */
    /* r = dl_control_mask( cif, drive, */
    /*                      CANMAT_402_CTRLCMD_MASK_AND_SWITCH_ON, CANMAT_402_CTRLCMD_MASK_OR_SWITCH_ON ); */
    /* if( CANMAT_OK != r ) return r; */

    /* // enable */
    /* r = dl_control_mask( cif, drive, */
    /*                      CANMAT_402_CTRLCMD_MASK_AND_ENABLE_OP, CANMAT_402_CTRLCMD_MASK_OR_ENABLE_OP ); */
    /* if( CANMAT_OK != r ) return r; */

    return CANMAT_OK;
}

enum canmat_status canmat_402_stop( struct canmat_iface *cif, struct canmat_402_drive *drive ) {
    return dl_control_mask( cif, drive, CANMAT_402_CTRLCMD_MASK_AND_SHUTDOWN, CANMAT_402_CTRLCMD_MASK_OR_SHUTDOWN );
}

enum canmat_status canmat_402_probe_pdo(
    struct canmat_iface *cif, struct canmat_pdo_descriptor_table tab,
    const struct canmat_402_drive *drive )
{
    return CANMAT_OK;

}
