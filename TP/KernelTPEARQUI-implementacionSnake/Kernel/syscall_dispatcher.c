#include "keyboard.h"
#include "lib.h"
#include "memoryManager.h"
#include "sound.h"
#include "videoDriver.h"
#include <time.h>
#include <syscall_dispatcher.h>
#include <semaphore.h>
#include <scheduler.h>
#include "timeRtc.h"
#include <pipe.h>
#include <queue.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

extern uint8_t hasInforeg;
extern const uint64_t inforeg[17];


extern Color RED;
extern Color WHITE;
extern Color BLACK;

int size = 0;

#define SYS_CALLS_QTY 43

static uint64_t sys_read(uint64_t fd, char *buff, uint64_t count);
static uint64_t sys_write(uint64_t fd, char buffer, uint64_t count);
static uint64_t sys_getHours();
static uint64_t sys_getMinutes();
static uint64_t sys_getSeconds();
static uint64_t sys_getScrHeight();
static uint64_t sys_getScrWidth();
static void sys_fillRect(int x, int y, int x2, int y2, Color *color);
static void sys_wait(int ms);
static uint64_t sys_inforeg(uint64_t registers[17]);
static uint64_t sys_pixelPlus();
static uint64_t sys_pixelMinus();
static uint64_t sys_playSound(uint32_t frequnce);
static uint64_t sys_mute();

static MemoryInfo *sys_memInfo();
static void *sys_memMalloc(uint64_t size);
static void sys_memFree(uint64_t ap);
static uint8_t sys_semInit(char*name,int value);
static uint8_t sys_semPost(char* name);
static uint8_t sys_semWait(char* name,int pid);
static uint8_t sys_semClose(char* name);
static pid_t sys_newProcess(void* rip, int bg, char*argv[],int argc);
static uint64_t sys_getPid();
static uint64_t sys_sleepTime(int sec);
// los void los pongo sino me tira warning
static int sys_nice(pid_t pid, int new_priority);
static int sys_pipe(int pipefd[2]);
static int sys_dup2(int fd1, int fd2);
static int sys_open(int fd);
static int sys_close(int fd);
static processInfo *sys_ps();
static int sys_changeProcessStatus(pid_t pid);
static pid_t sys_getCurrentPid();
static priority_t sys_getPriority(pid_t pid);

static pid_t sys_exec(void* program, int bg, char*argv[],int argc);
static void sys_exit(int return_value, char autokill);

static pid_t sys_waitpid(pid_t pid);
static int sys_kill(pid_t pid);
static int sys_block(pid_t pid);
static int sys_unblock(pid_t pid);

// llena buff con el caracter leido del teclado
static uint64_t sys_read(uint64_t fd, char *buff, uint64_t count)
{

  PCB *pcb = getProcess(getCurrentPid());
  if (pcb->lastFd <= fd)
    return 0;

  if (pcb->fileDescriptors[fd].mode == OPEN)
  {
    *buff = getCharFromKeyboard();
    return 1;
  }
  if (pcb->fileDescriptors[PIPEOUT].mode == OPEN)
  {
    return pipeReadData(pcb->pipe, buff, count);
  }
  return 0;
}

static uint64_t sys_write(uint64_t fd, char buffer, uint64_t count)
{
  PCB *pcb = getProcess(getCurrentPid());
  if (pcb->lastFd < fd)
    return;
  if (pcb->fileDescriptors[fd].mode == OPEN)
  {
    uint64_t i = 0;
    while (i < count)
    {
      switch (fd)
      {
      case STDOUT:
        dv_print(buffer, WHITE, BLACK);
        break;
      case STDERR:
        dv_print(buffer, RED, BLACK); // Asumimos que ERROR_FORMAT usa rojo
        break;
      }
      i++;
    }
  }
  else if (pcb->fileDescriptors[PIPEIN].mode == OPEN)
  {
    pipeWriteData(pcb->pipe, buffer, count);
  }
  return 1; // Retorna la cantidad de bytes escritos, que siempre es 1
  //   dv_print(buffer, WHITE, BLACK);
  // return 1;
}

static uint64_t sys_clear()
{
  dv_clear(BLACK);
  return 1;
}

static uint64_t sys_getHours() { return getHours(); }

static uint64_t sys_getMinutes() { return getMinutes(); }

static uint64_t sys_getSeconds() { return getSeconds(); }

static uint64_t sys_getScrHeight() { return dv_getHeight(); }

static uint64_t sys_getScrWidth() { return dv_getWidth(); }

static void sys_fillRect(int x, int y, int x2, int y2, Color *color)
{
  dv_fillRect(x, y, x2, y2, *color);
}

static void sys_wait(int ms)
{
  if (ms > 0)
  {
    int start_ms = ms_elapsed();
    do
    {
      _hlt();
    } while (ms_elapsed() - start_ms < ms);
  }
}

static uint64_t sys_inforeg(uint64_t registers[17])
{

  if (hasInforeg)
  {
    for (uint8_t i = 0; i < 17; i++)
    {
      registers[i] = inforeg[i];
    }
  }
  return hasInforeg;
}

static uint64_t sys_pixelPlus()
{
  increasePixelScale();
  sys_clear();
  return 1;
}

static uint64_t sys_pixelMinus()
{
  decreasePixelScale();
  sys_clear();
  return 1;
}

static uint64_t sys_playSound(uint32_t frequnce)
{
  startSound(frequnce);
  return 1;
}

static uint64_t sys_mute()
{
  stopSound();
  return 1;
}

static MemoryInfo *sys_memInfo() { return mem_info(); }

static void *sys_memMalloc(uint64_t size)
{
  return memory_manager_malloc(size);
}

static void sys_memFree(uint64_t ap) { free_memory_manager((void *)ap); }

static uint8_t sys_semInit(char*name,int value){
  return sem_init(name,value);
}

static uint8_t sys_semPost(char* name){
  return sem_post(name);
}

static uint8_t sys_semWait(char* name,int pid){
  return sem_wait(name, pid);
}

static uint8_t sys_semClose(char* name){
  return sem_close(name);
}

static pid_t sys_newProcess(void* rip, int bg, char*argv[],int argc){
  return new_process(rip, bg, argv,argc);
}

static uint64_t sys_getPid(){
  return getCurrentPid();
}

static uint64_t sys_sleepTime(int sec){
  sleep_time(sec);
}

static int sys_nice(pid_t pid, int newPriority)
{
  if (pid < 0)
  {
    return -1;
  }

  return changePriority(pid, newPriority);
}

static int sys_pipe(int pipefd[2])
{
  PCB *pcb = getProcess(getCurrentPid());
  pcb->fileDescriptors[PIPEIN].mode = OPEN;
  pcb->fileDescriptors[PIPEOUT].mode = OPEN;
  pcb->pipe = pipeOpen();
  pipefd[0] = PIPEIN;
  pipefd[1] = PIPEOUT;
  return 0;
}

static int sys_dup2(int fd1, int fd2)
{
  PCB *pcb = getProcess(getCurrentPid());
  if (fd1 > pcb->lastFd || fd2 > pcb->lastFd || pcb->fileDescriptors[fd2].mode == CLOSED)
    return 0;
  pcb->fileDescriptors[fd1] = pcb->fileDescriptors[fd2];
  return 1;
}
static int sys_open(int fd)
{
  PCB *pcb = getProcess(getCurrentPid());
  if (pcb->lastFd < fd)
    return 0;
  pcb->fileDescriptors[fd].mode = OPEN;
  return 1;
}
static int sys_close(int fd)
{
  PCB *pcb = getProcess(getCurrentPid());
  if (pcb->lastFd < fd)
    return 0;
  pcb->fileDescriptors[fd].mode = CLOSED;
  return 1;
}

static processInfo *sys_ps()
{
  return getProcessesInfo();
}

static priority_t sys_getPriority(int pid){
  return get_priority(pid);
}

// Returns READY if unblocked, BLOCKED if blocked, -1 if failed
static int sys_changeProcessStatus(pid_t pid)
{
  PCB *process = getProcess(pid);
  if (process == NULL)
  {
    return -1;
  }
  if (process->status == READY)
  {
    sys_block(pid);
    return BLOCKED;
  }
  else
  {
    sys_unblock(pid);
    return READY;
  }
}

static pid_t sys_getCurrentPid()
{
  return getCurrentPid();
}

static pid_t sys_exec(void* program, int bg, char *argv[], int argc)
{
  return new_process(program, bg, argv, argc);
}

static void sys_exit(int return_value, char autokill)
{
  PCB *pcb = getProcess(getCurrentPid());
  unsigned int lastFd = pcb->lastFd;

  for (int i = 0; i < lastFd; i++)
  {
    sys_close(i);
  }

  killProcess(return_value, autokill);
}

static pid_t sys_waitpid(pid_t pid)
{
  PCB *processPcb = getProcess(pid);
  if (processPcb == NULL)
  {
    return -1;
  }

  pid_t currentPid = getCurrentPid();
  enqueuePid(processPcb->blockedQueue, currentPid);
  blockProcess(currentPid);

  return pid;
}


static int sys_kill(pid_t pid)
{
  return kill_by_pid(pid);
}

static int sys_block(pid_t pid)
{
  if (pid <= 0)
  {
    return -1;
  }
  return blockProcess(pid);
}

static int sys_unblock(pid_t pid)
{
  if (pid <= 0)
  {
    return -1;
  }
  return unblockProcess(pid);
}

static uint64_t (*syscall_handlers[])(uint64_t, uint64_t, uint64_t, uint64_t,
                                      uint64_t) = {
    (void *)sys_read,         (void *)sys_write,       (void *)sys_clear,
    (void *)sys_getHours,     (void *)sys_getMinutes,  (void *)sys_getSeconds,
    (void *)sys_getScrHeight, (void *)sys_getScrWidth, (void *)sys_fillRect,
    (void *)sys_wait,         (void *)sys_inforeg,     (void *)sys_pixelPlus,
    (void *)sys_pixelMinus,   (void *)sys_playSound,   (void *)sys_mute,
    (void *)sys_memInfo,      (void *)sys_memMalloc,   (void *)sys_memFree, 
    (void*)sys_semInit,       (void*)sys_semPost,      (void*)sys_semWait,
    (void*)sys_newProcess,    (void*)sys_getPid,       (void*)sys_semClose, 
    (void *)sys_sleepTime,    (void *)sys_nice,        (void *)sys_pipe,         
    (void *)sys_dup2,         (void *)sys_open,        (void *)sys_close,        
    (void *)sys_ps,           (void *)sys_changeProcessStatus,
    (void *)sys_getCurrentPid,(void *)sys_exec,        (void *)sys_exit, 
    (void *)sys_waitpid,      (void *)sys_kill,        (void *)sys_block, 
    (void *)sys_unblock ,     (void*)sys_getPriority,  };

// Devuelve la syscall correspondiente
//                                rdi           rsi           rdx rd10 r8 r9
uint64_t syscall_dispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10,
                         uint64_t r8, uint64_t rax) {
  if (rax < SYS_CALLS_QTY && syscall_handlers[rax] != 0) {
    return syscall_handlers[rax](rdi, rsi, rdx, r10, r8);
  }

  return 0;
}