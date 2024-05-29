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
#define CONFIG_MAX_SYSV_MSG_LEN 1024

/* A message received by the kernel genuine type is char*. Though, its effective type here is struct msgbuf.
 * We use an union for clean cast. */
typedef union {
  struct msgbuf msgbuf;
  char          msg[CONFIG_MAX_SYSV_MSG_LEN];
} qsmsg_msgbuf_data_t;

typedef struct __attribute__((packed)) {

  bool                set;
} qmsg_msgbuf_t;

typedef struct __attribute__((packed)) {
    uint32_t      msg_lspid; /**< for broadcasting recv queue, id of the last sender */
    uint32_t      msg_stime; /**< time of last snd event */
    uint32_t      msg_rtime; /**< time of last rcv event */
    qsmsg_msgbuf_data_t msg; /**< message content (for caching) */
    size_t        msg_size;  /**< current message size, upto msgsz */
    uint16_t      msg_perm; /**< queue permission, used for the broadcast recv queue case (send forbidden) */
    bool          set;
    key_t         key;
} qmsg_entry_t;

/*
 * list of all msg queues. If key == 0, the message queue is not initalised.
 *
 * This is size-impacting in SRAM and should be considered
 */
static qmsg_entry_t qmsg_vector[CONFIG_MAX_TASKS];

/*
 * Zeroify properly the qmsg_vector. This function is called at task early init state, before main,
 * by the zeroify_libc_globals() callback.
 */
static inline void msg_zeroify(void) {
    memset((void*)qmsg_vector, 0x0, (CONFIG_MAX_TASKS * sizeof(qmsg_entry_t)));
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
    qmsg_vector[tid].msg_size = 0;
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
    const uint8_t *__msgp = msgp;
    size_t chunk_offset = 0;
    size_t chunk_size;
    /* total number of bytes to emit */
    size_t __msgsz = msgsz + sizeof(long);

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
    /* sending size+mtype field (long), possibly with multiple IPCs */
    chunk_size = (__msgsz % MAX_IPC_MSG_SIZE);
    do {
        copy_from_user(__msgp, chunk_size);
        ret = sys_send_ipc(qmsg_vector[msqid].key, chunk_size);
        __msgp += chunk_size;
        __msgsz -= chunk_size;
        if (__msgsz < chunk_size) {
            /* support for residual */
            chunk_size = __msgsz;
        }
    } while(__msgsz > 0);
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
    uint8_t i = 0;
    uint8_t free_cell = 0;
    int32_t timeout = 0;
    size_t __msgsz = msgsz + sizeof(long);
    uint8_t *__msgp = msgp;
    size_t chunk_offset;

    /* sanitation */
    if (msgsz > CONFIG_MAX_SYSV_MSG_LEN) {
        if (!(msgflg & MSG_NOERROR)) {
            /* msgsz bigger than max transmittable content and truncate (NOERROR)
             * not allowed */
            errcode = -1; /* POSIX Compliance */
            __shield_set_errno(EINVAL);
            goto err;
        }
    }
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
    if (qmsg_vector[msqid].msg_perm == 0x444) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EPERM);
        goto err;
    }
    if (msgsz > qmsg_vector[msqid].msg_perm) {
        errcode = -1; /* POSIX Compliance */
        __shield_set_errno(EPERM);
        goto err;
    }

tryagain:

    /* check local previously stored messages for current msgqid */
    if (qmsg_vector[msqid].set == true) {
        /* msgtyp == 0, the first read message is transmitted */
        if (msgtyp == 0) {
            if (qmsg_vector[msqid].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
                /* truncate not allowed!*/
                errcode = -1;
                __shield_set_errno(E2BIG);
                goto err;
            }
            goto handle_cached_msg;
        /* no EXCEPT mode, try to match msgtyp */
        } else if ((msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msg.msgbuf.mtype != msgtyp)) {
            /* found a waiting msg for except mode */
            if (qmsg_vector[msqid].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
                /* truncate not allowed!*/
                errcode = -1;
                __shield_set_errno(E2BIG);
                goto err;
            }
            goto handle_cached_msg;
        } else if (!(msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msg.msgbuf.mtype == msgtyp)) {
            /* found a waiting msg with corresponding type */
            if (qmsg_vector[msqid].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
                /* truncate not allowed!*/
                errcode = -1;
                __shield_set_errno(E2BIG);
                goto err;
            }
            goto handle_cached_msg;
        } else if (qmsg_vector[msqid].msg_size < msgsz) {
            /* at least one chunk of msgqid was stored before, yet not reaching
             * msgsz. Need to read other chunks to complete */
        }
    }
    /* here, current msqid is free, or not yet fully loaded,
     * get back msgsz from kernel, checking msgqid using the key (source id) */

    /* get back message content from kernel */
    if (msgflg & IPC_NOWAIT) {
        /* sync wait */
        timeout = -1;
    }
    /* as msgsz may be bigger thant an IPC size, we must support chunks */
    chunk_offset = 0;
    qmsg_vector[msqid].msg_size = 0;

    do {
        exchange_event_t* rcv_buf;
        qmsg_entry_t *entry = &qmsg_vector[msqid];

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
        rcv_buf = &_s_svcexchange;
        /* check than received message source (taskh_t) matches current key */
        if (rcv_buf->source == entry->key) {
            /* received message matches current msgqid */
            if (entry->msg_size + rcv_buf->length > CONFIG_MAX_SYSV_MSG_LEN) {
                /* overflow here */
                errcode = -1; /* POSIX Compliance */
                __shield_set_errno(EBADF);
                goto err;
            }
            memcpy(&entry->msg.msg[chunk_offset], &rcv_buf->data[0], rcv_buf->length);
            chunk_offset += rcv_buf->length;
            qmsg_vector[msqid].msg_size += rcv_buf->length;
        } else {
            /* else, rcv msg is not from the same key (another one has emitted) */
            /* push current chunk to other queue that do match the source */
            for (uint8_t queue = 0; queue < CONFIG_MAX_TASKS; ++queue) {
                if (rcv_buf->source == qmsg_vector[msqid].key) {
                    entry = &qmsg_vector[msqid];
                    if (entry->msg_size + rcv_buf->length > CONFIG_MAX_SYSV_MSG_LEN) {
                        /* overflow here */
                        errcode = -1; /* POSIX Compliance */
                        __shield_set_errno(EBADF);
                        goto err;
                    }
                    memcpy(&entry->msg.msg[entry->msg_size], &rcv_buf->data[0], rcv_buf->length);
                    if (chunk_offset)
                    entry->msg_size += rcv_buf->length;
                    entry->set = true;
                }
                /** WARN: if an IPC from a source from which a msgget() has never been
                 * made is received, the IPC content is discarded */
            }
        }
    } while (qmsg_vector[msqid].msg_size <= msgsz);

    /* Now that the buffer received. Check its content. Here, like for cache check, we must check
     * mtype according to msgflg */
    if ((msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msg.msgbuf.mtype != msgtyp)) {
        /* if this message matches, check for its size, according to MSG_NOERROR flag */
        if (qmsg_vector[msqid].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
            /* truncate not allowed! keeping locally, let caller come back with increased msgsz */
            qmsg_vector[msqid].set = true;
            errcode = -1;
            __shield_set_errno(E2BIG);
            goto err;
        }
        goto handle_cached_msg;


    } else if (!(msgflg & MSG_EXCEPT) && (qmsg_vector[msqid].msg.msgbuf.mtype == msgtyp)) {
        /* if this message matches, check for its size, according to MSG_NOERROR flag */
        if (qmsg_vector[msqid].msg_size > msgsz && !(msgflg & MSG_NOERROR)) {
            /* truncate not allowed!*/
            qmsg_vector[msqid].set = true;
            errcode = -1;
            __shield_set_errno(E2BIG);
            goto err;
        }
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
    memcpy(msgp, &(qmsg_vector[msqid].msg.msgbuf), qmsg_vector[msqid].msg_size);
    qmsg_vector[msqid].set = false;
    errcode = qmsg_vector[msqid].msg_size;
    return errcode;
}
