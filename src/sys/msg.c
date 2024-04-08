// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

/*
 * This is a ligthway, high performance implementation of the POSIX message passing service.
 * There is, by now, no effective userspace queueing but only kernel queueing and IPC handling.
 * The goal here is to abstract the EwoK kernel IPC complexity into a user-friendly interface
 * without reducing their performances.
 *
 * Although, for a more intelligent message passing mechanism (i.e. effective userspace bus)
 * check the liberpes (RPC implementation) instead.
 */

#include <stdint.h>
#include <shield/sys/msg.h>
#include <errno.h>
#include <string.h>
#include <uapi.h>

#include <shield/private/errno.h>
#include <shield/private/coreutils.h>

/**
 * the SVC exhcange area must hold:
 * - the exchange header set by the kernel
 * - the mtype field (long)
 * - the effective message content
 */
#define MAX_IPC_MSG_SIZE (CONFIG_SVC_EXCHANGE_AREA_LEN - sizeof(exchange_event_t) - sizeof(long))
/* max number of cached messages per source task */
#define CONFIG_STD_POSIX_SYSV_MSQ_DEPTH 1

extern size_t _s_svcexchange;

/* A message received by the kernel genuine type is char*. Though, its effective type here is struct msgbuf.
 * We use an union for clean cast. */
typedef union {
  struct msgbuf msgbuf;
  char          msg[MAX_IPC_MSG_SIZE];
} qsmsg_msgbuf_data_t;

typedef struct __attribute__((packed)) {
  qsmsg_msgbuf_data_t msg;
  uint8_t             msg_size;
  bool                set;
} qmsg_msgbuf_t;

typedef struct __attribute__((packed)) {
    uint32_t     msg_lspid; /**< for broadcasting recv queue, id of the last sender */
    uint32_t      msg_stime; /**< time of last snd event */
    uint32_t      msg_rtime; /**< time of last rcv event */
    qmsg_msgbuf_t msgbuf_v[CONFIG_STD_POSIX_SYSV_MSQ_DEPTH];
    uint8_t       msgbuf_ent;
    uint16_t      msg_perm; /**< queue permission, used for the broadcast recv queue case (send forbidden) */
    bool          set;
    key_t         key;
} qmsg_entry_t;

/*
 * list of all msg queues. If key == 0, the message queue is not initalised.
 *
 * This is size-impacting in SRAM and should be considered
 */
static qmsg_entry_t qmsg_vector[CONFIG_MAX_TASKS+1];

/*
 * Zeroify properly the qmsg_vector. This function is called at task early init state, before main,
 * by the zeroify_libc_globals() callback.
 */
static inline void msg_zeroify(void) {
    memset((void*)qmsg_vector, 0x0, (CONFIG_MAX_TASKS+1 * sizeof(qmsg_entry_t)));
}

/*
 * POSIX message passing API
 */
int msgget(key_t key, int msgflg)
{
    /* POSIX compliant */
    int errcode;
    Status ret;
    int tid = -1;
    /*
     * 1. Is there a previously cached qmsg id for the given key ?
     */
    /*@
      @ loop invariant \valid_read(qmsg_vector[0..CONFIG_MAXTASKS]);
      @ loop invariant 0 <= i <= CONFIG_MAXTASKS+1;
      @ loop assigns i;
      @ loop variant ((CONFIG_MAXTASKS+1) - errcode);
      */
    for (uint8_t i = 0; i <= CONFIG_MAX_TASKS; ++i) {
        if (unlikely(qmsg_vector[i].key == key && qmsg_vector[i].set == true)) {
            if (msgflg & IPC_EXCL) {
                /* fails if key exists */
                errcode = -1; /* POSIX Compliance */
                __shield_set_errno(EEXIST);
            } else {
                errcode = i;
            }
            goto err;
        }
    }

    /*
     * 2. No cache entry found. Try to get back the id from the kernel, and cache it
     */
    if (!(key == IPC_PRIVATE || (msgflg & IPC_CREAT))) {
        /* no cache entry found but cache entry creation **not** requested */
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(ENOENT);
        goto err;
    }
    /* get back first free vector cell */
    for (uint8_t i = 0; i < CONFIG_MAX_TASKS; ++i) {
        if (qmsg_vector[i].set == false) {
            tid = i;
            break;
        }
    }
    if (unlikely(tid == -1)) {
         errcode = -1; /* POSIX Compliance */
        __shield_set_errno(ENOMEM);
        goto err;
    }
    qmsg_vector[tid].key = key;
    qmsg_vector[tid].msg_perm = 0x666; /* unicast communication. Permission is handled by kernel */
    qmsg_vector[tid].msg_stime = 0;
    qmsg_vector[tid].msg_rtime = 0;
    qmsg_vector[tid].set = true;
    errcode = tid;
err:
    return errcode;
}

/*
 * Sending message msgp of size msgsz to 'msqid'.
 */
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
{
    int errcode = -1;
    Status ret;

    if (msgp == NULL) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EFAULT);
        goto err;
    }
    if (msqid < 0 || msqid > CONFIG_MAX_TASKS) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EINVAL);
        goto err;
    }
    if (qmsg_vector[msqid].set == false) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EINVAL);
        goto err;
    }
    if (msgsz > MAX_IPC_MSG_SIZE) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(E2BIG);
        goto err;
    }
    if (qmsg_vector[msqid].msg_perm == 0x444) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EPERM);
        goto err;
    }
    /* sending size+mtype field (long) */
    copy_from_user((const uint8_t*)msgp, msgsz+sizeof(long));
    ret = sys_send_ipc(qmsg_vector[msqid].key, msgsz+sizeof(long));
    /* clear queue once emitted */
    memset(&qmsg_vector[msqid], 0x0, sizeof(qmsg_entry_t));

    switch (ret) {
        case STATUS_INVALID:
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EINVAL);
            goto err;
            break;
        case STATUS_DENIED:
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EACCES);
            goto err;
            break;
        case STATUS_BUSY:
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EAGAIN);
            goto err;
        case STATUS_OK:
            break;
        default:
            /* abnormal other return code, should not happen */
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EINVAL);
            goto err;
            break;
    }
    /* default on success */
    errcode = 0;
err:
    return errcode;
}

/*
 * msgrcv return in msgp the content of mtext[]. The mtype field is ignored
 * as used as a discriminant for message selection purpose.
 * Although, as msgsnd()/msgrcv() API is **not** a kernel API, the received
 * buffer content the overall data (including mtype), as the kernel as no
 * idea of the msgbuf structure.
 *
 * As a consequence, we can't recv the content directly in msgp, as msgp
 * (and msgz) may be too short to recv the buffer including the mtype.
 *
 * A possible way, here is to:
 * - if the local msgbuf vector is full and mtype does not match any of the
 *   stored message, returns ENOMEM
 * - check the local msgbuf vector if there is a pending message with the
 *   corresponding mtype. If yes, returns it.
 * - If not, get back an IPC from the kernel
 *   gave back and:
 *      * if the mtype flags match the msgtyp argument, copy back the mtext content
 *        to msgp
 *      * if the mtype flag does not match the msgtyp argument, store the overall
 *        msgbuf content localy to a msgbuf vector, and:
 *          -> if IPC_NOWAIT is not set, try agin (blocking mode)
 *          -> in IPC_NOWAIT is set, return EGAIN (using waitforevent timeout=1).
 */
ssize_t msgrcv(int msqid,
               void *msgp,
               size_t msgsz,
               long msgtyp,
               int msgflg)
{
    ssize_t errcode = -1;
    Status ret;
    int tid = msqid;
    uint16_t rcv_size;
    uint8_t i = 0;
    uint8_t free_cell = 0;
    int32_t timeout = 0;

    /* sanitation */
    if (msgp == NULL) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EFAULT);
        goto err;
    }
    if (msqid < 0 || msqid > CONFIG_MAX_TASKS) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EINVAL);
        goto err;
    }
    if (qmsg_vector[msqid].set == false) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EINVAL);
        goto err;
    }
    if (qmsg_vector[msqid].msg_perm == 0x444) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EPERM);
        goto err;
    }

tryagain:

    /* check local previously stored messages */
    for (i = 0; i < CONFIG_STD_POSIX_SYSV_MSQ_DEPTH; ++i) {
        if (qmsg_vector[msqid].msgbuf_v[i].set == true) {
            /* msgtyp == 0, the first read message is transmitted */
            if (msgtyp == 0) {
                if (qmsg_vector[msqid].msgbuf_v[i].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
                    /* truncate not allowed!*/
                    errcode = -1;
                    __shield_set_errno(E2BIG);
                    goto err;
                }
                goto handle_cached_msg;
            /* no EXCEPT mode, try to match msgtyp */
            } else if ((msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msgbuf_v[i].msg.msgbuf.mtype != msgtyp)) {
                /* found a waiting msg for except mode */
                if (qmsg_vector[msqid].msgbuf_v[i].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
                    /* truncate not allowed!*/
                    errcode = -1;
                    __shield_set_errno(E2BIG);
                    goto err;
                }
                goto handle_cached_msg;

            } else if (!(msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msgbuf_v[i].msg.msgbuf.mtype == msgtyp)) {
                /* found a waiting msg with corresponding type */
                if (qmsg_vector[msqid].msgbuf_v[i].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
                    /* truncate not allowed!*/
                    errcode = -1;
                    __shield_set_errno(E2BIG);
                    goto err;
                }
                goto handle_cached_msg;
            }
        } else {
            free_cell = i;
        }
    }
    /* no cached message found ? if msgbuf_vector is full, NOMEM */
    if (unlikely(qmsg_vector[msqid].msgbuf_ent == CONFIG_STD_POSIX_SYSV_MSQ_DEPTH)) {
        errcode = -1;
        __shield_set_errno(ENOMEM);
        goto err;
    }
    /* here, free_cell should have been set at least one time, or the execution derivated to
     * handle_cached_msg. we can use free_cell as cell id for next sys_ipc() call  */

    /* by default, we are able to get back upto struct msgbuf content size in cache */
    rcv_size = sizeof(struct msgbuf);

    /* get back message content from kernel */
    if (msgflg & IPC_NOWAIT) {
        /* sync wait */
        timeout = -1;
    }
    ret = sys_wait_for_event(EVENT_TYPE_IPC, timeout);
    switch (ret) {
        case STATUS_INVALID:
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EINVAL);
            goto err;
            break;
        case STATUS_DENIED:
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EACCES);
            goto err;
            break;
        case STATUS_AGAIN:
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EAGAIN);
            goto err;
        case STATUS_OK:
            break;
        default:
            /* abnormal other return code, should not happen */
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EINVAL);
            goto err;
            break;
    }
    exchange_event_t* rcv_buf = (exchange_event_t*)&_s_svcexchange;
    memcpy(&(qmsg_vector[msqid].msgbuf_v[free_cell].msg.msg[0]), &rcv_buf->data[0], rcv_buf->length);
    /* set recv msg size, removing the mtype field size */
    qmsg_vector[msqid].msgbuf_v[free_cell].msg_size = rcv_buf->length - sizeof(long);
    qmsg_vector[msqid].msgbuf_v[free_cell].set = true;
    qmsg_vector[msqid].msgbuf_ent++;

    /* Now that the buffer received. Check its content. Here, like for cache check, we must check
     * mtype according to msgflg */
    if ((msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msgbuf_v[free_cell].msg.msgbuf.mtype != msgtyp)) {
        /* if this message matches, check for its size, according to MSG_NOERROR flag */
        if (qmsg_vector[msqid].msgbuf_v[free_cell].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
            /* truncate not allowed!*/
            qmsg_vector[msqid].msgbuf_v[free_cell].set = true;
            errcode = -1;
            __shield_set_errno(E2BIG);
            goto err;
        }
        i = free_cell;
        goto handle_cached_msg;


    } else if (!(msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msgbuf_v[free_cell].msg.msgbuf.mtype == msgtyp)) {
        /* if this message matches, check for its size, according to MSG_NOERROR flag */
        if (qmsg_vector[msqid].msgbuf_v[free_cell].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
            /* truncate not allowed!*/
            qmsg_vector[msqid].msgbuf_v[free_cell].set = true;
            errcode = -1;
            __shield_set_errno(E2BIG);
            goto err;
        }
        i = free_cell;
        goto handle_cached_msg;
    }

    /* is the queue full now ? */

    /* In the case of msgflag without IPC_NOWAIT, if the message of type mtype is not found
     * (i.e. we arrived here), we try again, until the queue is full. */
    if (msgflg & IPC_NOWAIT) {
        errcode = -1;
        __shield_set_errno(EAGAIN);
        goto err;
    }
    goto tryagain;

    /* default on success */
err:
    return errcode;

    /* handle found cached message or just received message */
handle_cached_msg:
    rcv_size = (msgsz < qmsg_vector[msqid].msgbuf_v[i].msg_size) ? msgsz : qmsg_vector[msqid].msgbuf_v[i].msg_size;
    memcpy(msgp, &(qmsg_vector[msqid].msgbuf_v[i].msg.msgbuf), rcv_size + sizeof(long));
    qmsg_vector[msqid].msgbuf_ent--;
    qmsg_vector[msqid].msgbuf_v[i].set = false;
    errcode = rcv_size;
    return errcode;
}
