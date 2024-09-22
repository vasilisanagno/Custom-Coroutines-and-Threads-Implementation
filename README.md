# Custom Coroutines and Threads Implementation

This C application demonstrates the development of a custom coroutine system and the implementation of thread management using custom coroutines. The application does not rely on external libraries such as `pthread` and instead creates its own coroutines for concurrent programming, followed by building a thread management system on top of these coroutines.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
  - [Coroutines](#coroutines)
  - [Custom Thread Management Using Coroutines](#custom-thread-management-using-coroutines)
  - [Semaphores for Synchronization](#semaphores-for-synchronization)
- [Usage](#usage)
  - [Compiling the Program](#compiling-the-program)
  - [Running the Program](#running-the-program)

## Introduction

This project showcases a self-developed coroutine system in C, designed without external libraries like pthread. By crafting coroutines from the ground up, it introduces a custom solution for concurrent programming. The coroutines then serve as the foundation for building a custom thread management system, enabling precise control over thread switching and synchronization.

## Features

### Coroutines

- **Custom Coroutine Creation and Switching**: Implements a basic coroutine system using C functions like `getcontext()`, `makecontext()`, `setcontext()`, and `swapcontext()` to allow switching between different coroutines manually.  
  These functions are defined in the `coroutines.c` and `coroutines.h` files.

- **Manual Context Switching**: Coroutines switch context explicitly when a certain condition is met, such as when a buffer is full or empty during a file read-write operation.

### Custom Thread Management Using Coroutines

- **Thread Creation Using Coroutines**: Threads are built on top of the custom coroutine system, providing automatic switching using a custom-built scheduler with round-robin scheduling.  
  The thread implementation is defined in `threads.c` and `threads.h`.

- **Thread Functions**:
  - `mythreads_create()`: Creates a thread using coroutines.
  - `mythreads_yield()`: Voluntary context switch between threads.
  - `mythreads_join()`: Waits for a thread to finish.
  - `mythreads_destroy()`: Destroys a thread after it completes its execution.

### Semaphores for Synchronization

- **Binary Semaphores**: Implements semaphores for thread synchronization, preventing race conditions and managing access to shared resources.  
  The semaphore functions are defined within the same `threads.c` file.
  - `mythreads_sem_create()`: Creates and initializes a semaphore.
  - `mythreads_sem_down()`: Decreases the semaphore value (blocking if necessary).
  - `mythreads_sem_up()`: Increases the semaphore value (releases waiting threads).
  - `mythreads_sem_destroy()`: Destroys the semaphore when no longer needed.

## Usage

### Compiling the Program

To compile the program, use the provided `Makefile`. Simply navigate to the directory containing the `Makefile` and run:

```bash
make all
```

### Running the Program

```bash
./test
```
