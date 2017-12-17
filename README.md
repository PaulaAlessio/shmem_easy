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

Three executables will be generated: `device`, `client`, `detach`
TODO. There's two executables that should be called from two 
terminal emulators. 


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

Still TODO

### Catchcall for various shared memory operations: `shmctl()`

Still TODO
### Attach a process to its address:  `shmat()`

Still TODO
### Detach a process from its address: `shmdt()` 
