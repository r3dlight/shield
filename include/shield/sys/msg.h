#ifndef SYS_MSG_H_
#define SYS_MSG_H_
/*
 * This is a ligthway, high performance implementation of the POSIX message passing service.
 * There is, by now, no effective userspace queueing but only kernel queueing and IPC handling.
 * The goal here is to abstract the EwoK kernel IPC complexity into a user-friendly interface
 * without reducing their performances.
 *
 * Although, for a more intelligent message passing mechanism (i.e. effective userspace bus)
 * check the liberpes (RPC implementation) instead.
 */

#include <uapi.h>

/* messaging mode */
#define MSG_NOERROR    010000 /* truncate silently message if too long */
#define MSG_EXCEPT     020000 /* recv any msg excepting specifyed type */
#define MSG_COPY       040000 /* copy instead of removing queued msg (NOT SUPPORTED) */

#define IPC_CREAT	01000		/* Create key if key does not exist. */
#define IPC_EXCL	02000		/* Fail if key exists.  */
#define IPC_NOWAIT	04000		/* Do not wait, return with EAGAIN flag in case of error */
#define IPC_PRIVATE 0           /* key identifier to create new msgq */

/* generic IPC key_t for EwoK IPC (remote task name) */
typedef taskh_t key_t;

typedef long ssize_t;

/* Here, we hold a word-aligned structure in order to avoid
 * any unaligned access to mtex fields for u32 & u64 types.
 * The difference with the POSIX type is the mtext definition,
 * in order to simplify the content typing. Though, mtype handling
 * is the same:
 * msgsnd() must send typed messages (i.e. data containing a mtype field
 * in its first 4 bytes)
 * msgrcv() get back the mtext content directly, without the mtype field,
 * as the type is requested in argument.
 *
 * The message queue handles messages while not requested by msgrcv() in a
 * local cache.
 * Usual SysV message flags (see above) can be used to modify the API behavior
 * w. respect for the POSIX standard. */
struct msgbuf {
    long mtype;
    char mtext[1];
};

/*
 * As the following API tries to be rspectful of the POSIX API, return codes and arguments do
 * not used embedded oriented types (typically mbed_error_t and so on).
 */

/**
 * @fn Get back queue identifier from given key
 *
 * Typical usage:
 * initial get in init mode:
 *
 * msgget(taskh, IPC_CREAT | IPC_EXCL);
 *
 * other gets, in nominal mode:
 *
 * msgget(taskh, 0);
 *
 * @return msgqid, or -1, with errno set
 */
int msgget(key_t key, int msgflg);

/**
 * @fn Send a message to the given queue
 *
 * sending a message without blocking
 * msgsnd(qid, buf, msize, IPC_NOWAIT);
 */
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

/*
 * Receive a message from the given queue
 *
 * Receive a message in buf of any type but MAGIC_TYPE_X, that can be truncated
 * if its size is bigger than msgsz, without blocking (EAGAIN in case of nothing to read).
 *
 * msgrcv(qid, buf, msgsz, MAGIC_TYPE_X, IPC_NOWAIT| MSG_NOERROR|MSG_EXCEPT);
 *
 * Receive a message of type MAGIC_TYPE_Y only, that can't be truncated.
 * Blocks if no corresponding message upto cache full (in that case return with ENOMEM).
 * If a message of the corresponding type exists but is too big, return with E2BIG.
 *
 * msgrcv(qid, bug, msgsz, MAGIC_TYPE_Y, 0);
 *
 */
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
               int msgflg);


#endif/*!SYS_MSG_H_*/
