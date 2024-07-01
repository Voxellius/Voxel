# `threads` standard library
The `threads` standard library provides an interface to perform multithreading in Voxel. This library allows a program to execute multiple functions concurrently.

## Example
This example starts two threads to call the `repeat` function concurrently with a number of arguments. The function is also called in the main thread. The main thread waits for each thread to finish before closing them.

```voxel
import io;
import threads;

function repeat(value, times) {
    for (var i = 0; i < times; i++) {
        io.println(value);
    }

    io.println("My thread ID: " + threads.getOwnThread().id);
}

var thread1 = new threads.Thread(repeat, ["Thread 1", 5]);
var thread2 = new threads.Thread(repeat, ["Thread 2", 5]);

thread1.start();
thread2.start();

repeat("Main thread", 5);

thread1.waitUntilFinished();

thread1.close();
io.println("Closed thread 1");

thread2.waitUntilFinished();

thread2.close();
io.println("Closed thread 2");
```

It is important that threads are closed with the `close` method after they have been used to free up memory.

## Reference

### `enum ThreadState`
States which a `Thread` can be in.

#### `NOT_STARTED`
The thread has not started execution yet.

#### `RUNNING`
The thread is currently running.

#### `FINISHED`
The thread has finished execution and a return value may be available.

#### `CLOSED`
The thread has been closed and is no longer able to run.

### `enum ThreadError`

#### `THREAD_HAS_CLOSED`
The action cannot be performed on the thread as it has been closed.

#### `CANNOT_ENTER_STATE`
The thread cannot enter the requested state as it is either already in that state or has finished.

### `class Thread(callFunction: Function, callArgs?: [*])`
Create a new thread to execute a function (`callFunction`) concurrently with an optional set of arguments to pass to the function (`callArgs`). Instantiating this class does not begin executing the function right away — the `start` method must be called to begin execution. To prevent memory leaks, you must call the `close` function after you have done with a `Thread` (this is not done automatically since access to the return value may be required after execution finishes).

#### `close()`
Stop running the thread and destroy the execution context.

#### `start()`
Start running the thread or resume it if it was paused.

#### `pause()`
Pause the execution of the thread. The thread can resume execution by calling the `start` method again.

#### `id: Number`
The ID of the thread.

#### `isRunning(): Boolean`
Return `true` if the thread is running, or `false` otherwise. This will return `false` in cases where the thread is paused or has finished execution.

#### `hasFinished(): Boolean`
Return `true` if the thread has finished execution, or `false` otherwise. This will return `false` if the thread has not began execution.

#### `waitUntilFinished()`
Wait until the thread has finished execution. This method will also start the thread if it has not been started yet or resume execution if it is paused.

#### `state: ThreadState`
The current state of the thread.

### `class MainThread extends Thread`
This class represents the main thread which is created by the Voxel runtime when a program is run.

To obtain an instance of this class, call `getOwnThread`.

### `getOwnThread(): Thread`
Get an instance of the `Thread` class for the thread in which the function has been called. A `MainThread` will be returned if called in the main thread.