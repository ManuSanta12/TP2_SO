#include "keyboard.h"
#include "lib.h"
#include "memoryManager.h"
#include "sound.h"
#include "videoDriver.h"
#include <time.h>
#include <syscall_dispatcher.h>
#include <semaphore.h>
#include <scheduler.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

extern uint8_t hasInforeg;
extern const uint64_t inforeg[17];


extern Color RED;
extern Color WHITE;
extern Color BLACK;

int size = 0;

#define SYS_CALLS_QTY 20

static uint64_t sys_read(uint64_t fd, char *buff);
static uint64_t sys_write(uint64_t fd, char buffer);
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
static uint8_t sys_semOpen(char*name,int value);
static uint8_t sys_semPost(char* name);
static uint8_t sys_semWait(char* name,int pid);
static pid_t sys_newProcess(uint64_t rip, int argc, char *argv[]);

// los void los pongo sino me tira warning


// llena buff con el caracter leido del teclado
static uint64_t sys_read(uint64_t fd, char *buff)
{
  if (fd != 0)
  {
    return -1;
  }

  *buff = getCharFromKeyboard();
  return 0;
}

static uint64_t sys_write(uint64_t fd, char buffer)
{
  if (fd != 1)
  {
    return -1;
  }

  dv_print(buffer, WHITE, BLACK);
  return 1;
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

static uint8_t sys_semOpen(char*name,int value){
  return sem_init(name,value);
}

static uint8_t sys_semPost(char* name){
  return sem_post(name);
}

static uint8_t sys_semWait(char* name,int pid){
  return sem_wait(name, pid);
}

static pid_t sys_newProcess(uint64_t rip, int argc, char *argv[]){
  return new_process(rip, argc, argv);
}

static uint64_t (*syscall_handlers[])(uint64_t, uint64_t, uint64_t, uint64_t,
                                      uint64_t) = {
    (void *)sys_read,         (void *)sys_write,       (void *)sys_clear,
    (void *)sys_getHours,     (void *)sys_getMinutes,  (void *)sys_getSeconds,
    (void *)sys_getScrHeight, (void *)sys_getScrWidth, (void *)sys_fillRect,
    (void *)sys_wait,         (void *)sys_inforeg,     (void *)sys_pixelPlus,
    (void *)sys_pixelMinus,   (void *)sys_playSound,   (void *)sys_mute,
    (void *)sys_memInfo,      (void *)sys_memMalloc,   (void *)sys_memFree, 
    (void*)sys_semOpen,       (void*)sys_semPost,      (void*)sys_semWait,
    (void*)sys_newProcess};

// Devuelve la syscall correspondiente
//                                rdi           rsi           rdx rd10 r8 r9
uint64_t syscall_dispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10,
                         uint64_t r8, uint64_t rax) {
  if (rax < SYS_CALLS_QTY && syscall_handlers[rax] != 0) {
    return syscall_handlers[rax](rdi, rsi, rdx, r10, r8);
  }

  return 0;
}