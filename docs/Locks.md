# MmapFiles::Locks

Defines all lock types available which meet requirements.

## General Requirements

`T()`

`T(const T&) = delete`

`Returned<bool> Lock()`
> Undefined bheaviour if returned value is invalid.

`Returned<bool> Unlock()`
> Undefined bheaviour if returned value is invalid.

## Recursive Lock Requirements

Recursive locks are `SpinLock` and `MutexLock`.

If `Lock` is called multiple times by the same thread, a counter increases to indicate the number of locks the holding
thread has.  
If `Unlock` is called when the thread does not own the lock, there is no effect. There is no error or undefined behaviour. 

`T()`
> Default constructable

`T(const T&) = delete`
> Not copyable

`Returned<bool> Lock()`
> Attempt to acquire the lock, blocking until the lock is acquired. If the lock is locked, then requested by the holding thread, there is no blocking. Simply an immediate return true.

> Return true if instantly acquired lock, false otherwise.  

`Returned<bool> Unlock()`
> Attempt to free the lock. This call never blocks.  

> Return true if unlock was called from the holding thread. False otherwise.  

## Contention Moderated Lock Requirements

These locks give gaurentees on contention priority.

`T()`
> Default constructable

`T(const T&) = delete`
> Not copyable

`Returned<bool> Lock()`
> Attempt to acquire the lock, blocking until acquired.

> Return true when lock is acquired.  

`Returned<bool> Unlock()`
> Attempt to free the lock.

> Return true if unlock was called from holding thread.
