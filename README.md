# Interprocess communication (IPC), shared memory (shm)

## Synopsis

It is the fastest way of communicating between two processes, since the data 
does not need to be copied between the server and the client. In our
applications, the server will simulate an I/O device that will communicate 
with a device support (the client). 



## Installation and code Example

To compile the code, download the repository and type, 

```
make
```

Three executables will be generated: `device`, `run_device`, `client`. Open two 
different consoles or tabs and run `./device <WAVE_TYPE> <PERIOD>` on one of 
them and `./client` on the other. 

### Virtual hardware:  `./device <WAVE_TYPE> <PERIOD>`
  Input parameters:
   * `WAVE_TYPE`: string specifying the signal type. Options: `SIN` for
   sinusoidal, `QUA` for quadratic, `TRI` for triangular. 
   * `PERIOD`: double specifying the period.  
 
 The virtual hardware is configured to generate the signal specified by the 
 input parameters, with a resolution of 10 bits and generating a measure 
 every `0.1s`. The process creating the signal in a shared memory 
 segment is sent to the background  (via `./run_device`) and a dialog 
faking a pseudoterminal is opening and is accepting the following  *commands*:
   * `stop`: stops the device.
   * `start`: starts the device if on `stop` mode.
   * `config`: lets the user reconfigure the device and restarts.
   * `exit`: shuts down the device and removes the shared memory segment. 

### Accessing shared memory:  `./client`
  This executable monitors the device by accessing the shared memory
  once per second. 

## Motivation and description

The trick is to synchronize the access to a given region among multiple 
processes. Semaphores, can be used to synchronize the memory access. The kernel 
maintains a structure with at least the following members for each memory 
segment. 

```
struct shmid_ds {
  struct ipc_perm shm_perm;   /* permissions structure */
  size_t          shm_segsz;  /* segment size in bytes */
  pid_t           shm_lpid;   /* pid of the last operator */
  pid_t           shm_cpid;   /* pid of creator */
  shmat_t         shm_nattch; /* number of current attaches */
  time_t          shm_atime;  /* last attach time */
  time_t          shm_dtime;  /* last detach time */
  time_t          shm_ctime;  /* last change time */
}; 
```

In Linux, system limits that affect shared memory are, 

Description | Values (Linux 3.2.0) 
----------------------------------------|---------------------
max size in bytes of shm segment        | 32768
min size in bytes of shm segment        | 1
max number of shm segments per process  | 4096
max number of shm segment system wide   | 4096 

The functions involved in the process are, 
- `ftok()`,
- `shmget()`,
- `shmctl()`
- `shmat()`,
- `shmdt()`,


### `ftok()` function 

Unix requires a key of type `key_t` (an integer)for irequesting shared 
memory segments. We generate the key using the function `ftok`, that has 
the following prototype, 

```
key_t  ftok(
            const char *path,      /* a path string       */
            int        id          /* an integer value    */
           );   
```

It takes a character string that identifies a path and an integer 
(usually a character) value, and generates an integer of type `key_t` based 
on the first argument with the value of id in the most significant position. 
On failure, `-1` is returned. 
Thus, the function returns always the same value, when called with the same 
arguments. Once obtained, it can be used where required.  

### Obtain shared memory identifier: `shmget()`

```
#include <sys/shm.h>
int shmget(key_t key, size_t size, int flag);
//                        Returns: shared memory ID if OK, -1 on error
```

When a new segment is created, the following members of the 
`shmid_ds` structure are initialized: 

- `ipc_perm` structure is initialized, mode member is set to the corresponding 
   permission bits or flag
- `shm_lpid`, `shm_nattch`, `shm_atime`, `shm_dtime` are set to 0.
- `shm_ctime` is set to the current time.
- `shm_segsz` is set to the size requested. 


### Catchcall for various shared memory operations: `shmctl()`

```
#include <sys/shm.h>
int shmctl(int shmid, int cmd, struct shmid_ds *buf);
//                        Returns: 0 if OK, -1 on error
```

`cmd` argument: 
- `IPC_STAT`: fetch the `shmid_ds` structure for this segment,
- ...
- `IPC_RMID`: removed the shared memory segment shared from the system. 

We use this function to removed the shared memory segment. From the console, 
we can list and remove shared memory segments with the following commands, 

```
$ ipcs -ma           # list shared memory segments
$ ipcrm -m <SHM_ID>  # removed a shared memory segment
```


### Attach a process to its address:  `shmat()`

```
#include <sys/shm.h>
void * shmat(int shmid, const void *addr, int flag);
//                        Returns: pointer to shared memory if OK, -1 on error
```

The address in the calling proess depends on the address argument and whether 
the `SHM_RND` bit is specified in the flag. Unless we plan to run the 
application on a single type of hardware, it is better not to specify the 
address. If the flag `SHM_RDONLY` is specified, then it is attached as read 
only. 

### Detach a process from its address: `shmdt()` 

```
#include <sys/shm.h>
void * shmdt(const void *addr);
//                        Returns: 0 if OK, -1 on error
```
When we are done with a shared memory segment, this function is called 
to detach it. 

