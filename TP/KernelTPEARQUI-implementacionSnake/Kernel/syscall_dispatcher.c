#include "keyboard.h"
#include "lib.h"
#include "memoryManager.h"
#include "sound.h"
#include "videoDriver.h"
#include <time.h>
#include <syscall_dispatcher.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

extern uint8_t hasInforeg;
extern const uint64_t inforeg[17];


extern Color RED;
extern Color WHITE;
extern Color BLACK;

int size = 0;

#define SYS_CALLS_QTY 16

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

uint64_t syscall_dispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rax, uint64_t *registers)
{
  switch (rax)
  {
  case 0:
    return sys_read(rdi, (char *)rsi);
    break;
  case 1:
    return sys_write(rdi, (char)rsi);
    break;
  case 2:
    return sys_getHours();
    break;
  case 3:
    return sys_getMinutes();
    break;
  case 4:
    return sys_getSeconds();
    break;
  case 5:
    return sys_getScrHeight();
    break;
  case 6:
    return sys_getScrWidth();
    break;
  case 7:
    sys_fillRect(rdi, rsi, rdx, registers[0], (Color *)registers[1]);
    break;
  case 8:
    sys_wait(rdi);
    break;
  case 9:
    return sys_inforeg((uint64_t *)rdi);
    break;
  case 10:
    return sys_pixelPlus();
    break;
  case 11:
    return sys_pixelMinus();
    break;
  case 12:
    return sys_playSound((uint32_t)rdi);
    break;
  case 13:
    return sys_mute();
    break;
  case 14:
    return (uint64_t)sys_memInfo();
    break;
  case 15:
    return (uint64_t)sys_memMalloc(rdi);
    break;
  case 16:
    sys_memFree(rdi);
    break;
  }
  return 0;
}

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
